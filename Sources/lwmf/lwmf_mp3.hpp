/*
****************************************************
*                                                  *
* lwmf_mp3 - lightweight media framework         *
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
#include <cstdint>

#pragma comment(lib, "Winmm.lib")

#include "lwm_logging.hpp"

namespace lwmf
{


	class MP3
	{
	public:
		enum class PlayModes
		{
			REPEAT,
			FROMSTART,
			NOTIFY
		};

		void Load(const std::string& Filename, const std::string& Handle);
		void Play(PlayModes PlayMode);
		void Close();

	private:
		std::int_fast32_t GetDuration();
		std::string GetMCIError(MCIERROR Error);

		std::string AudioHandle;
		std::int_fast32_t Duration{};
	};

	inline void MP3::Load(const std::string& Filename, const std::string& Handle)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Load file " + Filename + "...");

		AudioHandle = Handle;

		const MCIERROR Error{ mciSendString(("open \"" + Filename + "\" type mpegvideo alias " + AudioHandle).c_str(), nullptr, 0, nullptr) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error loading " + Filename + "!" + GetMCIError(Error));
		}

		Duration = GetDuration();
	}

	inline void MP3::Play(PlayModes PlayMode)
	{
		std::string PlayModeString;

		switch (PlayMode)
		{
			case PlayModes::REPEAT:
			{
				PlayModeString = " repeat";
				break;
			}
			case PlayModes::FROMSTART:
			{
				PlayModeString = " from 0";
				break;
			}
			case PlayModes::NOTIFY:
			{
				PlayModeString = " notify";
				break;
			}
			default: {}
		}

		const MCIERROR Error{ (PlayMode == PlayModes::NOTIFY) ? mciSendString(("play " + AudioHandle + PlayModeString).c_str(), nullptr, 0, MainWindow) : mciSendString(("play " + AudioHandle + PlayModeString).c_str(), nullptr, 0, nullptr) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error playing " + AudioHandle + "!" + GetMCIError(Error));
		}
	}

	inline void MP3::Close()
	{
		const MCIERROR Error{ mciSendString(("close " + AudioHandle).c_str(), nullptr, 0, nullptr) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Warn, __FILENAME__, "Problem closing " + AudioHandle + "!" + GetMCIError(Error));
		}
	}

	inline std::int_fast32_t MP3::GetDuration()
	{
		std::vector<char> Buffer(256);
		const MCIERROR Error{ mciSendString(("status " + AudioHandle + " length").c_str(), Buffer.data(), 256, nullptr) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error getting length of " + AudioHandle + "!" + GetMCIError(Error));
		}

		return std::stoi(Buffer.data());
	}

	inline std::string MP3::GetMCIError(const MCIERROR Error)
	{
		std::vector<char> szErrorBuf(MAXERRORLENGTH);
		return mciGetErrorString(Error, static_cast<LPSTR>(szErrorBuf.data()), MAXERRORLENGTH) ? " MCI error: " + std::string(szErrorBuf.data()) : " Unknown MCI error!";;
	}


} // namespace lwmf