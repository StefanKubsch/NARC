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

#define NOMINMAX
#include <Windows.h>
#include <string>
#include <map>

#include "lwmf_logging.hpp"

namespace lwmf
{


	// Device identifier
	enum class DeviceIdentifier : USHORT
	{
		HID_MOUSE		= 2,
		HID_KEYBOARD	= 6
	};

	std::string DeviceString(DeviceIdentifier Device);
	void RegisterRawInputDevice(HWND hWnd, DeviceIdentifier Device);
	void UnregisterRawInputDevice(DeviceIdentifier Device);
	void CatchMouse(HWND hWnd);

	//
	// Functions
	//

	inline std::string DeviceString(const DeviceIdentifier Device)
	{
		std::map<DeviceIdentifier, std::string> DeviceTable
		{
			{ DeviceIdentifier::HID_MOUSE, "HID_MOUSE (DeviceIdentifier 2)" },
			{ DeviceIdentifier::HID_KEYBOARD, "HID_KEYBOARD (DeviceIdentifier 6)" },
		};

		const std::map<DeviceIdentifier, std::string>::iterator ItDeviceTable{ DeviceTable.find(Device) };
		std::string DevString;

		if (ItDeviceTable == DeviceTable.end())
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, __LINE__, "Unknown HID device identifier!");
		}
		else
		{
			DevString = ItDeviceTable->second;
		}

		return DevString;
	}

	inline void RegisterRawInputDevice(HWND hWnd, const DeviceIdentifier Device)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, "Register device " + DeviceString(Device) + "...");

		RAWINPUTDEVICE RawInputDevice{};
		RawInputDevice.usUsagePage = 1;
		RawInputDevice.usUsage = static_cast<USHORT>(Device);
		RawInputDevice.dwFlags = RIDEV_DEVNOTIFY;
		RawInputDevice.hwndTarget = hWnd;

		RegisterRawInputDevices(&RawInputDevice, 1, sizeof(RAWINPUTDEVICE)) == 0 ? LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, __LINE__, "Error registering raw input device " + DeviceString(Device) + "!") :
			LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, "Successfully registered device " + DeviceString(Device));
	}

	inline void UnregisterRawInputDevice(const DeviceIdentifier Device)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, "Unregister device " + DeviceString(Device) + "...");

		RAWINPUTDEVICE RawInputDevice{};
		RawInputDevice.usUsagePage = 1;
		RawInputDevice.usUsage = static_cast<USHORT>(Device);
		RawInputDevice.dwFlags = RIDEV_REMOVE;
		RawInputDevice.hwndTarget = nullptr;

		RegisterRawInputDevices(&RawInputDevice, 1, sizeof(RAWINPUTDEVICE)) == 0 ? LWMFSystemLog.AddEntry(LogLevel::Warn, __FILENAME__, __LINE__, "Error unregistering raw input device " + DeviceString(Device) + "!") :
			LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, "Successfully unregistered device " + DeviceString(Device));
	}

	inline void CatchMouse(HWND hWnd)
	{
		RECT WindowRect{};
		GetClientRect(hWnd, &WindowRect);
		MapWindowPoints(hWnd, nullptr, reinterpret_cast<LPPOINT>(&WindowRect), 2);
		ClipCursor(&WindowRect);
	}


} // namespace lwmf