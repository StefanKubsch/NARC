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
#include <charconv>
#include <vector>
#include <sysinfoapi.h>

#include "lwmf_text.hpp"
#include "lwmf_texture.hpp"

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
		std::vector<char> FPSString(4);
		std::to_chars(FPSString.data(), FPSString.data() + FPSString.size(), FPS);
		RenderText(Texture, "fps:" + std::string(FPSString.data()), PosX, PosY, Color);
	}


} // namespace lwmf