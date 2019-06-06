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
		AddLogEntry("RAWINPUT: Register device " + std::to_string(Device) + "...");
		RAWINPUTDEVICE RawInputDevice;

		RawInputDevice.usUsagePage = 1;
		RawInputDevice.usUsage = Device;
		RawInputDevice.dwFlags = RIDEV_DEVNOTIFY;
		RawInputDevice.hwndTarget = hWnd;

		if (RegisterRawInputDevices(&RawInputDevice, 1, sizeof(RawInputDevice)) == 0)
		{
			LogErrorAndThrowException("Error registering raw input device " + std::to_string(Device) + "!");
		}
	}

	inline void UnregisterRawInputDevice(const USHORT Device)
	{
		AddLogEntry("RAWINPUT: Unregister device " + std::to_string(Device) + "...");
		RAWINPUTDEVICE RawInputDevice;

		RawInputDevice.usUsagePage = 1;
		RawInputDevice.usUsage = Device;
		RawInputDevice.dwFlags = RIDEV_REMOVE;
		RawInputDevice.hwndTarget = nullptr;

		if (RegisterRawInputDevices(&RawInputDevice, 1, sizeof(RawInputDevice)) == 0)
		{
			LogErrorAndThrowException("Error unregistering raw input device " + std::to_string(Device) + "!");
		}
	}

	inline void CatchMouse(const HWND hWnd)
	{
		static RECT WindowRect{};

		GetClientRect(hWnd, &WindowRect);

		POINT UpperLeft{ WindowRect.left, WindowRect.top };
		POINT LowerRight{ WindowRect.right, WindowRect.bottom };

		MapWindowPoints(hWnd, nullptr, &UpperLeft, 1);
		MapWindowPoints(hWnd, nullptr, &LowerRight, 1);

		WindowRect.left = UpperLeft.x;
		WindowRect.top = UpperLeft.y;
		WindowRect.right = LowerRight.x;
		WindowRect.bottom = LowerRight.y;

		ClipCursor(&WindowRect);
	}


} // namespace lwmf