/*
*****************************************
*                                       *
* Game_SkyboxHandling.hpp               *
*                                       *
* (c) 2017, 2018, 2019 Stefan Kubsch    *
*****************************************
*/

#pragma once

#define FMT_HEADER_ONLY

#include <cstdint>
#include <string>
#include <SDL.h>
#include "fmt/format.h"

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"
#include "GFX_ImageHandling.hpp"
#include "GFX_OpenGLShaderClass.hpp"

namespace Game_SkyboxHandling
{


	void Init();
	void LoadSkyboxImage();
	void Render();
	void ClearSkyBox();

	//
	// Variables and constants
	//

	inline GFX_OpenGLShaderClass SkyboxShader{};
	inline GLuint SkyboxTexture{};
	inline std::int_fast32_t SkyboxWidth{};
	inline std::int_fast32_t SkyboxHeight{};
	inline bool SkyBoxEnabled{};

	//
	// Functions
	//

	inline void Init()
	{
		SkyboxShader.LoadShader("Default");
	}

	inline void LoadSkyboxImage()
	{
		if (const std::string INIFile{ fmt::format("./DATA/Level_{}/LevelData/SkyboxConfig.ini", SelectedLevel) }; Tools_ErrorHandling::CheckFileExistence(INIFile, ShowMessage, StopOnError))
		{
			SkyBoxEnabled = Tools_INIFile::ReadValue<bool>(INIFile, "SKYBOX", "SkyBoxEnabled");

			if (SkyBoxEnabled)
			{
				const TextureStruct TempTexture { GFX_ImageHandling::ImportImage(Tools_INIFile::ReadValue<std::string>(INIFile, "SKYBOX", "SkyBoxImageName"))	};

				SkyboxWidth = TempTexture.Width;
				SkyboxHeight = TempTexture.Height;

				SkyboxShader.LoadTextureInGPU(TempTexture, &SkyboxTexture);
			}
		}
	}

	inline void Render()
	{
		if (SkyBoxEnabled)
		{
			const std::int_fast32_t Left{ static_cast<std::int_fast32_t>(std::atan2f(Plane.X, Plane.Y) / lwmf::DoublePI * -SkyboxWidth) };
			const std::int_fast32_t Top{ static_cast<std::int_fast32_t>(VerticalLookCamera * 360.0F - 180.0F) };

			SkyboxShader.RenderTexture(&SkyboxTexture, Left, Top, SkyboxWidth, SkyboxHeight);

			if (Left < SkyboxWidth - lwmf::ViewportWidth)
			{
				SkyboxShader.RenderTexture(&SkyboxTexture, Left - SkyboxWidth, Top, SkyboxWidth, SkyboxHeight);
			}
		}
	}

	inline void ClearSkyBox()
	{
		if (SkyBoxEnabled)
		{
			glDeleteTextures(1, &SkyboxTexture);
		}
	}


} // namespace Game_SkyboxHandling

