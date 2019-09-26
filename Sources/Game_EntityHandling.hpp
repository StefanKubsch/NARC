/*
******************************************
*                                        *
* Game_EntityHandling.hpp                *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <random>
#include <algorithm>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Game_DataStructures.hpp"
#include "GFX_ImageHandling.hpp"
#include "Game_LevelHandling.hpp"
#include "Game_PathFinding.hpp"

namespace Game_EntityHandling
{


	enum class EntityTypes : std::int_fast32_t
	{
		Clear,
		Neutral,
		Enemy,
		Player,
		AmmoBox,
		Turret,
		Count
	};

	enum class EntitySounds : std::int_fast32_t
	{
		Kill			= 0,
		AmmoBoxPickup	= 0,
		Attack			= 1
	};

	enum class SortOrder : std::int_fast32_t
	{
		FrontToBack,
		BackToFront
	};

	void InitEntityAssets();
	void LoadWalkAnimTextures(std::int_fast32_t AssetIndex, const std::string& AssetTypeName);
	void LoadAdditionalAnimTextures(const std::string& AnimType, const std::string& AssetTypeName, std::vector<lwmf::TextureStruct>& AnimVector);
	void InitEntities();
	void RenderEntities();
	std::int_fast32_t GetEntityTextureIndex(std::int_fast32_t EntityNumber);
	void HandleEntityHit(EntityStruct& Entity);
	void ChangeEntityDirection(EntityStruct& Entity, char NewDirection);
	void TurnEntityBackwards(EntityStruct& Entity);
	void CalculateEntityPath(EntityStruct& Entity);
	void MoveEntities();
	void GetEntityDistance();
	void SortEntities(SortOrder SortOrder);
	void MarkEntityPositionOnMap(const EntityStruct& Entity);
	void PlayAudio(std::int_fast32_t TypeNumber, EntitySounds EntitySound);
	void CloseAudio();

	//
	// Variables and constants
	//

	// Populate the Mersenne-Twister-Random Engine with proper distributions
	inline static const std::uniform_int_distribution<std::int_fast32_t> Distribution200(1, 200);
	inline static const std::uniform_int_distribution<std::int_fast32_t> Distribution666(1, 666);

	static constexpr float EntityCollisionDetectionWallDist{ 0.5F };

	inline std::vector<std::vector<EntityTypes>> EntityMap;

	// Vectors used to sort the entities
	inline std::vector<std::int_fast32_t> EntityOrder;
	inline std::vector<float> EntityDistance;

	// 1D Zbuffer
	inline std::vector<float> ZBuffer;

	//
	// Functions
	//

	inline void InitEntityAssets()
	{
		EntityAssets.clear();
		EntityAssets.shrink_to_fit();

		std::int_fast32_t AssetFileIndex{};
		std::int_fast32_t AssetIndex{};

		while (true) //-V776
		{
			bool SkipAssetFlag{};

			if (const std::string EntityDataFile{ "./DATA/Level_" + std::to_string(SelectedLevel) + "/EntityData/" + std::to_string(AssetFileIndex) + ".ini" }; Tools_ErrorHandling::CheckFileExistence(EntityDataFile, ContinueOnError))
			{
				const std::string AssetTypeName{ lwmf::ReadINIValue<std::string>(EntityDataFile, "ENTITY", "EntityTypeName") };

				// If asset type was already loaded, skip this...
				for (const auto& Asset : EntityAssets)
				{
					if (Asset.Name == AssetTypeName)
					{
						SkipAssetFlag = true;
						break;
					}
				}

				if (const std::string INIFile{ "./DATA/Assets_Entities/" + AssetTypeName + "/AssetData.ini" }; !SkipAssetFlag && Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
				{
					EntityAssets.emplace_back();
					EntityAssets[AssetIndex].Number = AssetIndex;
					EntityAssets[AssetIndex].Name = AssetTypeName;

					//
					// Get GFX
					//

					LoadWalkAnimTextures(AssetIndex, AssetTypeName);
					LoadAdditionalAnimTextures("Attack", AssetTypeName, EntityAssets[AssetIndex].AttackTextures);
					LoadAdditionalAnimTextures("Kill", AssetTypeName, EntityAssets[AssetIndex].KillTextures);

					//
					// Get SFX
					//

					if (const std::string AssetType{ lwmf::ReadINIValue<std::string>(INIFile, "GENERAL", "AssetType") }; AssetType == "AmmoBox")
					{
						// Get Pickup audio
						EntityAssets[AssetIndex].Sounds.emplace_back();
						EntityAssets[AssetIndex].Sounds[0].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "AmmoPickup"));
					}
					else if (AssetType == "Enemy" || AssetType == "Turret")
					{
						// Get KillSound audio
						EntityAssets[AssetIndex].Sounds.emplace_back();
						EntityAssets[AssetIndex].Sounds[0].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "KillSound"));

						// Get AttackSound audio
						EntityAssets[AssetIndex].Sounds.emplace_back();
						EntityAssets[AssetIndex].Sounds[1].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "AttackSound"));
					}

					++AssetIndex;
				}

				++AssetFileIndex;
			}
			else
			{
				break;
			}
		}
	}

	inline void LoadWalkAnimTextures(const std::int_fast32_t AssetIndex, const std::string& AssetTypeName)
	{
		std::int_fast32_t DirectionIndex{};

		while (true)
		{
			if (const std::string Path{ "./GFX/Entities/" + std::to_string(EntitySize) + "/" + AssetTypeName + "/" + std::to_string(DirectionIndex) }; Tools_ErrorHandling::CheckFolderExistence(Path, ContinueOnError))
			{
				EntityAssets[AssetIndex].WalkingTextures.emplace_back();

				std::int_fast32_t TextureIndex{};

				while (true)
				{
					if (const std::string Texture{ Path + "/" + std::to_string(TextureIndex) + ".png" }; Tools_ErrorHandling::CheckFileExistence(Texture, ContinueOnError))
					{
						EntityAssets[AssetIndex].WalkingTextures[DirectionIndex].emplace_back(GFX_ImageHandling::ImportTexture(Texture, EntitySize));
						++TextureIndex;
					}
					else
					{
						break;
					}
				}

				++DirectionIndex;
			}
			else
			{
				break;
			}
		}
	}

	inline void LoadAdditionalAnimTextures(const std::string& AnimType, const std::string& AssetTypeName, std::vector<lwmf::TextureStruct>& AnimVector)
	{
		std::int_fast32_t TextureIndex{};

		while (true)
		{
			if (const std::string Texture{ "./GFX/Entities/" + std::to_string(EntitySize) + "/" + AssetTypeName + "/" + AnimType + "/" + std::to_string(TextureIndex) + ".png" }; Tools_ErrorHandling::CheckFileExistence(Texture, ContinueOnError))
			{
				AnimVector.emplace_back(GFX_ImageHandling::ImportTexture(Texture, EntitySize));
				++TextureIndex;
			}
			else
			{
				break;
			}
		}
	}

	inline void InitEntities()
	{
		Entities.clear();
		Entities.shrink_to_fit();
		EntityMap.clear();
		EntityMap.shrink_to_fit();
		EntityOrder.clear();
		EntityOrder.shrink_to_fit();
		EntityDistance.clear();
		EntityDistance.shrink_to_fit();
		ZBuffer.clear();
		ZBuffer.shrink_to_fit();
		ZBuffer.resize(static_cast<size_t>(ScreenTexture.Width));

		EntityMap = std::vector<std::vector<EntityTypes>>(static_cast<size_t>(Game_LevelHandling::LevelMapWidth), std::vector<EntityTypes>(static_cast<size_t>(Game_LevelHandling::LevelMapHeight), EntityTypes::Clear));

		std::int_fast32_t Index{};

		while (true)
		{
			if (const std::string INIFile{ "./DATA/Level_" + std::to_string(SelectedLevel) + "/EntityData/" + std::to_string(Index) + ".ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, ContinueOnError))
			{
				EntityOrder.emplace_back();
				EntityDistance.emplace_back();
				Entities.emplace_back();
				Entities[Index].Number = Index;
				Entities[Index].TypeName = lwmf::ReadINIValue<std::string>(INIFile, "ENTITY", "EntityTypeName");

				if (const std::string EntityTypeString{ lwmf::ReadINIValue<std::string>(INIFile, "ENTITY", "EntityType") }; EntityTypeString == "Neutral")
				{
					Entities[Index].Type = static_cast<std::int_fast32_t>(EntityTypes::Neutral);
				}
				else if (EntityTypeString == "Enemy")
				{
					Entities[Index].Type = static_cast<std::int_fast32_t>(EntityTypes::Enemy);
				}
				else if (EntityTypeString == "AmmoBox")
				{
					Entities[Index].Type = static_cast<std::int_fast32_t>(EntityTypes::AmmoBox);
				}
				else if (EntityTypeString == "Turret")
				{
					Entities[Index].Type = static_cast<std::int_fast32_t>(EntityTypes::Turret);
				}

				Entities[Index].WalkAnimStepWidth = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "ENTITY", "WalkAnimStepWidth");
				Entities[Index].AttackAnimStepWidth = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "ENTITY", "AttackAnimStepWidth");
				Entities[Index].MoveV = lwmf::ReadINIValue<float>(INIFile, "ENTITY", "EntityMoveV");
				Entities[Index].MoveSpeed = lwmf::ReadINIValue<float>(INIFile, "MOVEMENT", "MoveSpeed");
				Entities[Index].MovementBehaviour = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "MOVEMENT", "MovementBehaviour");
				Entities[Index].AttackMode = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "MOVEMENT", "AttackMode");
				Entities[Index].Pos = { lwmf::ReadINIValue<float>(INIFile, "POSITION", "StartPosX"), lwmf::ReadINIValue<float>(INIFile, "POSITION", "StartPosY") };
				Entities[Index].Dir = { lwmf::ReadINIValue<float>(INIFile, "DIRECTION", "DirX"), lwmf::ReadINIValue<float>(INIFile, "DIRECTION", "DirY") };
				Entities[Index].Direction = lwmf::ReadINIValue<char>(INIFile, "DIRECTION", "Direction");
				Entities[Index].RotationFactor = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "DIRECTION", "RotationFactor");
				Entities[Index].Hitpoints = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "STATUS", "Hitpoints");
				Entities[Index].HitAnimDuration = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "STATUS", "HitAnimDuration");
				Entities[Index].DamagePoints = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "DAMAGE", "DamagePoints");
				Entities[Index].DamageHitrate = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "DAMAGE", "DamageHitrate");
				Entities[Index].ContainedItem[lwmf::ReadINIValue<std::string>(INIFile, "CONTAINS", "ContainedItem")] = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "CONTAINS", "ContainedItemValue");

				// Assign proper asset data (= texture set) to entity
				for (const auto& Asset : EntityAssets)
				{
					if (Entities[Index].TypeName == Asset.Name)
					{
						Entities[Index].TypeNumber = Asset.Number;
						break;
					}
				}

				MarkEntityPositionOnMap(Entities[Index]);
				++Index;
			}
			else
			{
				break;
			}
		}
	}

	inline void RenderEntities()
	{
		const float InverseMatrix{ 1.0F / (Plane.X * Player.Dir.Y - Player.Dir.X * Plane.Y) };
		const std::int_fast32_t VerticalLookTemp{ ScreenTexture.Height + VerticalLook };
		const std::int_fast32_t NumberOfEntities{ static_cast<std::int_fast32_t>(Entities.size()) };

		for (std::int_fast32_t Index{}; Index < NumberOfEntities; ++Index)
		{
			if (!Entities[EntityOrder[Index]].IsDead)
			{
				const lwmf::FloatPointStruct EntityPos{ Entities[EntityOrder[Index]].Pos.X - Player.Pos.X, Entities[EntityOrder[Index]].Pos.Y - Player.Pos.Y };
				const float TransY{ InverseMatrix * (-Plane.Y * EntityPos.X + Plane.X * EntityPos.Y) };
				const std::int_fast32_t vScreen{ static_cast<std::int_fast32_t>(Entities[EntityOrder[Index]].MoveV / TransY) };
				const std::int_fast32_t EntitySizeTemp{ static_cast<std::int_fast32_t>(ScreenTexture.Height / TransY) };
				const std::int_fast32_t Temp{ (VerticalLookTemp >> 1) + vScreen };
				const std::int_fast32_t LineStartY{ (std::max)(-(EntitySizeTemp >> 1) + Temp, 0) };
				const std::int_fast32_t LineEndY{ (std::min)((EntitySizeTemp >> 1) + Temp, ScreenTexture.Height) };
				const std::int_fast32_t EntitySX{ static_cast<std::int_fast32_t>(ScreenTexture.WidthMid * (1.0F + InverseMatrix * (Player.Dir.Y * EntityPos.X - Player.Dir.X * EntityPos.Y) / TransY)) };
				const std::int_fast32_t LineEndX{ (std::min)((EntitySizeTemp >> 1) + EntitySX, ScreenTexture.Width) };
				const std::int_fast32_t Temp1{ (-EntitySizeTemp >> 1) + EntitySX };
				const std::int_fast32_t Temp2{ VerticalLookTemp << 7 };
				const std::int_fast32_t Temp3{ EntitySizeTemp << 7 };
				const std::int_fast32_t TextureIndex{ GetEntityTextureIndex(Index) };

				for (std::int_fast32_t x{ (-EntitySizeTemp >> 1) + EntitySX }; x < LineEndX; ++x)
				{
					if (TransY > 0.0F && x > 0 && x < ScreenTexture.Width && TransY < ZBuffer[x])
					{
						const std::int_fast32_t TextureX{ (x - Temp1) * EntitySize / EntitySizeTemp };

						for (std::int_fast32_t y{ LineStartY }; y < LineEndY; ++y)
						{
							const std::int_fast32_t Color{ Entities[Entities[EntityOrder[Index]].Number].AttackAnimEnabled ?
								EntityAssets[Entities[Entities[EntityOrder[Index]].Number].TypeNumber].AttackTextures[Entities[EntityOrder[Index]].AttackAnimStep].Pixels[((((((y - vScreen) << 8) - Temp2 + Temp3) * EntitySize) / EntitySizeTemp) >> 8) * EntitySize + TextureX] :
								EntityAssets[Entities[Entities[EntityOrder[Index]].Number].TypeNumber].WalkingTextures[TextureIndex][Entities[EntityOrder[Index]].WalkAnimStep].Pixels[((((((y - vScreen) << 8) - Temp2 + Temp3) * EntitySize) / EntitySizeTemp) >> 8) * EntitySize + TextureX] };

							// Check if alphachannel of pixel ist not transparent and draw pixel
							if ((Color & lwmf::AMask) != 0)
							{
								Entities[EntityOrder[Index]].IsHit ? lwmf::SetPixel(ScreenTexture, x, y, Color | 0xFFFFFF00) :
									(Game_LevelHandling::LightingFlag ? lwmf::SetPixel(ScreenTexture, x, y, lwmf::ShadeColor(Color, TransY, FogOfWarDistance)) : lwmf::SetPixel(ScreenTexture, x, y, Color));
							}
						}
					}
				}
			}
		}
	}

	inline std::int_fast32_t GetEntityTextureIndex(const std::int_fast32_t EntityNumber)
	{
		// Get angle between player and entity without atan2
		// Returns TextureIndex (0..7) for adressing correct texture

		const lwmf::FloatPointStruct EntityTemp{ Entities[Entities[EntityOrder[EntityNumber]].Number].Pos.X - Player.Pos.X, Entities[Entities[EntityOrder[EntityNumber]].Number].Pos.Y - Player.Pos.Y };
		const float CosTheta1{ (EntityTemp.X + EntityTemp.Y) * lwmf::SQRT1_2 };
		const float CosTheta3{ (EntityTemp.Y - EntityTemp.X) * lwmf::SQRT1_2 };
		float ClosestTheta{ EntityTemp.X };

		std::int_fast32_t TextureIndex{};

		if (ClosestTheta < CosTheta1)
		{
			TextureIndex = 1;
			ClosestTheta = CosTheta1;
		}

		if (ClosestTheta < EntityTemp.Y)
		{
			TextureIndex = 2;
			ClosestTheta = EntityTemp.Y;
		}

		if (ClosestTheta < CosTheta3)
		{
			TextureIndex = 3;
			ClosestTheta = CosTheta3;
		}

		if (ClosestTheta < -EntityTemp.X)
		{
			TextureIndex = 4;
			ClosestTheta = -EntityTemp.X;
		}

		if (ClosestTheta < -CosTheta1)
		{
			TextureIndex = 5;
			ClosestTheta = -CosTheta1;
		}

		if (ClosestTheta < -EntityTemp.Y)
		{
			TextureIndex = 6;
			ClosestTheta = -EntityTemp.Y;
		}

		if (ClosestTheta < -CosTheta3)
		{
			TextureIndex = 7;
		}

		// Add rotation factor to Textureindex dependent on heading direction of entity
		const std::int_fast32_t TextureIndexTemp{ TextureIndex + Entities[Entities[EntityOrder[EntityNumber]].Number].RotationFactor };
		return TextureIndexTemp < 8 ? TextureIndexTemp : TextureIndexTemp - 8;
	}

	inline void HandleEntityHit(EntityStruct& Entity)
	{
		if (Entity.Type != static_cast<std::int_fast32_t>(EntityTypes::AmmoBox))
		{
			Entity.IsHit = true;
			Entity.AttackMode = 1;
			Entity.Type = static_cast<std::int_fast32_t>(EntityTypes::Enemy);

			// Is entity still alive?
			if (Entity.Hitpoints > 0)
			{
				Entity.Hitpoints -= Weapons[Player.SelectedWeapon].Damage;
				Entity.HitAnimCounter += Entity.HitAnimDuration;
			}

			if (Entity.Hitpoints <= 0)
			{
				PlayAudio(Entity.TypeNumber, EntitySounds::Kill);

				// Entity is dead, but needs to be rendered one last time...
				// Will be set to "IsDead" in MoveEntities()
				Entity.WillBeDead = true;

				// ...and gets cleared from EntityMap
				Entity.MovementBehaviour = 0;
				EntityMap[static_cast<std::int_fast32_t>(Entity.Pos.X)][static_cast<std::int_fast32_t>(Entity.Pos.Y)] = EntityTypes::Clear;
			}
		}
	}

	inline void ChangeEntityDirection(EntityStruct& Entity, const char NewDirection)
	{
		// Possible directions (DirX / DirY)
		//		-1.0 / 0.0		North		RotationFactor 0
		//		1.0 / 0.0		South		RotationFactor 4
		//		0.0 / 1.0		East		RotationFactor 2
		//		0.0 / -1.0		West		RotationFactor 6

		switch (NewDirection)
		{
			case 'l':
			{
				// Turn left (-> West) if moved North previously
				if (Entity.Dir.X == -1.0F) //-V550
				{
					Entity.Dir = { 0.0F, -1.0F };
					Entity.Direction = 'W';
					Entity.RotationFactor = 6;
				}
				// Turn left (-> East) if moved South previously
				else if (Entity.Dir.X == 1.0F) //-V550
				{
					Entity.Dir = { 0.0F, 1.0F };
					Entity.Direction = 'E';
					Entity.RotationFactor = 2;
				}
				// Turn left (-> North) if moved East previously
				else if (Entity.Dir.Y == 1.0F) //-V550
				{
					Entity.Dir = { -1.0F, 0.0F };
					Entity.Direction = 'N';
					Entity.RotationFactor = 0;
				}
				// Turn left (-> South) if moved West previously
				else if (Entity.Dir.Y == -1.0F) //-V550
				{
					Entity.Dir = { 1.0F, 0.0F };
					Entity.Direction = 'S';
					Entity.RotationFactor = 4;
				}
				break;
			}
			case 'r':
			{
				// Turn right (-> East) if moved North previously
				if (Entity.Dir.X == -1.0F) //-V550
				{
					Entity.Dir = { 0.0F, 1.0F };
					Entity.Direction = 'E';
					Entity.RotationFactor = 2;
				}
				// Turn right (-> West) if moved South previously
				else if (Entity.Dir.X == 1.0F) //-V550
				{
					Entity.Dir = { 0.0F, -1.0F };
					Entity.Direction = 'W';
					Entity.RotationFactor = 6;
				}
				// Turn right (-> South) if moved East previously
				else if (Entity.Dir.Y == 1.0F) //-V550
				{
					Entity.Dir = { 1.0F, 0.0F };
					Entity.Direction = 'S';
					Entity.RotationFactor = 4;
				}
				// Turn right (-> North) if moved West previously
				else if (Entity.Dir.Y == -1.0F) //-V550
				{
					Entity.Dir = { -1.0F, 0.0F };
					Entity.Direction = 'N';
					Entity.RotationFactor = 0;
				}
				break;
			}

			default: {}
		}
	}

	inline void TurnEntityBackwards(EntityStruct& Entity)
	{
		switch (Entity.Direction)
		{
			case 'N':
			{
				Entity.Dir = { 1.0F, 0.0F };
				Entity.Direction = 'S';
				Entity.RotationFactor = 4;
				break;
			}
			case 'E':
			{
				Entity.Dir = { 0.0F, -1.0F };
				Entity.Direction = 'W';
				Entity.RotationFactor = 6;
				break;
			}
			case 'S':
			{
				Entity.Dir = { -1.0F, 0.0F };
				Entity.Direction = 'N';
				Entity.RotationFactor = 0;
				break;
			}
			case 'W':
			{
				Entity.Dir = { 0.0F, 1.0F };
				Entity.Direction = 'E';
				Entity.RotationFactor = 2;
				break;
			}

			default:{}
		}
	}

	inline void CalculateEntityPath(EntityStruct& Entity)
	{
		if (Entity.Type == static_cast<std::int_fast32_t>(EntityTypes::Enemy) || Entity.Type == static_cast<std::int_fast32_t>(EntityTypes::Neutral))
		{
			Entity.PathFindingWayPoints.clear();

			Entity.PathFindingStart = Game_PathFinding::SetPathFindingPoint(static_cast<std::int_fast32_t>(Entity.Pos.X), static_cast<std::int_fast32_t>(Entity.Pos.Y), Game_LevelHandling::LevelMapWidth);
			Entity.PathFindingTarget = Game_PathFinding::SetPathFindingPoint(static_cast<std::int_fast32_t>(Player.Pos.X), static_cast<std::int_fast32_t>(Player.Pos.Y), Game_LevelHandling::LevelMapWidth); //-V778

			Entity.ValidPathFound = Game_PathFinding::CalculatePath(Game_PathFinding::FlattenedMap, Game_LevelHandling::LevelMapWidth, Game_LevelHandling::LevelMapHeight, Entity.PathFindingStart, Entity.PathFindingTarget, false, Entity.PathFindingWayPoints);
		}
	}

	inline void MoveEntities()
	{
		for (auto&& Entity : Entities)
		{
			if (Entity.IsHit && --Entity.HitAnimCounter == 0)
			{
				Entity.IsHit = false;

				// Don´t show entity in next gameloop cycle
				Entity.IsDead = Entity.WillBeDead ? true : false;
			}

			if (!Entity.IsDead)
			{
				// Run A* pathfinding routine...
				CalculateEntityPath(Entity);

				switch (Entity.MovementBehaviour)
				{
					case 0:
					{
						//
						// Stationary
						//

						// Refresh ammo position on map once another entity moved over this position
						if (Entity.Type == static_cast<std::int_fast32_t>(EntityTypes::AmmoBox) && EntityMap[static_cast<std::int_fast32_t>(Entity.Pos.X)][static_cast<std::int_fast32_t>(Entity.Pos.Y)] == EntityTypes::Clear)
						{
							MarkEntityPositionOnMap(Entity);
						}
						break;
					}
					case 2:
					{
						//
						// Free roaming mode
						//

						const float EntityCollisionDetectionFactor{ Entity.MoveSpeed + EntityCollisionDetectionWallDist };

						EntityMap[static_cast<std::int_fast32_t>(Entity.Pos.X)][static_cast<std::int_fast32_t>(Entity.Pos.Y)] = EntityTypes::Clear;

						// Wait by chance
						// Check if a chance hit occured and if no current timer is running
						if (Entity.WaitTimer == 0 && Distribution666(RNG) == 99 && Entity.AttackMode == 0)
						{
							Entity.WaitTimer = static_cast<std::int_fast32_t>(Distribution200(RNG));
						}

						// Switch textures for walking animations
						if (Entity.WaitTimer > 0)
						{
							--Entity.WaitTimer;
							Entity.WalkAnimStep = 0;
						}
						else
						{
							if (EntityAssets[Entity.TypeNumber].WalkingTextures[0].size() == 1)
							{
								// Not animated
								Entity.WalkAnimStep = 0;
							}
							else
							{
								// animated
								if (++Entity.WalkAnimCounter > Entity.WalkAnimStepWidth)
								{
									Entity.WalkAnimStep < static_cast<std::int_fast32_t>(EntityAssets[Entity.TypeNumber].WalkingTextures[0].size()) - 1 ? ++Entity.WalkAnimStep : Entity.WalkAnimStep = 0;
									Entity.WalkAnimCounter = 0;
								}
							}

							// Move forward
							Entity.Pos.X += Entity.Dir.X * Entity.MoveSpeed;
							Entity.Pos.Y += Entity.Dir.Y * Entity.MoveSpeed;
						}

						// Switch textures for attack animations
						if (Entity.AttackAnimEnabled && ++Entity.AttackAnimCounter > Entity.AttackAnimStepWidth)
						{
							if (Entity.AttackAnimStep < static_cast<std::int_fast32_t>(EntityAssets[Entity.TypeNumber].AttackTextures.size()) - 1)
							{
								++Entity.AttackAnimStep;
							}
							else
							{
								Entity.AttackAnimStep = 0;
								Entity.AttackAnimEnabled = false;
								Entity.AttackFinished = true;
							}

							Entity.AttackAnimCounter = 0;
						}

						const std::int_fast32_t EntityPosXTemp{ static_cast<std::int_fast32_t>(Entity.Pos.X + Entity.Dir.X * EntityCollisionDetectionFactor) };
						const std::int_fast32_t EntityPosYTemp{ static_cast<std::int_fast32_t>(Entity.Pos.Y + Entity.Dir.Y * EntityCollisionDetectionFactor) };

						if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][EntityPosXTemp][EntityPosYTemp] != 0)
						{
							Entity.Pos.X -= Entity.Dir.X * Entity.MoveSpeed;
							Entity.Pos.Y -= Entity.Dir.Y * Entity.MoveSpeed;

							// Random choice of new direction (left or right)
							ChangeEntityDirection(Entity, "lr"[rand() % 2]);
						}
						// Turn backwards if stepping on another enemy or neutral entity
						else if (EntityMap[EntityPosXTemp][EntityPosYTemp] == EntityTypes::Enemy
							|| EntityMap[EntityPosXTemp][EntityPosYTemp] == EntityTypes::Neutral
							|| EntityMap[EntityPosXTemp][EntityPosYTemp] == EntityTypes::Turret)
						{
							TurnEntityBackwards(Entity);
						}
						// What happens if entity meets player?
						else if (EntityMap[EntityPosXTemp][EntityPosYTemp] == EntityTypes::Player)
						{
							// Deal damage to player
							if (Entity.Type == static_cast<std::int_fast32_t>(EntityTypes::Enemy))
							{
								Entity.Pos.X -= Entity.Dir.X * Entity.MoveSpeed;
								Entity.Pos.Y -= Entity.Dir.Y * Entity.MoveSpeed;

								if (--Entity.DamageHitrateCounter <= 0)
								{
									PlayAudio(Entity.TypeNumber, EntitySounds::Attack);
									Entity.DamageHitrateCounter = Entity.DamageHitrate * static_cast<std::int_fast32_t>(FrameLock);
									Entity.AttackAnimEnabled = true;

									// Once it attacked, entity is in "rage" mode, so it will attack without a pause...
									Entity.AttackMode = 1;
								}
							}
							else if (Entity.Type == static_cast<std::int_fast32_t>(EntityTypes::Neutral))
							{
								TurnEntityBackwards(Entity);
							}
						}

						if (Entity.AttackFinished)
						{
							Player.HurtPlayer(Entity.DamagePoints);
							Entity.AttackFinished = false;
						}

						MarkEntityPositionOnMap(Entity);
						break;
					}

					default:{}
				}
			}
		}
	}

	inline void GetEntityDistance()
	{
		const std::int_fast32_t NumberOfEntities{ static_cast<std::int_fast32_t>(Entities.size()) };

		for (std::int_fast32_t Index{}; Index < NumberOfEntities; ++Index)
		{
			EntityOrder[Index] = Index;
			EntityDistance[Index] = lwmf::CalcEuclidianDistance<float>(Player.Pos.X, Entities[Index].Pos.X, Player.Pos.Y, Entities[Index].Pos.Y);
		}
	}

	inline void SortEntities(const SortOrder SortOrder)
	{
		const std::int_fast32_t NumberOfEntities{ static_cast<std::int_fast32_t>(Entities.size()) };

		for (std::int_fast32_t Index{}; Index < NumberOfEntities - 1; ++Index)
		{
			std::int_fast32_t MinIndex{ Index };

			for (std::int_fast32_t j{ Index + 1 }; j < NumberOfEntities; ++j)
			{
				switch (SortOrder)
				{
					case SortOrder::FrontToBack:
					{
						if (EntityDistance[j] < EntityDistance[MinIndex])
						{
							MinIndex = j;
						}
						break;
					}
					case SortOrder::BackToFront:
					{
						if (EntityDistance[j] > EntityDistance[MinIndex])
						{
							MinIndex = j;
						}
						break;
					}

					default:{}
				}
			}

			if (MinIndex != Index)
			{
				std::swap(EntityDistance[Index], EntityDistance[MinIndex]);
				std::swap(EntityOrder[Index], EntityOrder[MinIndex]);
			}
		}
	}

	inline void MarkEntityPositionOnMap(const EntityStruct& Entity)
	{
		const lwmf::IntPointStruct EntityPos{ static_cast<std::int_fast32_t>(Entity.Pos.X), static_cast<std::int_fast32_t>(Entity.Pos.Y) };

		switch (Entity.Type)
		{
			case static_cast<std::int_fast32_t>(EntityTypes::Neutral) :
			{
				EntityMap[EntityPos.X][EntityPos.Y] = EntityTypes::Neutral;
				break;
			}
			case static_cast<std::int_fast32_t>(EntityTypes::Enemy):
			{
				EntityMap[EntityPos.X][EntityPos.Y] = EntityTypes::Enemy;
				break;
			}
			case static_cast<std::int_fast32_t>(EntityTypes::AmmoBox):
			{
				EntityMap[EntityPos.X][EntityPos.Y] = EntityTypes::AmmoBox;
				break;
			}
			case static_cast<std::int_fast32_t>(EntityTypes::Turret):
			{
				EntityMap[EntityPos.X][EntityPos.Y] = EntityTypes::Turret;
				break;
			}

			default: {}
		}
	}

	inline void PlayAudio(const std::int_fast32_t TypeNumber, const EntitySounds EntitySound)
	{
		EntityAssets[TypeNumber].Sounds[static_cast<std::int_fast32_t>(EntitySound)].Play(lwmf::MP3::PlayModes::FROMSTART);
	}

	inline void CloseAudio()
	{
		for (auto&& Asset : EntityAssets)
		{
			for (auto&& Sound : Asset.Sounds)
			{
				Sound.Close();
			}
		}
	}


} // namespace Game_EntityHandling