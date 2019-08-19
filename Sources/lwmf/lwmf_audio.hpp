/*
****************************************************
*                                                  *
* lwmf_audio - lightweight media framework         *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <Windows.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include <string>

#pragma comment(lib, "Winmm.lib")

#include "lwm_logging.hpp"

namespace lwmf
{


	enum class AudioTypes
	{
		MP3,
		WAV
	};

	enum class AudioPlayModes
	{
		REPEAT,
		FROMSTART,
		NOTIFY
	};

	void LoadAudioFile(const std::string& Filename, AudioTypes AudioType, const std::string& AudioHandle);
	void PlayAudio(const std::string& AudioHandle, HWND WindowInstanceHandle, AudioPlayModes AudioPlayMode);
	std::int_fast32_t GetAudioLength(const std::string& AudioHandle);
	void CloseAudio(const std::string& AudioHandle);

	//
	// Variables and constants
	//


	//
	// Functions
	//

	inline void LoadAudioFile(const std::string& Filename, const AudioTypes AudioType, const std::string& AudioHandle)
	{
		std::string AudioTypeString;

		switch (AudioType)
		{
			case AudioTypes::MP3:
			{
				AudioTypeString = "mpegvideo";
				break;
			}
			case AudioTypes::WAV:
			{
				AudioTypeString = "waveaudio";
				break;
			}
			default: {}
		}

		const std::string LoadAudioString{ "open " + Filename + " type " + AudioTypeString + " alias " + AudioHandle };

		mciSendString(LoadAudioString.c_str(), nullptr, 0, nullptr);
	}

	inline void PlayAudio(const std::string& AudioHandle, const HWND WindowInstanceHandle, const AudioPlayModes AudioPlayMode)
	{
		std::string AudioPlayModeString;

		switch (AudioPlayMode)
		{
			case AudioPlayModes::REPEAT:
			{
				AudioPlayModeString = " repeat";
				break;
			}
			case AudioPlayModes::FROMSTART:
			{
				AudioPlayModeString = " from 0";
				break;
			}
			case AudioPlayModes::NOTIFY:
			{
				AudioPlayModeString = " notify";
				break;
			}
			default: {}
		}

		const std::string PlayAudioString{ "play " + AudioHandle + AudioPlayModeString };

		mciSendString(PlayAudioString.c_str(), nullptr, 0, WindowInstanceHandle);
	}

	inline std::int_fast32_t GetAudioLength(const std::string& AudioHandle)
	{
		std::string AudioLengthString{ "status " + AudioHandle + " length" };

		char* Buffer{ new char[256] };
		mciSendString(AudioLengthString.c_str(), Buffer, 256, nullptr);

		return std::stoi(Buffer);
	}

	inline void CloseAudio(const std::string& AudioHandle)
	{
		const std::string CloseAudioString{ "close " + AudioHandle };

		mciSendString(CloseAudioString.c_str(), nullptr, 0, nullptr);
	}

} // namespace lwmf