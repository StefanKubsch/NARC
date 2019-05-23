/*
******************************************
*                                        *
* GFX_Window.hpp                         *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#define FMT_HEADER_ONLY

#include <cstdint>
#include "fmt/format.h"

#include "Game_GlobalDefinitions.hpp"
#include "Tools_Console.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"
#include "GFX_OpenGLShaderClass.hpp"

namespace GFX_Window
{


	void Init();

	//
	// Variables and constants
	//

	inline GFX_OpenGLShaderClass ScreenTextureShader;

	//
	// Functions
	//

	inline void Init()
	{
		if (const std::string INIFile{ "./DATA/GameConfig/WindowConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, ShowMessage, StopOnError))
		{
			Tools_Console::DisplayText(BRIGHT_MAGENTA, "\nCreating window...\n");

			// Create fullscreen if VSync = true, otherwise a window
			lwmf::CreateOpenGLWindow(WindowInstance,
				ScreenTexture,
				Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "WINDOW", "ViewportWidth"),
				Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "WINDOW", "ViewportHeight"),
				Tools_INIFile::ReadValue<std::string>(INIFile, "WINDOW", "WindowName").c_str(), VSync);

			VSync ?	lwmf::SetVSync(-1) : lwmf::SetVSync(0);

			lwmf::InitOpenGLLoader();

			ScreenTextureShader.LoadShader("Default");
			ScreenTextureShader.PrepareLWMFTexture(ScreenTexture, 0, 0);

			lwmf::RegisterRawInputDevice(lwmf::MainWindow, lwmf::HID_MOUSE);
			lwmf::RegisterRawInputDevice(lwmf::MainWindow, lwmf::HID_KEYBOARD);

			ShowCursor(FALSE);
		}
	}


} // namespace GFX_Window

