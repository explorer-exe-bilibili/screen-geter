#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "winmm.lib")

#define AUDIO_BUFFER_SIZE 1024
// ��Ƶ¼���̺߳���
DWORD WINAPI AudioCaptureThread(LPVOID lpParam) {
	// �Ӳ����л�ȡ��Ƶ�ͻ���ָ��
	IAudioClient* pAudioClient = (IAudioClient*)lpParam;
	// ��ȡ��Ƶ����ͻ��˽ӿ�
	IAudioCaptureClient* pAudioCaptureClient;
	pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pAudioCaptureClient);

	BYTE* pData;
	UINT32 packetLength = 0;
	DWORD flags = 0;

	// ѭ����ȡ��Ƶ����
	while (true) {
		// ����Ƿ������ݰ�׼����
		while (pAudioCaptureClient->GetNextPacketSize(&packetLength) == S_OK && packetLength == 0) {
			// û�����ݰ�׼���ã�������ʱ
			Sleep(10);
			using namespace std;

			std::cout << "package not prepared" << endl;
		}

		if (packetLength > 0) {
			// ��ȡ���ݰ�
			pAudioCaptureClient->GetBuffer(&pData, &packetLength, &flags, NULL, NULL);

			if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
				// ��侲������
				memset(pData, 0, packetLength);
			}

			// ������Ƶ����
			// ��������ֱ��д���ļ�
			FILE* pFile = fopen("output.wav", "ab");
			fwrite(pData, 1, packetLength, pFile);
			fclose(pFile);

			// �ͷ����ݰ�
			pAudioCaptureClient->ReleaseBuffer(packetLength);
		}
	}

	// ������Դ
	pAudioCaptureClient->Release();
	return 0;
}

int audio() {
	CoInitialize(NULL);

	// �����豸ö����
	IMMDeviceEnumerator* pDeviceEnumerator;
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pDeviceEnumerator);
	if (FAILED(hr)) {
		std::cerr << "Failed to create device enumerator." << std::endl;
		return -1;
	}

	// ��ȡĬ����Ƶ�豸
	IMMDevice* pDevice;
	hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
	if (FAILED(hr)) {
		std::cerr << "Failed to get default audio endpoint." << std::endl;
		pDeviceEnumerator->Release();
		return -1;
	}

	// ��ȡ��Ƶ�ͻ���
	IAudioClient* pAudioClient;
	hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
	if (FAILED(hr)) {
		std::cerr << "Failed to activate audio client." << std::endl;
		pDevice->Release();
		pDeviceEnumerator->Release();
		return -1;
	}

	// ��ʼ����Ƶ�ͻ���
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

	// ������Ƶ¼���߳�
	HANDLE hThread = CreateThread(NULL, 0, AudioCaptureThread, pAudioClient, 0, NULL);
	if (hThread == NULL) {
		std::cerr << "Failed to create audio capture thread." << std::endl;
		pAudioClient->Release();
		pDevice->Release();
		pDeviceEnumerator->Release();
		return -1;
	}

	// �ȴ��û������Խ�������
	std::cout << "Recording... Press Enter to stop." << std::endl;
	std::cin.get();

	// ֹͣ¼��
	pAudioClient->Stop();

	// �ȴ�¼���߳̽���
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	// ������Դ
	pAudioClient->Release();
	pDevice->Release();
	pDeviceEnumerator->Release();
	CoUninitialize();

	return 0;
}