#define  _CRT_SECURE_NO_WARNINGS
#include <portaudio.h>
#include <iostream>
#include <fstream>
#include <thread>
#include "config.h"
#include <filesystem>
#include<Windows.h>

extern bool runing;

#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_CHANNELS    (1) 
#define SAMPLE_SIZE    (16)

typedef struct {
	char         RIFF[4];
	uint32_t      ChunkSize;
	char         WAVE[4];
	char         fmt[4];
	uint32_t      Subchunk1Size;
	uint16_t      AudioFormat;
	uint16_t      NumChannels;
	uint32_t      SampleRate;
	uint32_t      ByteRate;
	uint16_t      BlockAlign;
	uint16_t      BitsPerSample;
	char         Subchunk2ID[4];
	uint32_t      Subchunk2Size;
} WavHeader;

int recordCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
	std::ofstream* fileStream = (std::ofstream*)userData;
	const int16_t* rptr = (const int16_t*)inputBuffer;

	for (unsigned long i = 0; i < framesPerBuffer; i++) {
		fileStream->write((const char*)rptr, sizeof(int16_t));
		rptr++;
	}

	return paContinue;
}

int ac() {
	PaStream* stream;
	PaError err;
	err = Pa_Initialize();
	if (err != paNoError) {
		std::cout << "PortAudio initialize failed." << std::endl;
		return -1;
	}
	int timew = config::getint(ACTIME);
	while (1) {
		std::string filename;
		time_t now = time(0);
		struct tm* localTime = localtime(&now);
		char timeStr[32];
		if (!std::filesystem::is_directory("wav"))
			_wmkdir(L"wav");
		sprintf(timeStr, "wav\\%04d-%02d-%02d %02d-%02d-%02d", localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
		filename += (timeStr);
		filename += ".wav";
		std::ofstream wavFile(filename, std::ios::binary);

		WavHeader header;

		header.RIFF[0] = 'R'; header.RIFF[1] = 'I'; header.RIFF[2] = 'F'; header.RIFF[3] = 'F';
		header.WAVE[0] = 'W'; header.WAVE[1] = 'A'; header.WAVE[2] = 'V'; header.WAVE[3] = 'E';
		header.fmt[0] = 'f'; header.fmt[1] = 'm'; header.fmt[2] = 't'; header.fmt[3] = ' ';
		header.Subchunk1Size = 16;
		header.AudioFormat = 1;
		header.NumChannels = NUM_CHANNELS;
		header.SampleRate = SAMPLE_RATE;
		header.BitsPerSample = SAMPLE_SIZE;
		header.ByteRate = SAMPLE_RATE * NUM_CHANNELS * SAMPLE_SIZE / 8;
		header.BlockAlign = NUM_CHANNELS * SAMPLE_SIZE / 8;
		header.Subchunk2ID[0] = 'd'; header.Subchunk2ID[1] = 'a'; header.Subchunk2ID[2] = 't'; header.Subchunk2ID[3] = 'a';
		header.Subchunk2Size = 0;
		header.ChunkSize = 0;

		wavFile.write((const char*)&header, sizeof(header));

		err = Pa_OpenDefaultStream(&stream, NUM_CHANNELS, 0, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, recordCallback, &wavFile);
		if (err != paNoError) {
			std::cout << "Failed to open default stream." << std::endl;
			Pa_Terminate();
			return -1;
		}

		std::cout << "Recording... Press Enter to stop." << std::endl;
		err = Pa_StartStream(stream);
		if (err != paNoError) {
			std::cout << "Failed to start stream." << std::endl;
			Pa_CloseStream(stream);
			Pa_Terminate();
			return -1;
		}

		std::this_thread::sleep_for(std::chrono::seconds(config::getint(ACTIME)));
		auto start = std::chrono::high_resolution_clock::now();
		while (runing) {
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			if (duration.count() >= timew) {
				break;
			}
			Sleep(10);
		}
		if (!runing) {
			break;
		}
		err = Pa_StopStream(stream);
		if (err != paNoError) {
			std::cout << "Failed to stop stream." << std::endl;
		}

		err = Pa_CloseStream(stream);
		if (err != paNoError) {
			std::cout << "Failed to close stream." << std::endl;
		}

		uint32_t fileSize = wavFile.tellp();
		header.ChunkSize = fileSize - 8;
		header.Subchunk2Size = fileSize - sizeof(WavHeader);
		wavFile.seekp(0, std::ios::beg);
		wavFile.write((const char*)&header, sizeof(header));
		wavFile.close();
		std::cout << "Done. Audio is saved to 'recorded.wav'." << std::endl;
	}
	Pa_Terminate();
	return 0;
}