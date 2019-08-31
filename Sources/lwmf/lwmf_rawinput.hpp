/*
***************************************************
*                                                 *
* lwmf_rawinput - lightweight media framework     *
*                                                 *
* (C) 2019 - present by Stefan Kubsch             *
*                                                 *
***************************************************
*/

#pragma once

#include <Windows.h>

#include "lwm_logging.hpp"

namespace lwmf
{


	// Device identifier
	enum class DeviceIdentifier : USHORT
	{
		HID_MOUSE		= 2,
		HID_KEYBOARD	= 6
	};

	void RegisterRawInputDevice(HWND hWnd, DeviceIdentifier Device);
	void UnregisterRawInputDevice(DeviceIdentifier Device);
	void CatchMouse(HWND hWnd);

	//
	// Functions
	//

	inline void RegisterRawInputDevice(const HWND hWnd, const DeviceIdentifier Device)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Register device " + std::to_string(static_cast<USHORT>(Device)) + "...");

		RAWINPUTDEVICE RawInputDevice;
		RawInputDevice.usUsagePage = 1;
		RawInputDevice.usUsage = static_cast<USHORT>(Device);
		RawInputDevice.dwFlags = RIDEV_DEVNOTIFY;
		RawInputDevice.hwndTarget = hWnd;

		if (RegisterRawInputDevices(&RawInputDevice, 1, sizeof(RawInputDevice)) == 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "Error registering raw input device " + std::to_string(static_cast<USHORT>(Device)) + "!");
		}
	}

	inline void UnregisterRawInputDevice(const DeviceIdentifier Device)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Unregister device " + std::to_string(static_cast<USHORT>(Device)) + "...");

		RAWINPUTDEVICE RawInputDevice;
		RawInputDevice.usUsagePage = 1;
		RawInputDevice.usUsage = static_cast<USHORT>(Device);
		RawInputDevice.dwFlags = RIDEV_REMOVE;
		RawInputDevice.hwndTarget = nullptr;

		if (RegisterRawInputDevices(&RawInputDevice, 1, sizeof(RawInputDevice)) == 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Warn, __FILENAME__, "Error unregistering raw input device " + std::to_string(static_cast<USHORT>(Device)) + "!");
		}
	}

	inline void CatchMouse(const HWND hWnd)
	{
		static RECT WindowRect{};
		GetClientRect(hWnd, &WindowRect);
		MapWindowPoints(hWnd, nullptr, reinterpret_cast<LPPOINT>(&WindowRect), 2);
		ClipCursor(&WindowRect);
	}


} // namespace lwmf