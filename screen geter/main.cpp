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
HHOOK keyboardHook = 0;		// ���Ӿ��

LRESULT CALLBACK LowLevelKeyboardProc(
	_In_ int nCode,		// �涨������δ�����Ϣ��С�� 0 ��ֱ�� CallNextHookEx
	_In_ WPARAM wParam,	// ��Ϣ����
	_In_ LPARAM lParam	// ָ��ĳ���ṹ���ָ�룬������ KBDLLHOOKSTRUCT���ͼ����������¼���
) {
	KBDLLHOOKSTRUCT* ks = (KBDLLHOOKSTRUCT*)lParam;		// �����ͼ����������¼���Ϣ
	if (ks->flags == 128 || ks->flags == 129)
	{
		// ��ؼ���
		switch (ks->vkCode) {
		case VK_F12:
			runing = 0;
			break;
		}
	}
	// ����Ϣ���ݸ��������е���һ������
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(int argc, CHAR * argv[])
{
	// ��װ����
	keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,	LowLevelKeyboardProc,GetModuleHandleA(NULL),NULL);
	if (keyboardHook == 0) { cout << "�ҹ�����ʧ��" << endl; return -1; }
	MSG msg;
	thread p(amain);
	p.detach();
	while (runing)
	{
		// �����Ϣ����������Ϣ
		if(PeekMessageA(&msg,0,0,0,PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
			Sleep(1);

	}
	// ɾ������
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
		//����һ����Ļ�豸�������
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
		//����Ļ�豸�����������ڴ��豸��������
		BitBlt(hMemDC, 0, 0, nWidth, nHeight, hdcScreen, 0, 0, SRCCOPY);
		std::wcout << filename << endl;
		// Initialize GDI+.
		GdiplusWrapper gdiplus;
		gdiplus.SaveImage(hBitmap, filename.c_str(), L"image/jpeg");

		//�ͷ���Դ
		DeleteDC(hdcScreen);
		DeleteDC(hMemDC);
		DeleteObject(hBitmap);
		Sleep(config::getint(TIME));
	}
	return 0;
}