/*
****************************************************
*                                                  *
* lwmf_mp3 - lightweight media framework           *
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
#include <vector>

#pragma comment(lib, "Winmm.lib")

#include "lwm_logging.hpp"

namespace lwmf
{


	enum class AudioPlayModes
	{
		REPEAT,
		FROMSTART,
		NOTIFY
	};

	void LoadMP3(const std::string& Filename, const std::string& MP3Handle);
	void PlayMP3(const std::string& MP3Handle, HWND Window, AudioPlayModes PlayMode);
	std::int_fast32_t GetMP3Length(const std::string& MP3Handle);
	void CloseMP3(const std::string& MP3Handle);
	std::string GetMCIError(MCIERROR Error);

	//
	// Functions
	//

	inline void LoadMP3(const std::string& Filename, const std::string& MP3Handle)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Load file " + Filename + "...");

		const MCIERROR Error{ mciSendString(("open \"" + Filename + "\" type mpegvideo alias " + MP3Handle).c_str(), nullptr, 0, nullptr) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error loading " + Filename + "!" + GetMCIError(Error));
		}
	}

	inline void PlayMP3(const std::string& MP3Handle, const HWND Window, const AudioPlayModes PlayMode)
	{
		std::string PlayModeString;

		switch (PlayMode)
		{
			case AudioPlayModes::REPEAT:
			{
				PlayModeString = " repeat";
				break;
			}
			case AudioPlayModes::FROMSTART:
			{
				PlayModeString = " from 0";
				break;
			}
			case AudioPlayModes::NOTIFY:
			{
				PlayModeString = " notify";
				break;
			}
			default: {}
		}

		const MCIERROR Error{ mciSendString(("play " + MP3Handle + PlayModeString).c_str(), nullptr, 0, Window) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error playing " + MP3Handle + "!" + GetMCIError(Error));
		}
	}

	inline std::int_fast32_t GetMP3Length(const std::string& MP3Handle)
	{
		std::vector<char> Buffer(256);
		const MCIERROR Error{ mciSendString(("status " + MP3Handle + " length").c_str(), Buffer.data(), 256, nullptr) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error getting length of " + MP3Handle + "!" + GetMCIError(Error));
		}

		return std::stoi(Buffer.data());
	}

	inline void CloseMP3(const std::string& MP3Handle)
	{
		const MCIERROR Error{ mciSendString(("close " + MP3Handle).c_str(), nullptr, 0, nullptr) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Warn, __FILENAME__, "Problem closing " + MP3Handle + "!" + GetMCIError(Error));
		}
	}

	inline std::string GetMCIError(const MCIERROR Error)
	{
		std::vector<char> szErrorBuf(MAXERRORLENGTH);
		return mciGetErrorString(Error, static_cast<LPSTR>(szErrorBuf.data()), MAXERRORLENGTH) ? " MCI error: " + std::string(szErrorBuf.data()) : " Unknown MCI error!";;
	}


} // namespace lwmf