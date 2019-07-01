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


	void RegisterRawInputDevice(HWND hWnd, USHORT Device);
	void UnregisterRawInputDevice(USHORT Device);
	void CatchMouse(HWND hWnd);

	//
	// Variables and constants
	//

	// Device identifier
	constexpr USHORT HID_MOUSE{ 2 };
	constexpr USHORT HID_KEYBOARD{ 6 };

	//
	// Functions
	//

	inline void RegisterRawInputDevice(const HWND hWnd, const USHORT Device)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Register device " + std::to_string(Device) + "...");
		RAWINPUTDEVICE RawInputDevice;

		RawInputDevice.usUsagePage = 1;
		RawInputDevice.usUsage = Device;
		RawInputDevice.dwFlags = RIDEV_DEVNOTIFY;
		RawInputDevice.hwndTarget = hWnd;

		if (RegisterRawInputDevices(&RawInputDevice, 1, sizeof(RawInputDevice)) == 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "Error registering raw input device " + std::to_string(Device) + "!");
		}
	}

	inline void UnregisterRawInputDevice(const USHORT Device)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Unregister device " + std::to_string(Device) + "...");
		RAWINPUTDEVICE RawInputDevice;

		RawInputDevice.usUsagePage = 1;
		RawInputDevice.usUsage = Device;
		RawInputDevice.dwFlags = RIDEV_REMOVE;
		RawInputDevice.hwndTarget = nullptr;

		if (RegisterRawInputDevices(&RawInputDevice, 1, sizeof(RawInputDevice)) == 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error unregistering raw input device " + std::to_string(Device) + "!");
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