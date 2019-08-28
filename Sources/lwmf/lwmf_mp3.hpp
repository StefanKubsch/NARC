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


	class MP3 final
	{
	public:
		enum class PlayModes : std::int_fast32_t
		{
			FROMSTART,
			NOTIFY
		};

		void Load(const std::string& Filename);
		void Play(PlayModes PlayMode);
		void RewindToStart();
		std::int_fast32_t GetDuration();
		std::uint_fast32_t GetDeviceID();
		void Close();

	private:
		std::string GetMCIError(MCIERROR Error);

		std::uint_fast32_t DeviceID{};

	};

	inline void MP3::Load(const std::string& Filename)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Load file " + Filename + "...");

		MCI_OPEN_PARMS OpenParams;
		OpenParams.dwCallback = NULL;
		OpenParams.lpstrDeviceType = std::to_string(MCI_ALL_DEVICE_ID).c_str();
		OpenParams.lpstrElementName = Filename.c_str();
		OpenParams.lpstrAlias = nullptr;

		mciSendCommand(NULL, MCI_OPEN, MCI_WAIT | MCI_OPEN_ELEMENT | MCI_OPEN_SHAREABLE, reinterpret_cast<DWORD_PTR>(&OpenParams));

		DeviceID = OpenParams.wDeviceID;

		MCI_SET_PARMS SetParams;
		SetParams.dwCallback = NULL;
		SetParams.dwTimeFormat = 0;

		mciSendCommand(DeviceID, MCI_SET, MCI_SET_TIME_FORMAT, reinterpret_cast<DWORD_PTR>(&SetParams));

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "File loaded with MCI device ID: " + std::to_string(DeviceID));
	}

	inline void MP3::Play(const PlayModes PlayMode)
	{
		MCI_PLAY_PARMS PlayParams;
		PlayParams.dwCallback = reinterpret_cast<DWORD_PTR>(MainWindow);

		MCIERROR Error{};

		switch (PlayMode)
		{
			case PlayModes::FROMSTART:
			{
				PlayParams.dwFrom = 0;
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
		MCI_SEEK_PARMS SeekParams;
		SeekParams.dwCallback = reinterpret_cast<DWORD_PTR>(MainWindow);
		mciSendCommand(DeviceID, MCI_SEEK, MCI_SEEK_TO_START, reinterpret_cast<DWORD_PTR>(&SeekParams));
	}

	inline std::int_fast32_t MP3::GetDuration()
	{
		MCI_STATUS_PARMS StatusParams;
		StatusParams.dwItem = MCI_STATUS_LENGTH;

		const MCIERROR Error{ mciSendCommand(DeviceID, MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, reinterpret_cast<DWORD_PTR>(&StatusParams)) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error getting length of MCI device ID: " + std::to_string(DeviceID) + "!" + GetMCIError(Error));
		}

		return static_cast<std::int_fast32_t>(StatusParams.dwReturn);
	}

	inline std::uint_fast32_t MP3::GetDeviceID()
	{
		return DeviceID;
	}

	inline void MP3::Close()
	{
		MCIERROR Error{ mciSendCommand(DeviceID, MCI_STOP, MCI_WAIT, NULL) };

		if (Error != 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Warn, __FILENAME__, "Error stopping MCI device ID: " + std::to_string(DeviceID) + "!" + GetMCIError(Error));
		}

		Error = mciSendCommand(DeviceID, MCI_CLOSE, MCI_WAIT, NULL);

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