/*
******************************************
*                                        *
* Game_Effects.hpp                       *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>

#include "Tools_ErrorHandling.hpp"

namespace Game_Effects
{


	void InitEffects();
	void StartBloodstainDrawing();
	void CountdownBloodstainCounter();
	void DrawBloodstain();

	//
	// Variables and constants
	//

	inline lwmf::ShaderClass BloodstainShader{};
	inline GLuint BloodstainTexture{};
	inline std::int_fast32_t BloodstainDuration{};
	inline std::int_fast32_t BloodstainCounter{};
	inline bool BloodstainFlag{};

	//
	// Functions
	//

	inline void InitEffects()
	{
		if (const std::string INIFile{ "./DATA/GameConfig/EffectsConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			// Init the "bloodstain" effect when player is hit by an enemy
			BloodstainDuration = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "EFFECTS", "BloodstainDuration");
			BloodstainShader.LoadShader("Default", ScreenTexture);

			const lwmf::TextureStruct TempTexture{ GFX_ImageHandling::ImportImage(lwmf::ReadINIValue<std::string>(INIFile, "TEXTURES", "Bloodstains")) };
			BloodstainShader.LoadStaticTextureInGPU(TempTexture, &BloodstainTexture, 0, 0, TempTexture.Width, TempTexture.Height);
		}
	}

	inline void StartBloodstainDrawing()
	{
		BloodstainFlag = true;
		BloodstainCounter = BloodstainDuration;
	}

	inline void CountdownBloodstainCounter()
	{
		if (--BloodstainCounter == 0)
		{
			BloodstainFlag = false;
		}
	}

	inline void DrawBloodstain()
	{
		if (BloodstainFlag)
		{
			BloodstainShader.RenderStaticTexture(&BloodstainTexture, true, std::clamp(0.5F - (0.5F / static_cast<float>(BloodstainCounter)), 0.0F, 0.5F));
		}
	}


} // namespace Game_Effects
