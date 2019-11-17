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

#include "Tools_ErrorHandling.hpp"
#include "Game_PlayerClass.hpp"

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
	std::string INIFile{ GameConfigFolder };
	INIFile += "HUDHealthBarConfig.ini";

	if (Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		Pos = { lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "PosX"), lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "PosY") };
		HealthBarWidth = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "HealthBarWidth");
		const std::int_fast32_t HealthBarLength{ lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "HealthBarLength") };
		HealthBarFactor = HealthBarLength / 100;

		Green = lwmf::ReadINIValueRGBA(INIFile, "GREEN");
		Red = lwmf::ReadINIValueRGBA(INIFile, "RED");
		Orange = lwmf::ReadINIValueRGBA(INIFile, "ORANGE");
		Black = lwmf::ReadINIValueRGBA(INIFile, "BLACK");

		RectRed = { Pos.X, Pos.Y, HealthBarLength, HealthBarWidth };
		RectOrange = { Pos.X - 3, Pos.Y - 3, HealthBarLength + 6, HealthBarWidth + 6 };
		RectBlack1 = { Pos.X - 1, Pos.Y - 1, HealthBarLength + 2, HealthBarWidth + 2 };
		RectBlack2 = { Pos.X - 4, Pos.Y - 4, HealthBarLength + 8, HealthBarWidth + 8 };
	}
}

inline void Game_HealthBarClass::Display()
{
	lwmf::Rectangle(ScreenTexture, RectBlack2.X, RectBlack2.Y, RectBlack2.Width, RectBlack2.Height, Black);
	lwmf::FilledRectangle(ScreenTexture, RectOrange.X, RectOrange.Y, RectOrange.Width, RectOrange.Height, Orange, Orange);
	lwmf::Rectangle(ScreenTexture, RectBlack1.X, RectBlack1.Y, RectBlack1.Width, RectBlack1.Height, Black);
	lwmf::FilledRectangle(ScreenTexture, RectRed.X, RectRed.Y, RectRed.Width, RectRed.Height, Red, Red);
	lwmf::FilledRectangle(ScreenTexture, Pos.X, Pos.Y, Player.Hitpoints * HealthBarFactor, HealthBarWidth, Green, Green);
}