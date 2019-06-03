/*
******************************************
*                                        *
* Game_HealthBarClass.hpp	             *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"
#include "Game_PlayerClass.hpp"
#include "Game_DataStructures.hpp"

class Game_HealthBarClass final
{
public:
	void Init();
	void Display();

private:
	lwmf::IntRectStruct RectRed{};
	lwmf::IntRectStruct RectOrange{};
	lwmf::IntRectStruct RectBlack1{};
	lwmf::IntRectStruct RectBlack2{};

	lwmf::IntPointStruct Pos{};
	std::int_fast32_t HealthBarWidth{};
	std::int_fast32_t HealthBarFactor{};

	std::int_fast32_t Green{};
	std::int_fast32_t Red{};
	std::int_fast32_t Orange{};
	std::int_fast32_t Black{};
};

inline void Game_HealthBarClass::Init()
{
	if (const std::string INIFile{ "./DATA/GameConfig/HUDHealthBarConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		Pos.X = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "GENERAL", "PosX");
		Pos.Y = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "GENERAL", "PosY");
		HealthBarWidth = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "GENERAL", "HealthBarWidth");
		const std::int_fast32_t HealthBarLength{ Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "GENERAL", "HealthBarLength") };
		HealthBarFactor = HealthBarLength / 100;

		Green = Tools_INIFile::GetRGBColor(INIFile, "GREEN");
		Red = Tools_INIFile::GetRGBColor(INIFile, "RED");
		Orange = Tools_INIFile::GetRGBColor(INIFile, "ORANGE");
		Black = Tools_INIFile::GetRGBColor(INIFile, "BLACK");

		RectRed = { Pos.X, Pos.Y, HealthBarLength, HealthBarWidth };
		RectOrange = { Pos.X - 3, Pos.Y - 3, HealthBarLength + 6, HealthBarWidth + 6 };
		RectBlack1 = { Pos.X - 1, Pos.Y - 1, HealthBarLength + 1, HealthBarWidth + 1 };
		RectBlack2 = { Pos.X - 4, Pos.Y - 4, HealthBarLength + 8, HealthBarWidth + 7 };
	}
}

inline void Game_HealthBarClass::Display()
{
	const lwmf::IntRectStruct RectHealthGreen{ Pos.X, Pos.Y, Player.Hitpoints * HealthBarFactor, HealthBarWidth };

	lwmf::FilledRectangle(ScreenTexture, RectBlack2.X, RectBlack2.Y, RectBlack2.Width, RectBlack2.Height, Black);
	lwmf::FilledRectangle(ScreenTexture, RectOrange.X, RectOrange.Y, RectOrange.Width, RectOrange.Height, Orange);
	lwmf::FilledRectangle(ScreenTexture, RectBlack1.X, RectBlack1.Y, RectBlack1.Width, RectBlack1.Height, Black);
	lwmf::FilledRectangle(ScreenTexture, RectRed.X, RectRed.Y, RectRed.Width, RectRed.Height, Red);
	lwmf::FilledRectangle(ScreenTexture, RectHealthGreen.X, RectHealthGreen.Y, RectHealthGreen.Width, RectHealthGreen.Height, Green);
}

