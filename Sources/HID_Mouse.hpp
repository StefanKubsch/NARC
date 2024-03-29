/*
***********************************************
*                                             *
* HID_Mouse.hpp                               *
*                                             *
* (c) 2017 - 2020 Stefan Kubsch               *
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

	inline lwmf::ShaderClass MouseIconShader{};
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
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init mouse config...");

		if (const std::string INIFile{ GameConfigFolder + "InputConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			MouseSensitivityLowerLimit = lwmf::ReadINIValue<float>(INIFile, "MOUSE", "MouseSensitivityLowerLimit");
			MouseSensitivityUpperLimit = lwmf::ReadINIValue<float>(INIFile, "MOUSE", "MouseSensitivityUpperLimit");
			MouseSensitivityStep = lwmf::ReadINIValue<float>(INIFile, "MOUSE", "MouseSensitivityStep");

			MouseSensitivity = lwmf::ReadINIValue<float>(INIFile, "MOUSE", "MouseSensitivity");
			Tools_ErrorHandling::CheckAndClampRange(MouseSensitivity, MouseSensitivityLowerLimit, MouseSensitivityUpperLimit, __FILENAME__, "MouseSensitivity");

			// Load MouseIcon
			const lwmf::TextureStruct TempMouseIconTexture{ GFX_ImageHandling::ImportImage(lwmf::ReadINIValue<std::string>(INIFile, "MOUSE", "MouseIcon")) };
			MouseIconShader.LoadShader("Default", Canvas);
			MouseIconShader.LoadStaticTextureInGPU(TempMouseIconTexture, &MouseIconShader.OGLTextureID, Canvas.Width - 153, 0, TempMouseIconTexture.Width, TempMouseIconTexture.Height);

			lwmf::RegisterRawInputDevice(lwmf::MainWindow, lwmf::DeviceIdentifier::HID_MOUSE);
			ShowCursor(FALSE);
		}
	}

	inline void ChangeMouseSensitivity(const char Change)
	{
		if (Change == '+')
		{
			MouseSensitivity += MouseSensitivityStep;
		}
		else if (Change == '-')
		{
			MouseSensitivity -= MouseSensitivityStep;
		}

		Tools_ErrorHandling::CheckAndClampRange(MouseSensitivity, MouseSensitivityLowerLimit, MouseSensitivityUpperLimit, __FILENAME__, "MouseSensitivity");
	}


} // namespace HID_Mouse