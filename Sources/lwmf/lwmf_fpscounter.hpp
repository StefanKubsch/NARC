/*
*******************************************************
*                                                     *
* lwmf_fpscounter - lightweight media framework       *
*                                                     *
* (C) 2019 - present by Stefan Kubsch                 *
*                                                     *
*******************************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <sysinfoapi.h>

#include "lwmf_text.hpp"

namespace lwmf
{


	void FPSCounter();
	void DisplayFPSCounter(TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Color);

	//
	// Variables and constants
	//

	inline std::uint_fast64_t FPSUpdate{};
	inline std::int_fast64_t FPSFrames{};
	inline std::int_fast64_t FPS{};

	//
	// Functions
	//

	inline void FPSCounter()
	{
		if (const std::uint_fast64_t SystemTime{ GetTickCount64() }; SystemTime - FPSUpdate >= 1000)
		{
			FPS = FPSFrames;
			FPSUpdate = SystemTime;
			FPSFrames = 0;
		}

		++FPSFrames;
	}

	inline void DisplayFPSCounter(TextureStruct& Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Color)
	{
		RenderText(Texture, "fps:" + std::to_string(FPS), PosX, PosY, Color);
	}


} // namespace lwmf