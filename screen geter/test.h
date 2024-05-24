#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "winmm.lib")

#define AUDIO_BUFFER_SIZE 1024
// 音频录制线程函数
DWORD WINAPI AudioCaptureThread(LPVOID lpParam) {
	// 从参数中获取音频客户端指针
	IAudioClient* pAudioClient = (IAudioClient*)lpParam;
	// 获取音频捕获客户端接口
	IAudioCaptureClient* pAudioCaptureClient;
	pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pAudioCaptureClient);

	BYTE* pData;
	UINT32 packetLength = 0;
	DWORD flags = 0;

	// 循环获取音频数据
	while (true) {
		// 检查是否有数据包准备好
		while (pAudioCaptureClient->GetNextPacketSize(&packetLength) == S_OK && packetLength == 0) {
			// 没有数据包准备好，稍作延时
			Sleep(10);
			using namespace std;

			std::cout << "package not prepared" << endl;
		}

		if (packetLength > 0) {
			// 获取数据包
			pAudioCaptureClient->GetBuffer(&pData, &packetLength, &flags, NULL, NULL);

			if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
				// 填充静音数据
				memset(pData, 0, packetLength);
			}

			// 处理音频数据
			// 这里我们直接写入文件
			FILE* pFile = fopen("output.wav", "ab");
			fwrite(pData, 1, packetLength, pFile);
			fclose(pFile);

			// 释放数据包
			pAudioCaptureClient->ReleaseBuffer(packetLength);
		}
	}

	// 清理资源
	pAudioCaptureClient->Release();
	return 0;
}

int audio() {
	CoInitialize(NULL);

	// 创建设备枚举器
	IMMDeviceEnumerator* pDeviceEnumerator;
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pDeviceEnumerator);
	if (FAILED(hr)) {
		std::cerr << "Failed to create device enumerator." << std::endl;
		return -1;
	}

	// 获取默认音频设备
	IMMDevice* pDevice;
	hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
	if (FAILED(hr)) {
		std::cerr << "Failed to get default audio endpoint." << std::endl;
		pDeviceEnumerator->Release();
		return -1;
	}

	// 获取音频客户端
	IAudioClient* pAudioClient;
	hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
	if (FAILED(hr)) {
		std::cerr << "Failed to activate audio client." << std::endl;
		pDevice->Release();
		pDeviceEnumerator->Release();
		return -1;
	}

	// 初始化音频客户端
	WAVEFORMATEX* pwfx = NULL;
	hr = pAudioClient->GetMixFormat(&pwfx);
	if (FAILED(hr)) {
		std::cerr << "Failed to get mix format." << std::endl;
		pAudioClient->Release();
		pDevice->Release();
		pDeviceEnumerator->Release();
		return -1;
	}

	hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, pwfx, NULL);
	if (FAILED(hr)) {
		std::cerr << "Failed to initialize audio client." << std::endl;
		CoTaskMemFree(pwfx);
		pAudioClient->Release();
		pDevice->Release();
		pDeviceEnumerator->Release();
		return -1;
	}

	CoTaskMemFree(pwfx);

	// 创建音频录制线程
	HANDLE hThread = CreateThread(NULL, 0, AudioCaptureThread, pAudioClient, 0, NULL);
	if (hThread == NULL) {
		std::cerr << "Failed to create audio capture thread." << std::endl;
		pAudioClient->Release();
		pDevice->Release();
		pDeviceEnumerator->Release();
		return -1;
	}

	// 等待用户输入以结束程序
	std::cout << "Recording... Press Enter to stop." << std::endl;
	std::cin.get();

	// 停止录制
	pAudioClient->Stop();

	// 等待录制线程结束
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	// 清理资源
	pAudioClient->Release();
	pDevice->Release();
	pDeviceEnumerator->Release();
	CoUninitialize();

	return 0;
}