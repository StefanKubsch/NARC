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
#include <SDL.h>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"

namespace HID_Mouse
{


	void Init();
	void ChangeMouseSensitivity(char Change);

	//
	// Variables and constants
	//

	inline SDL_Point MousePos{};
	inline SDL_Point OldMousePos{};

	inline float MouseSensitivity{};
	inline float MouseSensitivityLowerLimit{};
	inline float MouseSensitivityUpperLimit{};
	inline float MouseSensitivityStep{};

	//
	// Functions
	//

	inline void Init()
	{
		if (const std::string INIFile{ "./DATA/GameConfig/InputConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, ShowMessage, StopOnError))
		{
			MouseSensitivity = Tools_INIFile::ReadValue<float>(INIFile, "MOUSE", "MouseSensitivity");
			MouseSensitivityLowerLimit = Tools_INIFile::ReadValue<float>(INIFile, "MOUSE", "MouseSensitivityLowerLimit");
			MouseSensitivityUpperLimit = Tools_INIFile::ReadValue<float>(INIFile, "MOUSE", "MouseSensitivityUpperLimit");
			MouseSensitivityStep = Tools_INIFile::ReadValue<float>(INIFile, "MOUSE", "MouseSensitivityStep");
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