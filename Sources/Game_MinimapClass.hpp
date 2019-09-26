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
	if (const std::string INIFile{ "./DATA/GameConfig/HUDMinimapConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		Pos = { lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "PosX"), lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "PosY") };
		PlayerColor = lwmf::ReadINIValueRGBA(INIFile, "PLAYER");
		EnemyColor = lwmf::ReadINIValueRGBA(INIFile, "ENEMY");
		NeutralColor = lwmf::ReadINIValueRGBA(INIFile, "NEUTRAL");
		AmmoBoxColor = lwmf::ReadINIValueRGBA(INIFile, "AMMO");
		WallColor = lwmf::ReadINIValueRGBA(INIFile, "WALLS");
		DoorColor = lwmf::ReadINIValueRGBA(INIFile, "DOORS");
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
				lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, WallColor, WallColor);
			}

			if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Door)][MapPosX][MapPosY] != 0)
			{
				lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, DoorColor, DoorColor);
			}

			switch (Game_EntityHandling::EntityMap[MapPosX][MapPosY])
			{
				case Game_EntityHandling::EntityTypes::Player:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, PlayerColor, PlayerColor);
					break;
				}
				case Game_EntityHandling::EntityTypes::Enemy:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, EnemyColor, EnemyColor); //-V1037
					break;
				}
				case Game_EntityHandling::EntityTypes::Turret:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, EnemyColor, EnemyColor);
					break;
				}
				case Game_EntityHandling::EntityTypes::Neutral:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, NeutralColor, NeutralColor);
					break;
				}
				case Game_EntityHandling::EntityTypes::AmmoBox:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, IconRect.Width, IconRect.Height, AmmoBoxColor, AmmoBoxColor);
					break;
				}
				default: {}
			}
		}
	}
}