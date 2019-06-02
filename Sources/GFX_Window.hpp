/*
******************************************
*                                        *
* GFX_Window.hpp                         *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_Console.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"

namespace GFX_Window
{


	void Init();

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

			// Fullscreenflag is always true, since window cannot be resized - so we can create faster OpenGL textures
			lwmf::FullscreenFlag = 1;
			lwmf::InitOpenGLLoader();

			ScreenTextureShader.LoadShader("Default", ScreenTexture);
			ScreenTextureShader.PrepareLWMFTexture(ScreenTexture, 0, 0);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			lwmf::RegisterRawInputDevice(lwmf::MainWindow, lwmf::HID_MOUSE);
			lwmf::RegisterRawInputDevice(lwmf::MainWindow, lwmf::HID_KEYBOARD);

			ShowCursor(FALSE);
		}
	}


} // namespace GFX_Window

