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
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <map>
#include <fstream>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <shlwapi.h>
#include <wmsdk.h>
#include <atlcomcli.h>
#include <cstring>

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
		void Pause();
		void Restart();
		double GetDuration();
		double GetPosition();
		void SetVolume(std::int_fast32_t LeftPercent, std::int_fast32_t RightPercent);
		bool IsFinished();

	private:
		static void CheckHRESError(HRESULT Error, const std::string& Operation, enum LogLevel Level);
		static void CheckMMRESError(MMRESULT Error, const std::string& Operation, enum LogLevel Level);

		enum class State : std::int_fast32_t
		{
			Finished,
			Playing,
			Paused,
			Stopped
		};

		std::vector<BYTE> WaveBuffer{};
		WAVEFORMATEX PCMFormat{};
		MPEGLAYER3WAVEFORMAT MP3Format{};
		WAVEHDR WaveHDR{};
		HWAVEOUT WaveOut{};
		State Playstate{};
		std::string AudioName;
		DWORD Bitrate{};
		DWORD SampleRate{};
		WORD NumberOfChannels{ 2 };
		double Duration{};
	};

	inline void MP3Player::Load(const std::string& Filename)
	{
		// This MP3 Player uses the Windows Waveform Audio Interface:
		// https://docs.microsoft.com/en-us/windows/win32/multimedia/waveform-audio-interface

		// For a description of the MP3 header, have a look here:
		// https://www.mp3-tech.org/programmer/frame_header.html

		// This implementation only supports MPEG Version 1!

		//
		// Get device informations
		//

		const UINT NumberOfDevices{ waveOutGetNumDevs() };

		if (NumberOfDevices != 0)
		{
			for (UINT i{}; i < NumberOfDevices; ++i)
			{
				WAVEOUTCAPS WaveOutCaps{};
				CheckMMRESError(waveOutGetDevCaps(i, &WaveOutCaps, sizeof(WAVEOUTCAPS)), "waveOutGetDevCaps", LogLevel::Info); //-V106

				LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Found audio device: " + std::string(WaveOutCaps.szPname));
				LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Number of channels: " + std::to_string(WaveOutCaps.wChannels));
			}
		}
		else
		{
			LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "No audio device found!");
		}

		//
		// Read MP3 Header
		//

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Reading MP3 header of " + Filename);
		std::ifstream File(Filename.c_str(), std::ios::in | std::ios::binary);

		if (File.fail())
		{
			std::array<char, 100> ErrorMessage{};
			strerror_s(ErrorMessage.data(), 100, errno);

			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error loading " + Filename + ": " + std::string(ErrorMessage.data()));
		}
		else
		{
			AudioName = Filename;

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
			constexpr std::array<std::int_fast32_t, 16> BitrateTable{ 0x000, 0x020, 0x028, 0x030, 0x038, 0x040, 0x050, 0x060, 0x070, 0x080, 0x0A0, 0x0C0, 0x0E0, 0x100, 0x140, 0x000 };
			Bitrate = BitrateTable[StreamChar >> 4];
			LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Bitrate: " + std::to_string(Bitrate));

			// Get Samplerate
			constexpr std::array<std::int_fast32_t, 4> SampleRateTable{ 0x0AC44, 0x0BB80, 0x07D00, 0x00000 };
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

			// Description of MPEGLAYER3WAVEFORMAT structure
			// https://docs.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-mpeglayer3waveformat
			MP3Format = { { WAVE_FORMAT_MPEGLAYER3, NumberOfChannels, SampleRate, Bitrate * (1024 / 8), 1, 0, MPEGLAYER3_WFX_EXTRA_BYTES }, MPEGLAYER3_ID_MPEG, MPEGLAYER3_FLAG_PADDING_OFF, MP3BlockSize, 1, 0 };

			// Description of WAVEFORMATEX structure
			// https://docs.microsoft.com/en-us/previous-versions/dd757713(v%3dvs.85)?redirectedfrom=MSDN
			const WORD nBlockAlign{ static_cast<WORD>((NumberOfChannels * 16) / 8) };
			PCMFormat = { WAVE_FORMAT_PCM, NumberOfChannels, SampleRate, SampleRate * nBlockAlign, nBlockAlign, 16, 0 };

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

			CheckHRESError(CoInitializeEx(nullptr, COINIT_MULTITHREADED), "CoInitializeEx", LogLevel::Critical);

			// Create a local scope for working with the CComPtrs...
			{
				CComPtr<IWMSyncReader> SyncReader{};
				CheckHRESError(WMCreateSyncReader(nullptr, WMT_RIGHT_PLAYBACK, &SyncReader), "WMCreateSyncReader", LogLevel::Error);
				const CComPtr<IStream> MP3Stream{ SHCreateMemStream(InputBuffer.data(), static_cast<UINT>(FileSize)) };
				CheckHRESError(SyncReader->OpenStream(MP3Stream), "OpenStream", LogLevel::Error);

				CComPtr<IWMHeaderInfo> HeaderInfo{};
				CheckHRESError(SyncReader->QueryInterface(&HeaderInfo), "QueryInterface", LogLevel::Error);
				WORD DataTypeLength{ sizeof(QWORD) };
				WORD StreamNum{};
				WMT_ATTR_DATATYPE DataTypeAttribute{};
				QWORD DurationInNano{};
				CheckHRESError(HeaderInfo->GetAttributeByName(&StreamNum, L"Duration", &DataTypeAttribute, reinterpret_cast<BYTE*>(&DurationInNano), &DataTypeLength), "GetAttributeByName", LogLevel::Error); //-V206
				// Round Duration to 3 decimal places ( = precision of 1ms)
				Duration = static_cast<double>(DurationInNano * 100) / 1000000000.0;

				CComPtr<IWMProfile> Profile{};
				CheckHRESError(SyncReader->QueryInterface(&Profile), "QueryInterface", LogLevel::Error);
				CComPtr<IWMStreamConfig> StreamConfig{};
				CheckHRESError(Profile->GetStream(0, &StreamConfig), "GetStream", LogLevel::Error);
				CComPtr<IWMMediaProps> MediaProperties;
				CheckHRESError(StreamConfig->QueryInterface(&MediaProperties), "QueryInterface", LogLevel::Error);

				DWORD MediaTypeSize{};
				CheckHRESError(MediaProperties->GetMediaType(nullptr, &MediaTypeSize), "GetMediaType", LogLevel::Error);
				std::vector<WM_MEDIA_TYPE> MediaType(MediaTypeSize);
				CheckHRESError(MediaProperties->GetMediaType(MediaType.data(), &MediaTypeSize), "GetMediaType", LogLevel::Error);

				WaveBuffer.resize(static_cast<std::size_t>(Duration* PCMFormat.nAvgBytesPerSec));

				HACMSTREAM ACMStream{};
				CheckMMRESError(acmStreamOpen(&ACMStream, nullptr, reinterpret_cast<LPWAVEFORMATEX>(&MP3Format), &PCMFormat, nullptr, 0, 0, 0), "acmStreamOpen", LogLevel::Error);
				DWORD RawBufferSize{};
				CheckMMRESError(acmStreamSize(ACMStream, MP3BlockSize, &RawBufferSize, ACM_STREAMSIZEF_SOURCE), "acmStreamSize", LogLevel::Error);

				std::array<BYTE, MP3BlockSize> MP3BlockBuffer{};
				std::vector<BYTE> RawBuffer(static_cast<std::size_t>(RawBufferSize));

				ACMSTREAMHEADER StreamHead{};
				StreamHead.cbStruct = sizeof(ACMSTREAMHEADER);
				StreamHead.pbSrc = MP3BlockBuffer.data();
				StreamHead.cbSrcLength = MP3BlockSize;
				StreamHead.pbDst = RawBuffer.data();
				StreamHead.cbDstLength = RawBufferSize;
				CheckMMRESError(acmStreamPrepareHeader(ACMStream, &StreamHead, 0), "acmStreamPrepareHeader", LogLevel::Error);

				ULARGE_INTEGER NewPosition{};
				LARGE_INTEGER SeekValue{};
				CheckHRESError(MP3Stream->Seek(SeekValue, STREAM_SEEK_SET, &NewPosition), "MP3StreamSeek", LogLevel::Error);

				std::size_t Offset{};

				while (true)
				{
					ULONG Counter{};
					CheckHRESError(MP3Stream->Read(MP3BlockBuffer.data(), MP3BlockSize, &Counter), "MP3StreamRead", LogLevel::Error);

					if (Counter != MP3BlockSize)
					{
						break;
					}

					CheckMMRESError(acmStreamConvert(ACMStream, &StreamHead, ACM_STREAMCONVERTF_BLOCKALIGN), "acmStreamConvert", LogLevel::Error);
					std::copy(RawBuffer.begin(), RawBuffer.begin() + static_cast<std::size_t>(StreamHead.cbDstLengthUsed), WaveBuffer.begin() + Offset);
					Offset += static_cast<std::size_t>(StreamHead.cbDstLengthUsed);
				}

				CheckMMRESError(acmStreamUnprepareHeader(ACMStream, &StreamHead, 0), "acmStreamUnprepareHeader", LogLevel::Error);
				CheckMMRESError(acmStreamClose(ACMStream, 0), "acmStreamClose", LogLevel::Error);
			}

			CoUninitialize();

			Playstate = State::Stopped;
		}
	}

	inline void MP3Player::Close()
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Closing audio: " + AudioName);

		if (!WaveBuffer.empty())
		{
			CheckMMRESError(waveOutReset(WaveOut), "waveOutReset", LogLevel::Warn);

			while (waveOutUnprepareHeader(WaveOut, &WaveHDR, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
			{
				Sleep(100);
			}

			WaveBuffer.clear();
			WaveBuffer.shrink_to_fit();

			CheckMMRESError(waveOutClose(WaveOut), "waveOutClose", LogLevel::Warn);

			Playstate = State::Stopped;
		}
		else
		{
			LWMFSystemLog.AddEntry(LogLevel::Warn, __FILENAME__, "No audio file was loaded!");
		}
	}

	inline void MP3Player::Play()
	{
		if (!WaveBuffer.empty())
		{
			WaveHDR = { reinterpret_cast<LPSTR>(WaveBuffer.data()), static_cast<DWORD>(WaveBuffer.size()) };
			waveOutOpen(&WaveOut, WAVE_MAPPER, &PCMFormat, NULL, 0, CALLBACK_NULL);
			waveOutPrepareHeader(WaveOut, &WaveHDR, sizeof(WAVEHDR));
			waveOutWrite(WaveOut, &WaveHDR, sizeof(WAVEHDR));
			Playstate = State::Playing;
		}
		else
		{
			LWMFSystemLog.AddEntry(LogLevel::Warn, __FILENAME__, "No audio file was loaded!");
		}
	}

	inline void MP3Player::Pause()
	{
		if (!WaveBuffer.empty() && Playstate == State::Playing)
		{
			waveOutPause(WaveOut);
			Playstate = State::Paused;
		}
	}

	inline void MP3Player::Restart()
	{
		if (!WaveBuffer.empty() && Playstate == State::Paused)
		{
			waveOutRestart(WaveOut);
			Playstate = State::Playing;
		}
	}

	inline double MP3Player::GetDuration()
	{
		if (!WaveBuffer.empty())
		{
			return Duration;
		}

		return 0;
	}

	inline double MP3Player::GetPosition()
	{
		if (!WaveBuffer.empty())
		{
			MMTIME MMTime{ TIME_SAMPLES, {} };
			waveOutGetPosition(WaveOut, &MMTime, sizeof(MMTIME));
			// Round Position to 3 decimal places ( = precision of 1ms)
			return std::round(static_cast<double>(MMTime.u.sample) / static_cast<double>(SampleRate) * 1000.0) / 1000.0;
		}

		return 0;
	}

	inline bool MP3Player::IsFinished()
	{
		if (!WaveBuffer.empty() && (Playstate == State::Playing || Playstate == State::Paused))
		{
			if (GetPosition() + 0.0001 <= Duration)
			{
				return true;
			}
		}

		Playstate = State::Finished;
		return false;
	}

	inline void MP3Player::SetVolume(const std::int_fast32_t LeftPercent, const std::int_fast32_t RightPercent)
	{
		if (!WaveBuffer.empty())
		{
			// 0xFFFF represents full volume, and a value of 0x0000 is silence
			const DWORD LeftVolume{ static_cast<DWORD>(0xFFFF * std::clamp(LeftPercent, 0, 100) / 100) };
			const DWORD RightVolume{ static_cast<DWORD>(0xFFFF * std::clamp(RightPercent, 0, 100) / 100) };

			waveOutSetVolume(WaveOut, LeftVolume + (RightVolume << 16));
		}
	}

	inline void MP3Player::CheckHRESError(const HRESULT Error, const std::string& Operation, const enum LogLevel Level)
	{
		if (Error != S_OK)
		{
			static std::map<HRESULT, std::string> ErrorTable
			{
				{ E_ABORT, "Operation aborted" },
				{ E_ACCESSDENIED, "General access denied error" },
				{ E_FAIL, "Unspecified failure" },
				{ E_HANDLE, "Handle that is not valid" },
				{ E_INVALIDARG, "One or more arguments are not valid" },
				{ E_NOINTERFACE, "No such interface supported" },
				{ E_NOTIMPL, "Not implemented" },
				{ E_OUTOFMEMORY, "Failed to allocate necessary memory" },
				{ E_POINTER, "Pointer that is not valid" },
				{ E_UNEXPECTED, "Unexpected failure" }
			};

			const std::map<HRESULT, std::string>::iterator ItErrorTable{ ErrorTable.find(Error) };

			if (ItErrorTable == ErrorTable.end())
			{
				LWMFSystemLog.AddEntry(Level, __FILENAME__, Operation + " HRESULT error: Unknown error - " + std::to_string(Error));
			}
			else
			{
				LWMFSystemLog.AddEntry(Level, __FILENAME__, Operation + " HRESULT error: " + ItErrorTable->second);
			}
		}
	}

	inline void MP3Player::CheckMMRESError(const MMRESULT Error, const std::string& Operation, const enum LogLevel Level)
	{
		if (Error != MMSYSERR_NOERROR)
		{
			std::array<char, 200> ErrorText{};
			waveOutGetErrorText(Error, ErrorText.data(), static_cast<UINT>(ErrorText.size()));
			LWMFSystemLog.AddEntry(Level, __FILENAME__, Operation + " MMRESULT error: " + std::string(ErrorText.data()));
		}
	}


} // namespace lwmf