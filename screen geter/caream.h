#pragma once
#define  _CRT_SECURE_NO_WARNINGS
#include <opencv2/core.hpp> 
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp> 
#include <iostream> 
#include <Windows.h>
#include <gdiplus.h>
#include "config.h"
#include <filesystem>
extern bool runing;


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


using namespace std;
int camrea()
{
	using namespace cv;
	using namespace std;

	//两个相机同时打开，初始化并分配内存以从摄像机加载视频流，分辨率为320*240、1280*720（延迟太大）、720*480 
	//initialize and allocate memory to load the video stream from camera 
	if (!config::getint(ENABLE2CAREMA)) {
		VideoCapture camera0(0);
		camera0.open(0, cv::CAP_DSHOW);


		if (!camera0.isOpened()) return 1;

		int a = 1;
		int f = 1;
		while (true) {
			Mat3b frame0;
			camera0 >> frame0;
			std::string filename;
			time_t now = time(0);
			struct tm* localTime = localtime(&now);
			char timeStr[32];
			if (!std::filesystem::is_directory("photos"))
				_wmkdir(L"photos");
			sprintf(timeStr, "photos\\%04d-%02d-%02d %02d-%02d-%02d", localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
			filename += (timeStr);
			filename += ".jpg";
			imwrite(filename.c_str(), frame0);
			cout << "took a photo" << endl;
			std::this_thread::sleep_for(std::chrono::seconds(config::getint(CAREAMTIME)));
			if (!runing) {
				break;
			}
		}
	}
	else {
		VideoCapture camera0(0);
		VideoCapture camera1(1);
		camera0.open(0, cv::CAP_DSHOW);
		camera1.open(0, cv::CAP_DSHOW);

		if (!camera0.isOpened()) return 1;
		if (!camera1.isOpened()) return 1;

		int a = 1;
		int f = 1;
		while (1) {
			Mat3b frame0;
			Mat3b frame1;
			camera0 >> frame0;
			camera1 >> frame1;
			std::string filename;
			time_t now = time(0);
			struct tm* localTime = localtime(&now);
			char timeStr[32];
			if (!std::filesystem::is_directory("photos"))
				_wmkdir(L"photos");
			sprintf(timeStr, "photos\\%04d-%02d-%02d %02d-%02d-%02d---1", localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
			filename += (timeStr);
			filename += ".jpg";
			imwrite(filename.c_str(), frame0);
			sprintf(timeStr, "photos\\%04d-%02d-%02d %02d-%02d-%02d---2", localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
			filename += (timeStr);
			filename += ".jpg";
			imwrite(filename.c_str(), frame1);
			cout << "took 2 photos" << endl;
			std::this_thread::sleep_for(std::chrono::seconds(config::getint(CAREAMTIME)));
			if (!runing) {
				break;
			}
		}
	}
	return 0;
}
