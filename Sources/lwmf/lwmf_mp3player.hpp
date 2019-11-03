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
#include <array>
#include <iostream>
#include <fstream>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <shlwapi.h>
#include <wmsdk.h>
#include <atlcomcli.h>

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
		void CheckHRESError(HRESULT Error, const std::string& Operation);
		void CheckMMRESError(MMRESULT Error, const std::string& Operation);

		std::vector<BYTE> WaveBuffer{};
		WAVEFORMATEX PCMFormat{};
		MPEGLAYER3WAVEFORMAT MP3Format{};
		WAVEHDR WaveHDR{};
		HWAVEOUT WaveOut{};
		DWORD WaveBufferLength{};
		DWORD Bitrate{};
		DWORD SampleRate{};
		WORD NumberOfChannels{ 2 };
		double Duration{};
		bool PlayStarted{};
	};

	inline void MP3Player::Load(const std::string& Filename)
	{
		// This MP3 Player uses the Windows Waveform Audio Interface:
		// https://docs.microsoft.com/en-us/windows/win32/multimedia/waveform-audio-interface

		// For a description of the MP3 header, have a look here:
		// https://www.mp3-tech.org/programmer/frame_header.html

		// This implementation only supports MPEG Version 1!

		//
		// Read MP3 Header
		//

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Reading MP3 header of " + Filename);
		std::ifstream File(Filename.c_str(), std::ios::in | std::ios::binary);

		if (File.fail())
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error loading " + Filename + "!");
		}
		else
		{
			std::int_fast32_t StreamChar{};

			// Search for header informations
			while (true)
			{
				StreamChar = File.get();

				if (StreamChar == 255)
				{
					StreamChar = File.get();

					if (StreamChar >> 4 == 15)
					{
						break;
					}
				}
			}

			// Check if file is MPEG Version 1
			if (((StreamChar & 15) >> 2) >> 1 != 1)
			{
				LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "This is not a MPEG Version 1 MP3!");
			}

			// Get Bitrate
			StreamChar = File.get();
			const std::array<std::int_fast32_t, 16> BitrateTable { 0x000, 0x020, 0x028, 0x030, 0x038, 0x040, 0x050, 0x060, 0x070, 0x080, 0x0A0, 0x0C0, 0x0E0, 0x100, 0x140, 0x000 };
			Bitrate = BitrateTable[StreamChar >> 4];
			LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Bitrate: " + std::to_string(Bitrate));

			// Get Samplerate
			const std::array<std::int_fast32_t, 4> SampleRateTable{ 0x0AC44, 0x0BB80, 0x07D00, 0x00000 };
			SampleRate = SampleRateTable[(StreamChar & 15) >> 2];
			LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Samplerate: " + std::to_string(SampleRate));

			// Get number of channels
			StreamChar = File.get();

			// NumberOfChannels is per default initialized with "2"
			// Set only to "1" if "Single Channel" is detected...
			if (((StreamChar >> 4) >> 2) == 3)
			{
				NumberOfChannels = 1;
			}

			LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Number of channels: " + std::to_string(NumberOfChannels));

			//
			// Transcode MP3 to PCM stream
			//

			constexpr WORD MP3BlockSize{ 522 };

			MP3Format = { { WAVE_FORMAT_MPEGLAYER3, NumberOfChannels, SampleRate, Bitrate << 7, 1, 0, MPEGLAYER3_WFX_EXTRA_BYTES }, MPEGLAYER3_ID_MPEG, MPEGLAYER3_FLAG_PADDING_OFF, MP3BlockSize, 1, 1393 };
			PCMFormat = { WAVE_FORMAT_PCM, NumberOfChannels, SampleRate, SampleRate << 2, 4, 16, 0 };

			std::size_t FileSize{};

			if (File.seekg(0, std::ios::end).good())
			{
				FileSize = File.tellg();
			}

			if (File.seekg(0, std::ios::beg).good())
			{
				FileSize -= File.tellg();
			}

			std::vector<BYTE> InputBuffer(FileSize);
			File.read(reinterpret_cast<char*>(InputBuffer.data()), FileSize);

			CComPtr<IWMSyncReader> SyncReader{};
			CheckHRESError(WMCreateSyncReader(nullptr, WMT_RIGHT_PLAYBACK, &SyncReader), "WMCreateSyncReader");
			CComPtr<IStream> MP3Stream{ SHCreateMemStream(InputBuffer.data(), static_cast<UINT>(FileSize)) };
			CheckHRESError(SyncReader->OpenStream(MP3Stream), "OpenStream");

			CComPtr<IWMHeaderInfo> HeaderInfo{};
			CheckHRESError(SyncReader->QueryInterface(&HeaderInfo), "QueryInterface");
			WORD DataTypeLength{ sizeof(QWORD) };
			WORD StreamNum{};
			WMT_ATTR_DATATYPE DataTypeAttribute{};
			QWORD DurationInNano{};
			CheckHRESError(HeaderInfo->GetAttributeByName(&StreamNum, L"Duration", &DataTypeAttribute, reinterpret_cast<BYTE*>(&DurationInNano), &DataTypeLength), "GetAttributeByName"); //-V206
			// Round Duration to 3 decimal places ( = precision of 1ms)
			Duration = static_cast<double>(DurationInNano * 100) / 1000000000.0;

			CComPtr<IWMProfile> Profile{};
			CheckHRESError(SyncReader->QueryInterface(&Profile), "QueryInterface");
			CComPtr<IWMStreamConfig> StreamConfig{};
			CheckHRESError(Profile->GetStream(0, &StreamConfig), "GetStream");
			CComPtr<IWMMediaProps> MediaProperties;
			CheckHRESError(StreamConfig->QueryInterface(&MediaProperties), "QueryInterface");

			DWORD MediaTypeSize{};
			CheckHRESError(MediaProperties->GetMediaType(nullptr, &MediaTypeSize), "GetMediaType");
			std::vector<WM_MEDIA_TYPE> MediaType(MediaTypeSize);
			CheckHRESError(MediaProperties->GetMediaType(MediaType.data(), &MediaTypeSize), "GetMediaType");

			WaveBufferLength = static_cast<DWORD>(Duration * PCMFormat.nAvgBytesPerSec);
			WaveBuffer.resize(static_cast<std::size_t>(WaveBufferLength));

			HACMSTREAM ACMStream{};
			CheckMMRESError(acmStreamOpen(&ACMStream, nullptr, reinterpret_cast<LPWAVEFORMATEX>(&MP3Format), &PCMFormat, nullptr, 0, 0, 0), "acmStreamOpen");
			DWORD RawBufferSize{};
			CheckMMRESError(acmStreamSize(ACMStream, MP3BlockSize, &RawBufferSize, ACM_STREAMSIZEF_SOURCE), "acmStreamSize");

			std::vector<BYTE> MP3BlockBuffer(static_cast<std::size_t>(MP3BlockSize));
			std::vector<BYTE> RawBuffer(static_cast<std::size_t>(RawBufferSize));

			ACMSTREAMHEADER StreamHead{};
			StreamHead.cbStruct = sizeof(ACMSTREAMHEADER);
			StreamHead.pbSrc = MP3BlockBuffer.data();
			StreamHead.cbSrcLength = MP3BlockSize;
			StreamHead.pbDst = RawBuffer.data();
			StreamHead.cbDstLength = RawBufferSize;
			CheckMMRESError(acmStreamPrepareHeader(ACMStream, &StreamHead, 0), "acmStreamPrepareHeader");

			ULARGE_INTEGER NewPosition{};
			LARGE_INTEGER SeekValue{};
			CheckHRESError(MP3Stream->Seek(SeekValue, STREAM_SEEK_SET, &NewPosition), "MP3StreamSeek");

			std::size_t Offset{};

			while (true)
			{
				ULONG Counter{};
				CheckHRESError(MP3Stream->Read(MP3BlockBuffer.data(), MP3BlockSize, &Counter), "MP3StreamRead");

				if (Counter != MP3BlockSize)
				{
					break;
				}

				CheckMMRESError(acmStreamConvert(ACMStream, &StreamHead, ACM_STREAMCONVERTF_BLOCKALIGN), "acmStreamConvert");
				std::copy(RawBuffer.begin(), RawBuffer.begin() + static_cast<std::size_t>(StreamHead.cbDstLengthUsed), WaveBuffer.begin() + Offset);
				Offset += static_cast<std::size_t>(StreamHead.cbDstLengthUsed);
			}

			CheckMMRESError(acmStreamUnprepareHeader(ACMStream, &StreamHead, 0), "acmStreamUnprepareHeader");
			CheckMMRESError(acmStreamClose(ACMStream, 0), "acmStreamClose");
		}
	}

	inline void MP3Player::Close()
	{
		waveOutReset(WaveOut);

		while (waveOutUnprepareHeader(WaveOut, &WaveHDR, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		{
			Sleep(100);
		}

		WaveBuffer.clear();
		WaveBuffer.shrink_to_fit();

		waveOutClose(WaveOut);

		PlayStarted = false;
	}

	inline void MP3Player::Play()
	{
		WaveHDR = { reinterpret_cast<LPSTR>(WaveBuffer.data()), WaveBufferLength };
		waveOutOpen(&WaveOut, WAVE_MAPPER, &PCMFormat, NULL, 0, CALLBACK_NULL);
		waveOutPrepareHeader(WaveOut, &WaveHDR, sizeof(WAVEHDR));
		waveOutWrite(WaveOut, &WaveHDR, sizeof(WAVEHDR));
		PlayStarted = true;
	}

	inline double MP3Player::GetDuration()
	{
		return Duration;
	}

	inline double MP3Player::GetPosition()
	{
		static MMTIME MMTime{ TIME_SAMPLES, {} };
		waveOutGetPosition(WaveOut, &MMTime, sizeof(MMTIME));
		// Round Position to 3 decimal places ( = precision of 1ms)
		return std::round(static_cast<double>(MMTime.u.sample) / static_cast<double>(SampleRate) * 1000.0) / 1000.0;
	}

	inline bool MP3Player::IsPlaying()
	{
		if (PlayStarted)
		{
			if (GetPosition() + 0.0001 <= Duration)
			{
				return true;
			}
		}

		PlayStarted = false;
		return false;
	}

	inline void MP3Player::CheckHRESError(HRESULT Error, const std::string& Operation)
	{
		if (Error != S_OK)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, Operation + " error: " + std::to_string(Error));
		}
	}

	inline void MP3Player::CheckMMRESError(MMRESULT Error, const std::string& Operation)
	{
		if (Error != MMSYSERR_NOERROR)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, Operation + " error: " + std::to_string(Error));
		}
	}


} // namespace lwmf