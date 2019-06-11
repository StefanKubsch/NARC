/*
***********************************************
*                                             *
* HID_Mouse.hpp                               *
*                                             *
* (c) 2017, 2018, 2019 Stefan Kubsch          *
***********************************************
*/

#pragma once

#include <string>

#include "Tools_ErrorHandling.hpp"

namespace HID_Mouse
{


	void Init();
	void ChangeMouseSensitivity(char Change);

	//
	// Variables and constants
	//

	inline lwmf::IntPointStruct MousePos{};
	inline lwmf::IntPointStruct OldMousePos{};

	inline float MouseSensitivity{};
	inline float MouseSensitivityLowerLimit{};
	inline float MouseSensitivityUpperLimit{};
	inline float MouseSensitivityStep{};

	//
	// Functions
	//

	inline void Init()
	{
		if (const std::string INIFile{ "./DATA/GameConfig/InputConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			MouseSensitivity = lwmf::ReadINIValue<float>(INIFile, "MOUSE", "MouseSensitivity");
			MouseSensitivityLowerLimit = lwmf::ReadINIValue<float>(INIFile, "MOUSE", "MouseSensitivityLowerLimit");
			MouseSensitivityUpperLimit = lwmf::ReadINIValue<float>(INIFile, "MOUSE", "MouseSensitivityUpperLimit");
			MouseSensitivityStep = lwmf::ReadINIValue<float>(INIFile, "MOUSE", "MouseSensitivityStep");

			lwmf::RegisterRawInputDevice(lwmf::MainWindow, lwmf::HID_MOUSE);
			ShowCursor(FALSE);
		}
	}

	inline void ChangeMouseSensitivity(const char Change)
	{
		if (Change == '+' && MouseSensitivity < MouseSensitivityUpperLimit)
		{
			MouseSensitivity += MouseSensitivityStep;
		}
		else if (Change == '-' && MouseSensitivity > MouseSensitivityLowerLimit)
		{
			MouseSensitivity -= MouseSensitivityStep;
		}
	}


} // namespace HID_Mouse