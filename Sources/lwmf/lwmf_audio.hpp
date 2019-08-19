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

	void LoadAudioFile(const std::string& Filename, AudioTypes AudioType, const std::string& AudioHandle);

	//
	// Variables and constants
	//


	//
	// Functions
	//

	void LoadAudioFile(const std::string& Filename, const AudioTypes AudioType, const std::string& AudioHandle)
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

		const std::string LoadAudioString{ "open \\" + Filename + " type " + AudioTypeString + " alias " + AudioHandle };

		mciSendString(LoadAudioString.c_str(), nullptr, 0, nullptr);
	}


} // namespace lwmf