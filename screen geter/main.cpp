//#include <opencv2/core.hpp> 
//#include <opencv2/imgcodecs.hpp> 
//#include <opencv2/highgui.hpp>
//#include <opencv2/opencv.hpp> 
//#include <iostream> 
//
//using namespace cv;
//using namespace std;
//
//int main()
//{
//    //两个相机同时打开，初始化并分配内存以从摄像机加载视频流，分辨率为320*240、1280*720（延迟太大）、720*480 
//    //initialize and allocate memory to load the video stream from camera 
//    VideoCapture camera0(0);
//    camera0.set(CAP_PROP_FRAME_WIDTH, 720);
//    camera0.set(CAP_PROP_FRAME_HEIGHT, 480);
//
//    VideoCapture camera1(1);
//    camera1.set(CAP_PROP_FRAME_WIDTH, 720);
//    camera1.set(CAP_PROP_FRAME_HEIGHT, 480);
//
//    if (!camera0.isOpened()) return 1;
//    if (!camera1.isOpened()) return 1;
//
//    int a = 1;
//    int f = 1;
//    while (true) {
//        //依次抓取和检索视频的每个帧 
//        //grab and retrieve each frames of the video sequentially 
//        Mat3b frame0;
//        camera0 >> frame0;
//        Mat3b frame1;
//        camera1 >> frame1;
//
//        //顺时针旋转摄像头180° 
//        //flip(frame1, frame1, -1); 
//        //flip(frame0, frame0, -1); 
//
//        imshow("Video0", frame0);
//        imshow("Video1", frame1);
//        //std::cout << frame1.rows() << std::endl; 
//        //wait for 40 milliseconds 
//
//        int c = waitKey(20);
//        //exit the loop if user press "Esc" key (ASCII value of "Esc" is 27)
//
//        if (27 == char(c)) break;
//
//        int b = waitKey(20);
//        if (32 == char(b)) {
//            imwrite("C:\\Users\\11815\\Desktop\\calib_example1\\" + to_string(a++) + ".tif", frame1);
//        }
//
//        int d = waitKey(20);
//        if (13 == char(d))
//        {
//            imwrite("C:\\Users\\11815\\Desktop\\calib_example2\\" + to_string(f++) + ".tif", frame0);
//        }
//    }
//    return 0;
//}
//




#define  _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <gdiplus.h>
#include <string>
#include <time.h>
#include <iostream>
#include <filesystem>
#include "config.h"
#pragma comment(lib,"gdiplus.lib")

using namespace std;

class GdiplusWrapper {
public:
	GdiplusWrapper() {
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	}

	~GdiplusWrapper() {
		Gdiplus::GdiplusShutdown(gdiplusToken);
	}

	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
	{
		UINT  num = 0;          // number of image encoders
		UINT  size = 0;         // size of the image encoder array in bytes

		Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

		Gdiplus::GetImageEncodersSize(&num, &size);
		if (size == 0)
			return -1;  // Failure

		pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == NULL)
			return -1;  // Failure

		Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

		for (UINT j = 0; j < num; ++j)
		{
			if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j;  // Success
			}
		}

		free(pImageCodecInfo);
		return -1;  // Failure
	}

	void SaveImage(HBITMAP hBitmap, const WCHAR* filename, const WCHAR* format) {
		CLSID pngClsid;
		Gdiplus::Bitmap bitmap(hBitmap, NULL);
		GetEncoderClsid(format, &pngClsid);
		bitmap.Save(filename, &pngClsid);
	}

private:
	ULONG_PTR gdiplusToken;
};

void main()
{
	config::init();
	HDC hdcScreen = ::GetDC(NULL);
	//HDC hdcScreen = CreateDC("DISPLAY", NULL, NULL, NULL);
	int nBitPerPixel = GetDeviceCaps(hdcScreen, BITSPIXEL);
	int nWidth = GetDeviceCaps(hdcScreen, HORZRES);
	int nHeight = GetDeviceCaps(hdcScreen, VERTRES);
	while (1) {
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
}