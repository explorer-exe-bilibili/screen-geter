#define  _CRT_SECURE_NO_WARNINGS
#include <string>
#include <time.h>
#include <iostream>
#include <filesystem>
#include<thread>
#include "config.h"
#include"ac.h"
#include"test.h"
#include<Windows.h>
#include"caream.h"
#include <windows.h>
#include<iostream>
#include <stdio.h>
#include <conio.h>
#pragma comment(lib,"gdiplus.lib")
bool runing = 1;

using namespace std;
int amain();
HHOOK keyboardHook = 0;		// 钩子句柄

LRESULT CALLBACK LowLevelKeyboardProc(
	_In_ int nCode,		// 规定钩子如何处理消息，小于 0 则直接 CallNextHookEx
	_In_ WPARAM wParam,	// 消息类型
	_In_ LPARAM lParam	// 指向某个结构体的指针，这里是 KBDLLHOOKSTRUCT（低级键盘输入事件）
) {
	KBDLLHOOKSTRUCT* ks = (KBDLLHOOKSTRUCT*)lParam;		// 包含低级键盘输入事件信息
	if (ks->flags == 128 || ks->flags == 129)
	{
		// 监控键盘
		switch (ks->vkCode) {
		case VK_F12:
			runing = 0;
			break;
		}
	}
	// 将消息传递给钩子链中的下一个钩子
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(int argc, CHAR * argv[])
{
	// 安装钩子
	keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,	LowLevelKeyboardProc,GetModuleHandleA(NULL),NULL);
	if (keyboardHook == 0) { cout << "挂钩键盘失败" << endl; return -1; }
	MSG msg;
	thread p(amain);
	p.detach();
	while (runing)
	{
		// 如果消息队列中有消息
		if(PeekMessageA(&msg,0,0,0,PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
			Sleep(1);

	}
	// 删除钩子
	UnhookWindowsHookEx(keyboardHook);

	return 0;
}




int amain()
{
	config::init();
	if (config::getint(OPENAC)) {
		std::thread audio(ac);
		audio.detach();
	}
	if (config::getint(OPENCAREAM)) {
		std::thread careamt(camrea);
		careamt.detach();
	}
	HDC hdcScreen = ::GetDC(NULL);
	int nBitPerPixel = GetDeviceCaps(hdcScreen, BITSPIXEL);
	int nWidth = GetDeviceCaps(hdcScreen, HORZRES);
	int nHeight = GetDeviceCaps(hdcScreen, VERTRES);
	while (runing) {
		//建立一个屏幕设备环境句柄
		HDC hdcScreen = ::GetDC(NULL);
		HDC hMemDC;
		HBITMAP hBitmap, hOldBitmap;
		hMemDC = CreateCompatibleDC(hdcScreen);
		hBitmap = CreateCompatibleBitmap(hdcScreen, nWidth, nHeight);
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
		std::wstring filename;
		time_t now = time(0);
		struct tm* localTime = localtime(&now);
		wchar_t timeStr[32];
		if(!std::filesystem::is_directory("photos"))
			_wmkdir(L"photos");
		wsprintf(timeStr, L"photos\\%04d-%02d-%02d %02d-%02d-%02d", localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
		filename+= (timeStr);
		filename+=L".jpg";
		//把屏幕设备描述表拷贝到内存设备描述表中
		BitBlt(hMemDC, 0, 0, nWidth, nHeight, hdcScreen, 0, 0, SRCCOPY);
		std::wcout << filename << endl;
		// Initialize GDI+.
		GdiplusWrapper gdiplus;
		gdiplus.SaveImage(hBitmap, filename.c_str(), L"image/jpeg");

		//释放资源
		DeleteDC(hdcScreen);
		DeleteDC(hMemDC);
		DeleteObject(hBitmap);
		Sleep(config::getint(TIME));
	}
	return 0;
}