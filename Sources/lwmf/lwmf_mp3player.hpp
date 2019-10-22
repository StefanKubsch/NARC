/*
****************************************************
*                                                  *
* lwmf_mp3player - lightweight media framework     *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <windows.h>
#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <shlwapi.h>
#include <wmsdk.h>

#pragma comment(lib, "msacm32.lib")
#pragma comment(lib, "wmvcore.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Shlwapi.lib")

#include "lwmf_logging.hpp"

namespace lwmf
{


	class MP3Player
	{
	public:
		void Load(const std::string& Filename);
		void Close();
		void Play();
		double GetDuration();
		double GetPosition();
		bool IsPlaying();

	private:
		std::vector<BYTE> WaveBuffer{};
		WAVEFORMATEX PCMFormat{ WAVE_FORMAT_PCM, 2, 44100, 4 * 44100, 4, 16, 0 };
		WAVEHDR WaveHDR{};
		HWAVEOUT WaveOut{};
		DWORD WaveBufferLength{};
		double Duration{};
		bool PlayStarted{};
	};

	inline void MP3Player::Load(const std::string& Filename)
	{
		std::ifstream File(Filename.c_str(), std::ios::in | std::ios::binary);

		if (File.fail())
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error loading " + Filename + "!");
		}
		else
		{
			std::streamsize InputBufferSize{};

			if (File.seekg(0, std::ios::end).good())
			{
				InputBufferSize = File.tellg();
			}

			if (File.seekg(0, std::ios::beg).good())
			{
				InputBufferSize -= File.tellg();
			}

			std::vector<BYTE> InputBuffer(static_cast<size_t>(InputBufferSize));
			File.read(reinterpret_cast<char*>(InputBuffer.data()), InputBufferSize);

			constexpr DWORD MP3BlockSize{ 522 };
			MPEGLAYER3WAVEFORMAT MP3Format{ { WAVE_FORMAT_MPEGLAYER3, 2, 44100, 128 * (1024 / 8), 1, 0, MPEGLAYER3_WFX_EXTRA_BYTES }, MPEGLAYER3_ID_MPEG, MPEGLAYER3_FLAG_PADDING_OFF, MP3BlockSize, 1, 1393 };

			CoInitializeEx(nullptr, COINIT_MULTITHREADED);

			IWMSyncReader* SyncReader{};
			WMCreateSyncReader(nullptr, WMT_RIGHT_PLAYBACK, &SyncReader);

			IStream* MP3Stream{ SHCreateMemStream(InputBuffer.data(), static_cast<UINT>(InputBufferSize)) };
			SyncReader->OpenStream(MP3Stream);

			IWMHeaderInfo* HeaderInfo{};
			SyncReader->QueryInterface(&HeaderInfo);

			WORD DataTypeLength{ sizeof(QWORD) };
			WORD StreamNum{};
			WMT_ATTR_DATATYPE DataTypeAttribute{};
			QWORD DurationInNano{};
			HeaderInfo->GetAttributeByName(&StreamNum, L"Duration", &DataTypeAttribute, reinterpret_cast<BYTE*>(&DurationInNano), &DataTypeLength);
			Duration = static_cast<double>(DurationInNano) / 10000000.0;

			IWMProfile* Profile{};
			SyncReader->QueryInterface(&Profile);
			IWMStreamConfig* StreamConfig{};
			Profile->GetStream(0, &StreamConfig);
			IWMMediaProps* MediaProperties;
			StreamConfig->QueryInterface(&MediaProperties);

			DWORD MediaTypeSize{};
			MediaProperties->GetMediaType(nullptr, &MediaTypeSize);
			std::vector<WM_MEDIA_TYPE> MediaType(MediaTypeSize);
			MediaProperties->GetMediaType(MediaType.data(), &MediaTypeSize);

			MediaProperties->Release();
			StreamConfig->Release();
			Profile->Release();
			HeaderInfo->Release();
			SyncReader->Release();

			WaveBufferLength = Duration * PCMFormat.nAvgBytesPerSec;
			WaveBuffer.resize(static_cast<std::size_t>(WaveBufferLength));

			HACMSTREAM ACMStream{};
			acmStreamOpen(&ACMStream, nullptr, reinterpret_cast<LPWAVEFORMATEX>(&MP3Format), &PCMFormat, nullptr, 0, 0, 0);
			unsigned long RawBufferSize{};
			acmStreamSize(ACMStream, MP3BlockSize, &RawBufferSize, ACM_STREAMSIZEF_SOURCE);

			std::vector<BYTE> MP3BlockBuffer(MP3BlockSize);
			LPBYTE RawBuffer{ static_cast<LPBYTE>(LocalAlloc(LPTR, static_cast<std::size_t>(RawBufferSize))) };

			ACMSTREAMHEADER StreamHead{};
			StreamHead.cbStruct = sizeof(ACMSTREAMHEADER);
			StreamHead.pbSrc = MP3BlockBuffer.data();
			StreamHead.cbSrcLength = MP3BlockSize;
			StreamHead.pbDst = RawBuffer;
			StreamHead.cbDstLength = RawBufferSize;
			acmStreamPrepareHeader(ACMStream, &StreamHead, 0);

			BYTE* CurrentOutput{ WaveBuffer.data() };
			DWORD DecompressedSize{};

			ULARGE_INTEGER NewPosition{};
			LARGE_INTEGER SeekValue{};
			MP3Stream->Seek(SeekValue, STREAM_SEEK_SET, &NewPosition);

			while (true)
			{
				ULONG Counter{};
				MP3Stream->Read(MP3BlockBuffer.data(), MP3BlockSize, &Counter);

				if (Counter != MP3BlockSize)
				{
					break;
				}

				acmStreamConvert(ACMStream, &StreamHead, ACM_STREAMCONVERTF_BLOCKALIGN);
				std::memcpy(CurrentOutput, RawBuffer, static_cast<std::size_t>(StreamHead.cbDstLengthUsed));
				DecompressedSize += StreamHead.cbDstLengthUsed;
				CurrentOutput += StreamHead.cbDstLengthUsed;
			}

			acmStreamUnprepareHeader(ACMStream, &StreamHead, 0);
			LocalFree(RawBuffer);
			acmStreamClose(ACMStream, 0);

			MP3Stream->Release();
		}
	}

	inline void MP3Player::Close()
	{
		PlayStarted = false;
		waveOutReset(WaveOut);
		waveOutClose(WaveOut);
	}

	inline void MP3Player::Play()
	{
		WaveHDR = { reinterpret_cast<LPSTR>(WaveBuffer.data()), WaveBufferLength };
		waveOutOpen(&WaveOut, WAVE_MAPPER, &PCMFormat, NULL, 0, CALLBACK_NULL);
		waveOutPrepareHeader(WaveOut, &WaveHDR, sizeof(WaveHDR));
		waveOutWrite(WaveOut, &WaveHDR, sizeof(WaveHDR));
		PlayStarted = true;
	}

	inline double MP3Player::GetDuration()
	{
		return Duration;
	}

	inline double MP3Player::GetPosition()
	{
		static MMTIME MMTime { TIME_MS, 0 };
		waveOutGetPosition(WaveOut, &MMTime, sizeof(MMTIME));
		return static_cast<double>(MMTime.u.ms) / 100000.0;
	}

	inline bool MP3Player::IsPlaying()
	{
		if (PlayStarted & (GetPosition() < Duration))
		{
			return true;
		}

		return false;
	}


} // namespace lwmf