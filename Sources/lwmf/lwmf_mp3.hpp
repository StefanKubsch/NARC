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

		void Load(const std::string& Filename);
		void Play(PlayModes PlayMode);
		void Close();

		std::int_fast32_t Duration{};
		std::uint_fast32_t DeviceID{};
	private:
		void GetDuration();
		std::string GetMCIError(MCIERROR Error);
	};

	inline void MP3::Load(const std::string& Filename)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Load file " + Filename + "...");

		MCI_OPEN_PARMS OpenParams;
		OpenParams.lpstrDeviceType = "mpegvideo";
		OpenParams.lpstrElementName = Filename.c_str();

		mciSendCommand(0, MCI_OPEN, MCI_WAIT | MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_OPEN_SHAREABLE, reinterpret_cast<DWORD_PTR>(&OpenParams));

		DeviceID = OpenParams.wDeviceID;

		MCI_SET_PARMS SetParams;
		SetParams.dwCallback = NULL;
		SetParams.dwTimeFormat = 0;

		mciSendCommand(DeviceID, MCI_SET, MCI_SET_TIME_FORMAT, reinterpret_cast<DWORD_PTR>(&SetParams));
	}

	inline void MP3::Play(PlayModes PlayMode)
	{
		MCI_PLAY_PARMS PlayParams;
		PlayParams.dwCallback = reinterpret_cast<DWORD_PTR>(MainWindow);
		PlayParams.dwFrom = 0;

		MCIERROR Error{};

		switch (PlayMode)
		{
			case PlayModes::REPEAT:
			{
				Error = mciSendCommand(DeviceID, MCI_PLAY, MCI_NOTIFY, reinterpret_cast<DWORD_PTR>(&PlayParams));
				break;
			}
			case PlayModes::FROMSTART:
			{
				Error = mciSendCommand(DeviceID, MCI_PLAY, MCI_FROM, reinterpret_cast<DWORD_PTR>(&PlayParams));
				break;
			}
			case PlayModes::NOTIFY:
			{
				Error = mciSendCommand(DeviceID, MCI_PLAY, MCI_NOTIFY, reinterpret_cast<DWORD_PTR>(&PlayParams));
				break;
			}
			default: {}
		}

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error playing MCI device" + std::to_string(DeviceID) + "!" + GetMCIError(Error));
		}
	}

	inline void MP3::Close()
	{
		const MCIERROR Error{ mciSendCommand(DeviceID, MCI_CLOSE, 0, NULL) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Warn, __FILENAME__, "Error closing MCI device " + std::to_string(DeviceID) + "!" + GetMCIError(Error));
		}
	}

	inline void MP3::GetDuration()
	{
		MCI_STATUS_PARMS StatusParams;

		const MCIERROR Error{ mciSendCommand(DeviceID, MCI_STATUS, MCI_NOTIFY, reinterpret_cast<DWORD_PTR>(&StatusParams)) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error getting length of MCi device " + std::to_string(DeviceID) + "!" + GetMCIError(Error));
		}

		Duration = StatusParams.dwTrack;
	}

	inline std::string MP3::GetMCIError(const MCIERROR Error)
	{
		std::vector<char> szErrorBuf(MAXERRORLENGTH);
		return mciGetErrorString(Error, static_cast<LPSTR>(szErrorBuf.data()), MAXERRORLENGTH) ? " MCI error: " + std::string(szErrorBuf.data()) : " Unknown MCI error!";;
	}


} // namespace lwmf