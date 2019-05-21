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

	inline GFX_OpenGLShaderClass PixelBufferToScreen;

	//
	// Functions
	//

	inline void Init()
	{
		if (const std::string INIFile{ "./DATA/GameConfig/WindowConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, ShowMessage, StopOnError))
		{
			Tools_Console::DisplayText(BRIGHT_MAGENTA, "\nCreating window...\n");

			if (VSync)
			{
				// Create fullscreen "window"
				lwmf::CreateOpenGLWindow(GetModuleHandle(nullptr),
					Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "WINDOW", "ViewportWidth"),
					Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "WINDOW", "ViewportHeight"),
					Tools_INIFile::ReadValue<std::string>(INIFile, "WINDOW", "WindowName").c_str(), true);

				// -1 means "adaptive vsync"
				lwmf::SetVSync(-1);
			}
			else
			{
				// create normal window
				lwmf::CreateOpenGLWindow(GetModuleHandle(nullptr),
					Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "WINDOW", "ViewportWidth"),
					Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "WINDOW", "ViewportHeight"),
					Tools_INIFile::ReadValue<std::string>(INIFile, "WINDOW", "WindowName").c_str(), false);

				// 0 means no vsync
				lwmf::SetVSync(0);
			}

			lwmf::InitOpenGLLoader();

			PixelBufferToScreen.LoadShader("Default");
			PixelBufferToScreen.PreparePixelBufferTexture(0, 0, lwmf::ViewportWidth, lwmf::ViewportHeight);

			lwmf::RegisterRawInputDevice(lwmf::MainWindow, lwmf::HID_MOUSE);
			lwmf::RegisterRawInputDevice(lwmf::MainWindow, lwmf::HID_KEYBOARD);

			ShowCursor(FALSE);
		}
	}


} // namespace GFX_Window

