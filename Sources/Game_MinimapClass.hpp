/*
******************************************
*                                        *
* Game_MinimapClass.hpp	                 *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>

#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"
#include "Game_DataStructures.hpp"
#include "Game_LevelHandling.hpp"
#include "Game_EntityHandling.hpp"

class Game_MinimapClass final
{
public:
	void Init();
	void PreRender();
	void Display();

	bool Enabled{ true };

private:
	lwmf::IntRectStruct IconRect{};
	lwmf::IntPointStruct Pos{};
	std::int_fast32_t StartPosY{};
	std::int_fast32_t PlayerColor{};
	std::int_fast32_t EnemyColor{};
	std::int_fast32_t NeutralColor{};
	std::int_fast32_t AmmoBoxColor{};
	std::int_fast32_t WallColor{};
	std::int_fast32_t DoorColor{};
};

inline void Game_MinimapClass::Init()
{
	if (const std::string INIFile{ "./DATA/GameConfig/HUDMinimapConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, ShowMessage, StopOnError))
	{
		Pos.X = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "GENERAL", "PosX");
		Pos.Y = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "GENERAL", "PosY");
		PlayerColor = Tools_INIFile::GetRGBColor(INIFile, "PLAYER");
		EnemyColor = Tools_INIFile::GetRGBColor(INIFile, "ENEMY");
		NeutralColor = Tools_INIFile::GetRGBColor(INIFile, "NEUTRAL");
		AmmoBoxColor = Tools_INIFile::GetRGBColor(INIFile, "AMMO");
		WallColor = Tools_INIFile::GetRGBColor(INIFile, "WALLS");
		DoorColor = Tools_INIFile::GetRGBColor(INIFile, "DOORS");
	}
}

inline void Game_MinimapClass::PreRender()
{
	std::int_fast32_t TileSize{ 6 };

	// The bigger the level, the smaller the tiles of the minimap...
	if (Game_LevelHandling::LevelMapWidth + Game_LevelHandling::LevelMapHeight <= 50)
	{
		TileSize = 10;
	}

	if (Game_LevelHandling::LevelMapWidth + Game_LevelHandling::LevelMapHeight > 50 && Game_LevelHandling::LevelMapWidth + Game_LevelHandling::LevelMapHeight <= 100)
	{
		TileSize = 8;
	}

	// ...and the lower the resolution, the smaller the whole map...
	if (TileSize > 6 && ScreenTexture.Width <= 640)
	{
		TileSize >>= 1;
	}

	// Generate the "icon" textures which indicate the entities...

	IconRect.Width = TileSize;
	IconRect.Height = TileSize;

	// Set map position
	StartPosY = ScreenTexture.Height - Game_LevelHandling::LevelMapWidth * TileSize - Pos.Y;
}

inline void Game_MinimapClass::Display()
{
	for (std::int_fast32_t x{ Pos.X }, MapPosY{}; MapPosY < Game_LevelHandling::LevelMapHeight; ++MapPosY, x += IconRect.Width)
	{
		for (std::int_fast32_t y{ StartPosY }, MapPosX{}; MapPosX < Game_LevelHandling::LevelMapWidth; ++MapPosX, y += IconRect.Height)
		{
			if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][MapPosX][MapPosY] != 0)
			{
				lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, WallColor);
			}

			if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Door)][MapPosX][MapPosY] != 0)
			{
				lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, DoorColor);
			}

			switch (Game_EntityHandling::EntityMap[MapPosX][MapPosY])
			{
				case Game_EntityHandling::EntityTypes::Player:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, PlayerColor);
					break;
				}
				case Game_EntityHandling::EntityTypes::Enemy:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, EnemyColor); //-V1037
					break;
				}
				case Game_EntityHandling::EntityTypes::Turret:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, EnemyColor);
					break;
				}
				case Game_EntityHandling::EntityTypes::Neutral:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, NeutralColor);
					break;
				}
				case Game_EntityHandling::EntityTypes::AmmoBox:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, AmmoBoxColor);
					break;
				}
				default: {}
			}
		}
	}
}