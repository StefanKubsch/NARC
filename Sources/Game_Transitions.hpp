/*
*****************************************
*                                       *
* Game_Transitions.hpp                  *
*                                       *
* (c) 2017, 2018, 2019 Stefan Kubsch    *
*****************************************
*/

#pragma once

#include <cstdint>
#include <chrono>
#include <string>

#include "Game_GlobalDefinitions.hpp"
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
		const std::int_fast32_t BlackNoAlpha{ lwmf::RGBAtoINT(0, 0, 0, 0) };
		const std::string NextLevelText{ "...loading level number " + std::to_string(SelectedLevel) + "..." };
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "\n\n" + NextLevelText + "\n\n");

		lwmf::ClearBuffer();
		lwmf::ClearTexture(ScreenTexture, BlackNoAlpha);
		GeneralText.RenderTextCentered(NextLevelText, ScreenTexture.Height - GeneralText.GetFontHeight() - 50);
		ScreenTextureShader.RenderLWMFTexture(ScreenTexture, true, 1.0F);
		lwmf::SwapBuffer();
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
		std::uint_fast32_t Lag{};
		auto EndTime{ std::chrono::steady_clock::now() };

		while (Frame < LastFrame)
		{
			const auto StartTime{ std::chrono::steady_clock::now() };
			const auto ElapsedTime(std::chrono::duration_cast<std::chrono::milliseconds>(StartTime - EndTime));
			EndTime = StartTime;
			Lag += static_cast<std::uint_fast32_t>(ElapsedTime.count());

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
					lwmf::SetPixelSafe(ScreenTexture, FnResult % ScreenTexture.Width, static_cast<std::int_fast32_t>(FnResult / ScreenTexture.Width), FadeColor);
				}

				Lag -= LengthOfFrame;
			}

			lwmf::ClearBuffer();
			ScreenTextureShader.RenderLWMFTexture(ScreenTexture, true, 1.0F);
			lwmf::SwapBuffer();
		}
	}

	inline void DeathSequence()
	{
		const std::int_fast32_t Red{ lwmf::RGBAtoINT(255, 0, 0, 255) };
		const std::int_fast32_t Black{ lwmf::RGBAtoINT(0, 0, 0, 255) };

		lwmf::SetVSync(-1);
		FizzleFade(Red, 50);
		VSync ? lwmf::SetVSync(-1) : lwmf::SetVSync(0);

		GameOverText.RenderTextCentered("You are dead. Game over...", ScreenTexture.HeightMid - (GameOverText.GetFontHeight() >> 1));
		GameOverText1.RenderTextCentered("Press [SPACE] to continue", ScreenTexture.Height - GameOverText1.GetFontHeight() - 50);

		lwmf::SwapBuffer();

		if (HID_Keyboard::WaitForKeypress(VK_SPACE))
		{
			GamePausedFlag = true;
			lwmf::ClearTexture(ScreenTexture, Black);
		}
	}


} // namespace Game_Transitions

