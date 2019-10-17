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
#include "Tools_ErrorHandling.hpp"

namespace GFX_Window
{


	void Init();

	//
	// Functions
	//

	inline void Init()
	{
		if (const std::string INIFile{ GameConfigFolder + "WindowConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			// Create fullscreen if VSync = true, otherwise a window
			lwmf::CreateOpenGLWindow(lwmf::WindowInstance,
				ScreenTexture,
				lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "WINDOW", "ViewportWidth"),
				lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "WINDOW", "ViewportHeight"),
				lwmf::ReadINIValue<std::string>(INIFile, "WINDOW", "WindowName").c_str(), VSync);

			VSync ?	lwmf::SetVSync(-1) : lwmf::SetVSync(0);

			// Fullscreenflag is always true, since window cannot be resized - so we can create faster OpenGL textures
			lwmf::FullscreenFlag = true;
			lwmf::InitOpenGLLoader();

			ScreenTextureShader.LoadShader("Default", ScreenTexture);
			ScreenTextureShader.PrepareLWMFTexture(ScreenTexture, 0, 0);
		}
	}


} // namespace GFX_Window

