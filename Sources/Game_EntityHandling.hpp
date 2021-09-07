/*
******************************************
*                                        *
* Game_EntityHandling.hpp                *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <map>
#include <utility>
#include <tuple>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Game_DataStructures.hpp"
#include "GFX_ImageHandling.hpp"
#include "Game_LevelHandling.hpp"
#include "Game_PathFinding.hpp"

namespace Game_EntityHandling
{


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
	void SwitchDirection(EntityStruct& Entity, char Direction);
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

	using DirectionTuple = std::tuple<float, float, char, std::int_fast32_t>;

	inline const std::vector<DirectionTuple> Directions
	{
		// Possible directions (DirX / DirY)
		//		-1.0 / 0.0		North		RotationFactor 0
		//		1.0 / 0.0		South		RotationFactor 4
		//		0.0 / 1.0		East		RotationFactor 2
		//		0.0 / -1.0		West		RotationFactor 6

		DirectionTuple(-1.0F, 0.0F, 'N', 0),
		DirectionTuple(1.0F, 0.0F, 'S', 4),
		DirectionTuple(0.0F, 1.0F, 'E', 2),
		DirectionTuple(0.0F, -1.0F, 'W', 6)
	};

	// Populate the Mersenne-Twister-Random Engine with proper distributions
	inline const std::uniform_int_distribution<std::int_fast32_t> Distribution200(1, 200);
	inline const std::uniform_int_distribution<std::int_fast32_t> Distribution666(1, 666);

	constexpr float EntityCollisionDetectionWallDist{ 0.5F };

	inline std::vector<std::vector<EntityTypes>> EntityMap{};

	// Vector used to sort the entities
	inline std::vector<std::pair<std::int_fast32_t, float>> EntityOrder{};

	// 1D Zbuffer
	inline std::vector<float> ZBuffer{};

	//
	// Functions
	//

	inline void InitEntityAssets()
	{
		CloseAudio();

		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init entity assets...");

		EntityAssets.clear();
		EntityAssets.shrink_to_fit();

		std::int_fast32_t AssetFileIndex{};
		std::int_fast32_t AssetIndex{};

		while (true) //-V776
		{
			bool SkipAssetFlag{};

			std::string EntityDataFile{ LevelFolder };
			EntityDataFile += std::to_string(SelectedLevel);
			EntityDataFile += "/EntityData/";
			EntityDataFile += std::to_string(AssetFileIndex);
			EntityDataFile += ".ini";

			if (Tools_ErrorHandling::CheckFileExistence(EntityDataFile, ContinueOnError))
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

				std::string INIFile{ AssetsEntitiesFolder };
				INIFile += AssetTypeName;
				INIFile += "/AssetData.ini";

				if (!SkipAssetFlag && Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
				{
					EntityAssets.emplace_back();
					EntityAssets[AssetIndex].Number = AssetIndex;
					EntityAssets[AssetIndex].Name = AssetTypeName;

					//
					// Get GFX
					//

					NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Load entity textures...");

					LoadWalkAnimTextures(AssetIndex, AssetTypeName);
					LoadAdditionalAnimTextures("Attack", AssetTypeName, EntityAssets[AssetIndex].AttackTextures);
					LoadAdditionalAnimTextures("Kill", AssetTypeName, EntityAssets[AssetIndex].KillTextures);

					//
					// Get SFX
					//

					NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Load entity audio...");

					EntityAssets[AssetIndex].Sounds.clear();
					EntityAssets[AssetIndex].Sounds.shrink_to_fit();

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
		EntityAssets[AssetIndex].WalkingTextures.clear(); //-V807
		EntityAssets[AssetIndex].WalkingTextures.shrink_to_fit();

		std::int_fast32_t DirectionIndex{};

		while (true)
		{
			std::string Path{ GFXEntitiesFolder };
			Path += std::to_string(EntitySize);
			Path += "/";
			Path += AssetTypeName;
			Path += "/";
			Path += std::to_string(DirectionIndex);

			if (Tools_ErrorHandling::CheckFolderExistence(Path, ContinueOnError))
			{
				EntityAssets[AssetIndex].WalkingTextures.emplace_back();

				std::int_fast32_t TextureIndex{};

				while (true)
				{
					std::string Texture{ Path };
					Texture += "/";
					Texture += std::to_string(TextureIndex);
					Texture += ".png";

					if (Tools_ErrorHandling::CheckFileExistence(Texture, ContinueOnError))
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
		AnimVector.clear();
		AnimVector.shrink_to_fit();

		std::int_fast32_t TextureIndex{};

		while (true)
		{
			std::string Texture{ GFXEntitiesFolder };
			Texture += std::to_string(EntitySize);
			Texture += "/";
			Texture += AssetTypeName;
			Texture += "/";
			Texture += AnimType;
			Texture += "/";
			Texture += std::to_string(TextureIndex);
			Texture += ".png";

			if (Tools_ErrorHandling::CheckFileExistence(Texture, ContinueOnError))
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
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init entities...");

		Entities.clear();
		Entities.shrink_to_fit();
		EntityMap.clear();
		EntityMap.shrink_to_fit();
		EntityOrder.clear();
		EntityOrder.shrink_to_fit();
		ZBuffer.clear();
		ZBuffer.shrink_to_fit();
		ZBuffer.resize(static_cast<size_t>(ScreenTexture.Width));

		EntityMap = std::vector<std::vector<EntityTypes>>(static_cast<size_t>(Game_LevelHandling::LevelMapWidth), std::vector<EntityTypes>(static_cast<size_t>(Game_LevelHandling::LevelMapHeight), EntityTypes::Clear));

		std::int_fast32_t Index{};

		while (true)
		{
			std::string INIFile{ LevelFolder };
			INIFile += std::to_string(SelectedLevel);
			INIFile += "/EntityData/";
			INIFile += std::to_string(Index);
			INIFile += ".ini";

			if (Tools_ErrorHandling::CheckFileExistence(INIFile, ContinueOnError))
			{
				EntityOrder.emplace_back();
				Entities.emplace_back();
				Entities[Index].Number = Index;
				Entities[Index].TypeName = lwmf::ReadINIValue<std::string>(INIFile, "ENTITY", "EntityTypeName");

				const std::string EntityTypeString{ lwmf::ReadINIValue<std::string>(INIFile, "ENTITY", "EntityType") }; //-V808

				const std::map<std::string, EntityTypes> EntityTypeCompare //-V808
				{
					{ "Clear", EntityTypes::Clear },
					{ "Neutral", EntityTypes::Neutral },
					{ "Enemy", EntityTypes::Enemy },
					{ "Player", EntityTypes::Player },
					{ "AmmoBox", EntityTypes::AmmoBox },
					{ "Turret", EntityTypes::Turret }
				};

				if (const auto Type{ EntityTypeCompare.find(EntityTypeString) }; Type != EntityTypeCompare.end())
				{
					Entities[Index].Type = Type->second;
				}
				else
				{
					NARCLog.AddEntry(lwmf::LogLevel::Critical, __FILENAME__, __LINE__, "Entity type wrong or not found!");
				}

				Entities[Index].WalkAnimStepWidth = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "ENTITY", "WalkAnimStepWidth");
				Entities[Index].AttackAnimStepWidth = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "ENTITY", "AttackAnimStepWidth");
				Entities[Index].KillAnimStepWidth = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "ENTITY", "KillAnimStepWidth");
				Entities[Index].MoveV = lwmf::ReadINIValue<float>(INIFile, "ENTITY", "EntityMoveV");
				Entities[Index].MoveSpeed = lwmf::ReadINIValue<float>(INIFile, "MOVEMENT", "MoveSpeed");
				Entities[Index].MovementBehaviour = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "MOVEMENT", "MovementBehaviour");
				Entities[Index].AttackMode = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "MOVEMENT", "AttackMode");
				Entities[Index].Pos = { lwmf::ReadINIValue<float>(INIFile, "POSITION", "StartPosX"), lwmf::ReadINIValue<float>(INIFile, "POSITION", "StartPosY") };

				// Load/set direction data: Dir.X, Dir.Y, Direction, Rotationfactor
				SwitchDirection(Entities[Index], lwmf::ReadINIValue<char>(INIFile, "DIRECTION", "Direction"));

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
			// Additional check if Loot is not picked up...
			if (!Entities[EntityOrder[Index].first].IsPickedUp)
			{
				const lwmf::FloatPointStruct EntityPos{ Entities[EntityOrder[Index].first].Pos.X - Player.Pos.X, Entities[EntityOrder[Index].first].Pos.Y - Player.Pos.Y };
				const float TransY{ InverseMatrix * (-Plane.Y * EntityPos.X + Plane.X * EntityPos.Y) };
				const std::int_fast32_t vScreen{ static_cast<std::int_fast32_t>(Entities[EntityOrder[Index].first].MoveV / TransY) };
				const std::int_fast32_t EntitySizeTemp{ static_cast<std::int_fast32_t>(ScreenTexture.Height / TransY) };
				const std::int_fast32_t Temp{ (VerticalLookTemp >> 1) + vScreen };
				const std::int_fast32_t LineStartY{ std::max(-(EntitySizeTemp >> 1) + Temp, 0) };
				const std::int_fast32_t LineEndY{ std::min((EntitySizeTemp >> 1) + Temp, ScreenTexture.Height) };
				const std::int_fast32_t EntitySX{ static_cast<std::int_fast32_t>(ScreenTexture.WidthMid * (1.0F + InverseMatrix * (Player.Dir.Y * EntityPos.X - Player.Dir.X * EntityPos.Y) / TransY)) };
				const std::int_fast32_t LineEndX{ std::min((EntitySizeTemp >> 1) + EntitySX, ScreenTexture.Width) };
				const std::int_fast32_t Temp1{ (-EntitySizeTemp >> 1) + EntitySX };
				const std::int_fast32_t Temp2{ VerticalLookTemp << 7 };
				const std::int_fast32_t Temp3{ EntitySizeTemp << 7 };
				const std::int_fast32_t TextureIndex{ GetEntityTextureIndex(Index) };

				for (std::int_fast32_t x{ (-EntitySizeTemp >> 1) + EntitySX }; x < LineEndX; ++x)
				{
					if (TransY > 0.0F && (static_cast<std::uint_fast32_t>(x) < static_cast<std::uint_fast32_t>(ScreenTexture.Width)) && TransY < ZBuffer[x])
					{
						const std::int_fast32_t TextureX{ (x - Temp1) * EntitySize / EntitySizeTemp };

						for (std::int_fast32_t y{ LineStartY }; y < LineEndY; ++y)
						{
							std::int_fast32_t Color{};
							const std::int_fast32_t PixelOffset{ ((((((y - vScreen) << 8) - Temp2 + Temp3) * EntitySize) / EntitySizeTemp) >> 8) * EntitySize + TextureX };

							if (Entities[Entities[EntityOrder[Index].first].Number].AttackAnimEnabled)
							{
								Color = EntityAssets[Entities[Entities[EntityOrder[Index].first].Number].TypeNumber].AttackTextures[Entities[EntityOrder[Index].first].AttackAnimStep].Pixels[PixelOffset];
							}
							else if (Entities[Entities[EntityOrder[Index].first].Number].KillAnimEnabled)
							{
								Color = EntityAssets[Entities[Entities[EntityOrder[Index].first].Number].TypeNumber].KillTextures[Entities[EntityOrder[Index].first].KillAnimStep].Pixels[PixelOffset];
							}
							else
							{
								Color = EntityAssets[Entities[Entities[EntityOrder[Index].first].Number].TypeNumber].WalkingTextures[TextureIndex][Entities[EntityOrder[Index].first].WalkAnimStep].Pixels[PixelOffset];
							}

							// Check if alphachannel of pixel ist not transparent and draw pixel
							if ((Color & lwmf::AMask) != 0)
							{
								if (Entities[EntityOrder[Index].first].IsHit && !Entities[EntityOrder[Index].first].KillAnimEnabled)
								{
									lwmf::SetPixel(ScreenTexture, x, y, Color | 0xFFFFFF00);
								}
								else
								{
									Game_LevelHandling::LightingFlag ? (lwmf::SetPixel(ScreenTexture, x, y, lwmf::ShadeColor(Color, TransY, FogOfWarDistance))) : lwmf::SetPixel(ScreenTexture, x, y, Color);
								}
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

		const lwmf::FloatPointStruct EntityTemp{ Entities[Entities[EntityOrder[EntityNumber].first].Number].Pos.X - Player.Pos.X, Entities[Entities[EntityOrder[EntityNumber].first].Number].Pos.Y - Player.Pos.Y };
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
		const std::int_fast32_t TextureIndexTemp{ TextureIndex + Entities[Entities[EntityOrder[EntityNumber].first].Number].RotationFactor };
		return TextureIndexTemp < 8 ? TextureIndexTemp : TextureIndexTemp - 8;
	}

	inline void HandleEntityHit(EntityStruct& Entity)
	{
		if (Entity.Type != EntityTypes::AmmoBox)
		{
			Entity.IsHit = true;
			Entity.AttackMode = 1;
			Entity.Type = EntityTypes::Enemy;

			// Is entity still alive?
			if (Entity.Hitpoints > 0)
			{
				Entity.Hitpoints -= Weapons[Player.SelectedWeapon].Damage;
				Entity.HitAnimCounter += Entity.HitAnimDuration;
			}

			if (Entity.Hitpoints <= 0 && !Entity.KillAnimEnabled)
			{
				PlayAudio(Entity.TypeNumber, EntitySounds::Kill);

				// Entity is killed, now the death animation needs to be rendered...
				// Will be set to "IsDead" in MoveEntities()
				Entity.KillAnimEnabled = true;
				Entity.AttackAnimEnabled = false;
				Entity.AttackFinished = true;
			}
		}
	}

	inline void SwitchDirection(EntityStruct& Entity, const char Direction)
	{
		const auto it{ std::find_if(Directions.begin(), Directions.end(), [&](const auto e) {return std::get<2>(e) == Direction; }) };

		if (it != Directions.end())
		{
			Entity.Dir = { std::get<0>(*it), std::get<1>(*it) };
			Entity.Direction = std::get<2>(*it);
			Entity.RotationFactor = std::get<3>(*it);
		}
		else
		{
			// This check is only needed when the entities are initially loaded...
			NARCLog.AddEntry(lwmf::LogLevel::Error, __FILENAME__, __LINE__, "Entity direction error! Direction must be 'N', 'S', 'E' or 'W'!");
		}
	}

	inline void ChangeEntityDirection(EntityStruct& Entity, const char NewDirection)
	{
		switch (NewDirection)
		{
			case 'l':
			{
				switch (Entity.Direction)
				{
					case 'N':
					{
						// Turn left (-> West) if moved North previously
						SwitchDirection(Entity, 'W');
						break;
					}
					case 'S':
					{
						// Turn left (-> East) if moved South previously
						SwitchDirection(Entity, 'E');
						break;
					}
					case 'E':
					{
						// Turn left (-> North) if moved East previously
						SwitchDirection(Entity, 'N');
						break;
					}
					case 'W':
					{
						// Turn left (-> South) if moved West previously
						SwitchDirection(Entity, 'S');
						break;
					}
					default: {}
				}
				break;
			}
			case 'r':
			{
				switch (Entity.Direction)
				{
					case 'N':
					{
						// Turn right (-> East) if moved North previously
						SwitchDirection(Entity, 'E');
						break;
					}
					case 'S':
					{
						// Turn right (-> West) if moved South previously
						SwitchDirection(Entity, 'W');
						break;
					}
					case 'E':
					{
						// Turn right (-> South) if moved East previously
						SwitchDirection(Entity, 'S');
						break;
					}
					case 'W':
					{
						// Turn right (-> North) if moved West previously
						SwitchDirection(Entity, 'N');
						break;
					}
					default: {}
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
				SwitchDirection(Entity, 'S');
				break;
			}
			case 'E':
			{
				SwitchDirection(Entity, 'W');
				break;
			}
			case 'S':
			{
				SwitchDirection(Entity, 'N');
				break;
			}
			case 'W':
			{
				SwitchDirection(Entity, 'E');
				break;
			}
			default:{}
		}
	}

	inline void CalculateEntityPath(EntityStruct& Entity)
	{
		if (Entity.Type == EntityTypes::Enemy || Entity.Type == EntityTypes::Neutral)
		{
			Entity.PathFindingWayPoints.clear();

			Entity.PathFindingStart = Game_LevelHandling::LevelMapWidth * static_cast<std::int_fast32_t>(Entity.Pos.Y) + static_cast<std::int_fast32_t>(Entity.Pos.X);
			Entity.PathFindingTarget = Game_LevelHandling::LevelMapWidth * static_cast<std::int_fast32_t>(Player.Pos.Y) + static_cast<std::int_fast32_t>(Player.Pos.X); //-V778

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
			}

			if (!Entity.IsDead && Entity.KillAnimEnabled && ++Entity.KillAnimCounter > Entity.KillAnimStepWidth)
			{
				if (Entity.KillAnimStep < static_cast<std::int_fast32_t>(EntityAssets[Entity.TypeNumber].KillTextures.size()) - 1)
				{
					++Entity.KillAnimStep;
				}
				else
				{
					Entity.IsDead = true;
					// ...and gets cleared from EntityMap, but the pile stays...
					Entity.MovementBehaviour = 0;
					EntityMap[static_cast<std::int_fast32_t>(Entity.Pos.X)][static_cast<std::int_fast32_t>(Entity.Pos.Y)] = EntityTypes::Clear;
				}

				Entity.KillAnimCounter = 0;
				break;
			}

			if (!Entity.IsDead && !Entity.KillAnimEnabled)
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
						if (Entity.Type == EntityTypes::AmmoBox && EntityMap[static_cast<std::int_fast32_t>(Entity.Pos.X)][static_cast<std::int_fast32_t>(Entity.Pos.Y)] == EntityTypes::Clear)
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
							if (Entity.Type == EntityTypes::Enemy)
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
							else if (Entity.Type == EntityTypes::Neutral)
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
			EntityOrder[Index] = { Index, lwmf::CalcEuclidianDistance<float>(Player.Pos.X, Entities[Index].Pos.X, Player.Pos.Y, Entities[Index].Pos.Y) };
		}
	}

	inline void SortEntities(const SortOrder SortOrder)
	{
		switch (SortOrder)
		{
			case SortOrder::FrontToBack:
			{
				std::sort(EntityOrder.begin(), EntityOrder.end(), [](auto& Left, auto& Right) {	return Left.second < Right.second; });
				break;
			}
			case SortOrder::BackToFront:
			{
				std::sort(EntityOrder.begin(), EntityOrder.end(), [](auto& Left, auto& Right) {	return Left.second > Right.second; });
				break;
			}
			default: {}
		}
	}

	inline void MarkEntityPositionOnMap(const EntityStruct& Entity)
	{
		EntityMap[static_cast<std::int_fast32_t>(Entity.Pos.X)][static_cast<std::int_fast32_t>(Entity.Pos.Y)] = Entity.Type;
	}

	inline void PlayAudio(const std::int_fast32_t TypeNumber, const EntitySounds EntitySound)
	{
		EntityAssets[TypeNumber].Sounds[static_cast<std::int_fast32_t>(EntitySound)].Play();
	}

	inline void CloseAudio()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Close entity audio...");

		for (auto&& Asset : EntityAssets)
		{
			for (auto&& Sound : Asset.Sounds)
			{
				Sound.Close();
			}
		}
	}


} // namespace Game_EntityHandling