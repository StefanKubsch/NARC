/*
******************************************
*                                        *
* GFX_Window.hpp                         *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"

namespace GFX_Window
{


	void Init();

	//
	// Functions
	//

	inline void Init()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init window...");

		if (const std::string INIFile{ GameConfigFolder + "WindowConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			lwmf::CreateOpenGLWindow(lwmf::WindowInstance,
				ScreenTexture,
				lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "WINDOW", "ViewportWidth"),
				lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "WINDOW", "ViewportHeight"),
				lwmf::ReadINIValue<std::string>(INIFile, "WINDOW", "WindowName").c_str(), Fullscreen);

			VSync ?	lwmf::SetVSync(-1) : lwmf::SetVSync(0);

			// lwmf::Fullscreenflag is always true, since window cannot be resized - so we can create faster OpenGL textures
			lwmf::FullscreenFlag = true;
			lwmf::InitOpenGLLoader();

			// Prepare main shader and texture for rendering
			ScreenTextureShader.LoadShader("Default", ScreenTexture);
			ScreenTextureShader.PrepareLWMFTexture(ScreenTexture, 0, 0);

			// Inital clearance of window. Looks better while loading the rest of the game...
			lwmf::ClearBuffer();
			lwmf::SwapBuffer();
		}
	}


} // namespace GFX_Window

