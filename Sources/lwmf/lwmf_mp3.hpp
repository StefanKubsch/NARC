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
			FROMSTART,
			NOTIFY
		};

		void Load(const std::string& Filename);
		void Play(PlayModes PlayMode);
		void RewindToStart();
		std::int_fast32_t GetDuration();
		void Close();

		std::uint_fast32_t DeviceID{};
	private:
		std::string GetMCIError(MCIERROR Error);
	};

	inline void MP3::Load(const std::string& Filename)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Load file " + Filename + "...");

		MCI_OPEN_PARMS OpenParams;
		OpenParams.lpstrDeviceType = "mpegvideo";
		OpenParams.lpstrElementName = Filename.c_str();

		mciSendCommand(NULL, MCI_OPEN, MCI_WAIT | MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_OPEN_SHAREABLE, reinterpret_cast<DWORD_PTR>(&OpenParams));

		DeviceID = OpenParams.wDeviceID;
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "File loaded with MCI device ID: " + std::to_string(DeviceID));
	}

	inline void MP3::Play(const PlayModes PlayMode)
	{
		MCI_PLAY_PARMS PlayParams;
		PlayParams.dwCallback = reinterpret_cast<DWORD_PTR>(MainWindow);
		PlayParams.dwFrom = 0;

		MCIERROR Error{};

		switch (PlayMode)
		{
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
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error playing MCI device ID: " + std::to_string(DeviceID) + "!" + GetMCIError(Error));
		}
	}

	inline void MP3::RewindToStart()
	{
		mciSendCommand(DeviceID, MCI_SEEK, MCI_SEEK_TO_START, NULL);
	}

	inline std::int_fast32_t MP3::GetDuration()
	{
		MCI_STATUS_PARMS StatusParams;
		StatusParams.dwItem = MCI_STATUS_LENGTH;
		StatusParams.dwTrack = 1;

		const MCIERROR Error{ mciSendCommand(DeviceID, MCI_STATUS, MCI_WAIT | MCI_TRACK | MCI_STATUS_ITEM, reinterpret_cast<DWORD_PTR>(&StatusParams)) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error getting length of MCI device ID: " + std::to_string(DeviceID) + "!" + GetMCIError(Error));
		}

		return static_cast<std::int_fast32_t>(StatusParams.dwReturn);
	}

	inline void MP3::Close()
	{
		const MCIERROR Error{ mciSendCommand(DeviceID, MCI_CLOSE, 0, NULL) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Warn, __FILENAME__, "Error closing MCI device ID: " + std::to_string(DeviceID) + "!" + GetMCIError(Error));
		}
	}

	inline std::string MP3::GetMCIError(const MCIERROR Error)
	{
		std::vector<char> szErrorBuf(MAXERRORLENGTH);
		return mciGetErrorString(Error, static_cast<LPSTR>(szErrorBuf.data()), MAXERRORLENGTH) ? " MCI error: " + std::string(szErrorBuf.data()) : " Unknown MCI error!";;
	}


} // namespace lwmf