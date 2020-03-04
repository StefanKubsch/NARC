/*
*****************************************
*                                       *
* Game_SkyboxHandling.hpp               *
*                                       *
* (c) 2017 - 2020 Stefan Kubsch         *
*****************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <cmath>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "GFX_ImageHandling.hpp"

namespace Game_SkyboxHandling
{


	void Init();
	void LoadSkyboxImage();
	void Render();
	void ClearSkyBox();

	//
	// Variables and constants
	//

	static inline lwmf::ShaderClass SkyboxShader{};
	static inline std::int_fast32_t SkyboxWidth{};
	static inline std::int_fast32_t SkyboxHeight{};
	inline bool SkyBoxEnabled{};

	//
	// Functions
	//

	inline void Init()
	{
		SkyboxShader.LoadShader("Default", ScreenTexture);
	}

	inline void LoadSkyboxImage()
	{
		ClearSkyBox();

		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init skybox...");

		std::string INIFile{ LevelFolder };
		INIFile += std::to_string(SelectedLevel);
		INIFile += "/LevelData/SkyboxConfig.ini";

		if (Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			SkyBoxEnabled = lwmf::ReadINIValue<bool>(INIFile, "SKYBOX", "SkyBoxEnabled");

			if (SkyBoxEnabled)
			{
				NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Load skybox image...");

				const lwmf::TextureStruct TempTexture { GFX_ImageHandling::ImportImage(lwmf::ReadINIValue<std::string>(INIFile, "SKYBOX", "SkyBoxImageName"))	};

				SkyboxWidth = TempTexture.Width;
				SkyboxHeight = TempTexture.Height;

				NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Load skybox texture into GPU RAM...");
				SkyboxShader.LoadTextureInGPU(TempTexture, &SkyboxShader.OGLTextureID);
			}
		}
	}

	inline void Render()
	{
		if (SkyBoxEnabled)
		{
			const std::int_fast32_t Left{ static_cast<std::int_fast32_t>(std::atan2f(Plane.X, Plane.Y) / lwmf::DoublePI * -SkyboxWidth) };
			const std::int_fast32_t Top{ static_cast<std::int_fast32_t>(VerticalLookCamera * 360.0F - 180.0F) };

			SkyboxShader.RenderTexture(&SkyboxShader.OGLTextureID, Left, Top, SkyboxWidth, SkyboxHeight, false, 1.0F);

			if (Left < SkyboxWidth - ScreenTexture.Width)
			{
				SkyboxShader.RenderTexture(&SkyboxShader.OGLTextureID, Left - SkyboxWidth, Top, SkyboxWidth, SkyboxHeight, false, 1.0F);
			}
		}
	}

	inline void ClearSkyBox()
	{
		if (SkyBoxEnabled)
		{
			NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Delete skybox texture from GPU...");
			glDeleteTextures(1, &SkyboxShader.OGLTextureID);
		}
	}


} // namespace Game_SkyboxHandling

