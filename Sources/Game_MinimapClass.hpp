/*
******************************************
*                                        *
* Game_MinimapClass.hpp	                 *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
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
	void DisplayRealtimeMap();
	static void DisplayPreRenderedMap();

	bool Enabled{ true };

private:
	void Clear();

	static inline lwmf::ShaderClass MiniMapShader{};

	lwmf::IntPointStruct Pos{};
	std::int_fast32_t TileSize{ 6 };
	std::int_fast32_t StartPosY{};
	std::int_fast32_t PlayerColor{};
	std::int_fast32_t EnemyColor{};
	std::int_fast32_t NeutralColor{};
	std::int_fast32_t AmmoBoxColor{};
	std::int_fast32_t WallColor{};
	std::int_fast32_t DoorColor{};
	std::int_fast32_t WayPointColor{};
	std::int_fast32_t WaypointOffset{};
	bool ShowWaypoints{};
	bool IsPreRendered{};
};

inline void Game_MinimapClass::Init()
{
	NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init minimap...");

	if (const std::string INIFile{ GameConfigFolder + "HUDMinimapConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		Pos = { lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "PosX"), lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "PosY") };
		ShowWaypoints = lwmf::ReadINIValue<bool>(INIFile, "GENERAL", "ShowWaypoints");
		PlayerColor = lwmf::ReadINIValueRGBA(INIFile, "PLAYER");
		EnemyColor = lwmf::ReadINIValueRGBA(INIFile, "ENEMY");
		NeutralColor = lwmf::ReadINIValueRGBA(INIFile, "NEUTRAL");
		AmmoBoxColor = lwmf::ReadINIValueRGBA(INIFile, "AMMO");
		WallColor = lwmf::ReadINIValueRGBA(INIFile, "WALLS");
		DoorColor = lwmf::ReadINIValueRGBA(INIFile, "DOORS");
		WayPointColor = lwmf::ReadINIValueRGBA(INIFile, "WAYPOINT");

		MiniMapShader.LoadShader("Default", ScreenTexture);
	}
}

inline void Game_MinimapClass::PreRender()
{
	Clear();

	NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Prerender minimap...");

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

	WaypointOffset = TileSize >> 1;

	// PreRender MiniMap
	lwmf::TextureStruct MiniMapTexture;
	lwmf::CreateTexture(MiniMapTexture, Game_LevelHandling::LevelMapHeight * TileSize, Game_LevelHandling::LevelMapWidth * TileSize, 0x000000FF);

	for (std::int_fast32_t x{}, MapPosY{}; MapPosY < Game_LevelHandling::LevelMapHeight; ++MapPosY, x += TileSize)
	{
		for (std::int_fast32_t y{}, MapPosX{}; MapPosX < Game_LevelHandling::LevelMapWidth; ++MapPosX, y += TileSize)
		{
			if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][MapPosX][MapPosY] != 0)
			{
				lwmf::FilledRectangle(MiniMapTexture, x, y, TileSize, TileSize, WallColor, WallColor);
			}

			if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Door)][MapPosX][MapPosY] != 0)
			{
				lwmf::FilledRectangle(MiniMapTexture, x, y, TileSize, TileSize, DoorColor, DoorColor);
			}
		}
	}

	// Set map position
	StartPosY = ScreenTexture.Height - Game_LevelHandling::LevelMapWidth * TileSize - Pos.Y;

	NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Load minimap texture into GPU RAM...");
	MiniMapShader.LoadStaticTextureInGPU(MiniMapTexture, &MiniMapShader.OGLTextureID, Pos.X, StartPosY, MiniMapTexture.Width, MiniMapTexture.Height);
	IsPreRendered = true;
}

inline void Game_MinimapClass::DisplayRealtimeMap()
{
	for (std::int_fast32_t x{ Pos.X }, MapPosY{}; MapPosY < Game_LevelHandling::LevelMapHeight; ++MapPosY, x += TileSize)
	{
		for (std::int_fast32_t y{ StartPosY }, MapPosX{}; MapPosX < Game_LevelHandling::LevelMapWidth; ++MapPosX, y += TileSize)
		{
			switch (Game_EntityHandling::EntityMap[MapPosX][MapPosY])
			{
				case EntityTypes::Player:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, TileSize, TileSize, PlayerColor, PlayerColor);
					break;
				}
				case EntityTypes::Enemy: case EntityTypes::Turret:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, TileSize, TileSize, EnemyColor, EnemyColor);
					break;
				}
				case EntityTypes::Neutral:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, TileSize, TileSize, NeutralColor, NeutralColor);
					break;
				}
				case EntityTypes::AmmoBox:
				{
					lwmf::FilledRectangle(ScreenTexture, x, y, TileSize, TileSize, AmmoBoxColor, AmmoBoxColor);
					break;
				}
				default: {}
			}

			if (ShowWaypoints)
			{
				for (auto& Entity : Entities)
				{
					if (!Entity.IsDead && (Entity.Type == EntityTypes::Neutral || Entity.Type == EntityTypes::Enemy))
					{
						for (auto& WayPoint : Entity.PathFindingWayPoints)
						{
							if (WayPoint.X == MapPosX && WayPoint.Y == MapPosY)
							{
								lwmf::SetPixel(ScreenTexture, x + WaypointOffset, y + WaypointOffset, WayPointColor);
								break;
							}
						}
					}
				}
			}
		}
	}
}

inline void Game_MinimapClass::DisplayPreRenderedMap()
{
	MiniMapShader.RenderStaticTexture(&MiniMapShader.OGLTextureID, true, 1.0F);
}

inline void Game_MinimapClass::Clear()
{
	if (IsPreRendered)
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Delete minimap texture from GPU...");
		glDeleteTextures(1, &MiniMapShader.OGLTextureID);
	}
}
