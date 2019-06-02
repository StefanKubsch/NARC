/*
*****************************************
*                                       *
* Game_Transitions.hpp                  *
*                                       *
* (c) 2017, 2018, 2019 Stefan Kubsch    *
*****************************************
*/

#pragma once

#define FMT_HEADER_ONLY

#include <cstdint>
#include <chrono>
#include <string>
#include "fmt/format.h"

#include "Game_GlobalDefinitions.hpp"
#include "Tools_Console.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"
#include "GFX_Window.hpp"
#include "GFX_TextClass.hpp"
#include "HID_Keyboard.hpp"

namespace Game_Transitions
{


	void Init();
	void LevelTransition();
	void FizzleFade(std::int_fast32_t FadeColor, std::int_fast32_t Speed);
	void DeathSequence();

	//
	// Variables and constants
	//

	inline GFX_TextClass GeneralText{};
	inline GFX_TextClass GameOverText{};
	inline GFX_TextClass GameOverText1{};

	//
	// Functions
	//

	inline void Init()
	{
		GeneralText.InitFont("./DATA/GameConfig/TransitionsConfig.ini", "GENERALFONT");
		GameOverText.InitFont("./DATA/GameConfig/TransitionsConfig.ini", "GAMEOVERFONT");
		GameOverText1.InitFont("./DATA/GameConfig/TransitionsConfig.ini", "GAMEOVERFONT1");
	}

	inline void LevelTransition()
	{
		const std::string NextLevelText{ fmt::format("...loading level number {}...", SelectedLevel) };
		Tools_Console::DisplayText(BRIGHT_WHITE, fmt::format("\n{}\n\n", NextLevelText));

		lwmf::ClearBuffer();
		lwmf::ClearTexture(ScreenTexture, 0);
		GeneralText.RenderTextCentered(NextLevelText, ScreenTexture.Height - GeneralText.FontHeight - 50);
		ScreenTextureShader.RenderLWMFTexture(ScreenTexture);
		SwapBuffers(lwmf::WindowHandle);
	}

	inline void FizzleFade(const std::int_fast32_t FadeColor, const std::int_fast32_t Speed)
	{
		//
		// This is my implementation of the famous "fizzle fade" as known from Wolfenstein 3D
		//
		// See explanation here:
		//
	    // http://fabiensanglard.net/fizzlefade/index.php
		//
		// I use a Feistel Network algorithm, get a basic idea here:
		//
		// https://jacopretorius.net/2017/09/fizzlefade-using-a-feistel-network.html
		//
		// My version is screensize & framerate-independent since I calculate the neccessary bits and use a gameloop...
		//

		const std::int_fast32_t Bits{ static_cast<std::int_fast32_t>(std::ceilf(std::log2f(static_cast<float>(ScreenTexture.Width * ScreenTexture.Height)) * 0.5F)) << 1 };
		const std::int_fast32_t LastFrame{ static_cast<std::int_fast32_t>(std::pow(2, Bits)) };
		const std::int_fast32_t HalfBits{ Bits >> 1 };
		const std::int_fast32_t HalfMask{ static_cast<std::int_fast32_t>(std::pow(2, HalfBits)) - 1 };
		std::int_fast32_t Frame{};
		std::uint_fast64_t Lag{};
		std::chrono::time_point<std::chrono::steady_clock> EndTime{ std::chrono::steady_clock::now() };

		while (Frame < LastFrame)
		{
			std::chrono::time_point<std::chrono::steady_clock> StartTime{ std::chrono::steady_clock::now() };
			auto ElapsedTime(std::chrono::duration_cast<std::chrono::milliseconds>(StartTime - EndTime));
			EndTime = StartTime;
			Lag += static_cast<std::uint_fast64_t>(ElapsedTime.count());

			while (Lag >= LengthOfFrame)
			{
				for (std::int_fast32_t j{}; j < LastFrame / Speed; ++j, ++Frame)
				{
					std::int_fast32_t Left{ Frame & HalfMask };
					std::int_fast32_t Right{ Frame >> HalfBits };

					for (std::int_fast32_t i{}; i < 5; ++i)
					{
						const std::int_fast32_t NewLeft{ Right };
						const std::int_fast32_t Fn{ (Right * 19 + (Right >> 1) ^ Right) & HalfMask };
						Right = Left ^ Fn;
						Left = NewLeft;
					}

					const std::int_fast32_t FnResult{ Right << HalfBits | (Left & (LastFrame - 1)) };
					const lwmf::IntPointStruct Pixel{FnResult % ScreenTexture.Width, static_cast<std::int_fast32_t>(FnResult / ScreenTexture.Width) };

					if (Pixel.X < ScreenTexture.Width && Pixel.Y < ScreenTexture.Height)
					{
						lwmf::SetPixel(ScreenTexture, Pixel.X, Pixel.Y, FadeColor);
					}
				}

				Lag -= LengthOfFrame;
			}

			lwmf::ClearBuffer();
			ScreenTextureShader.RenderLWMFTexture(ScreenTexture);
			SwapBuffers(lwmf::WindowHandle);
		}
	}

	inline void DeathSequence()
	{
		FizzleFade(0xFF0000FF, 50);

		GameOverText.RenderTextCentered("You are dead. Game over...", ScreenTexture.HeightMid - (GameOverText.FontHeight >> 1));
		GameOverText1.RenderTextCentered("Press any key to continue", ScreenTexture.Height - GameOverText1.FontHeight - 50);

		SwapBuffers(lwmf::WindowHandle);

		if (HID_Keyboard::WaitForKeypress())
		{
			GamePausedFlag = true;
			lwmf::ClearTexture(ScreenTexture, 0);
		}
	}


} // namespace Game_Transitions

