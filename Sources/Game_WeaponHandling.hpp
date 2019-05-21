/*
******************************************
*                                        *
* Game_WeaponHandling.hpp                *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#define FMT_HEADER_ONLY

#include <cstdint>
#include <string>
#include <fstream>
#include <SDL_mixer.h>
#include "fmt/format.h"

#include "Game_GlobalDefinitions.hpp"
#include "Tools_Console.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"
#include "GFX_ImageHandling.hpp"
#include "Game_DataStructures.hpp"
#include "Game_LevelHandling.hpp"
#include "Game_EntityHandling.hpp"

namespace Game_WeaponHandling
{


	enum class WeaponType : std::int_fast32_t
	{
		DirectHit
	};

	enum class WeaponsSounds : std::int_fast32_t
	{
		Shot,
		Dryfire,
		Reload
	};

	enum class WeaponState : std::int_fast32_t
	{
		Ready,
		ChangeUp,
		ChangeDown,
		ReloadInitiated
	};

	enum class FiringState : std::int_fast32_t
	{
		None,
		SingleShot,
		RapidFire
	};

	void InitConfig();
	void InitTextures();
	void InitAudio();
	void CheckForHit();
	void HandleAmmoBoxPickup();
	void InitiateRapidFire();
	void ReleaseRapidFire();
	void InitiateSingleShot();
	void InitiateReload();
	void InitiateWeaponChangeUp();
	void InitiateWeaponChangeDown();
	void FireWeapon();
	void CheckReloadStatus();
	void ChangeWeapon();
	void CountdownMuzzleFlashCounter();
	void CountdownCadenceCounter();
	void DrawWeapon();
	void PlayAudio(std::int_fast32_t SelectedPlayerWeapon, WeaponsSounds WeaponSound);

	//
	// Variables and constants
	//

	inline WeaponState CurrentWeaponState;
	inline FiringState CurrentFiringState;

	inline std::int_fast32_t WeaponHeightFadeInOut{};
	inline std::int_fast32_t WeaponFadeInOutY{};

	inline float WeaponPace{};
	inline bool WeaponPaceFlag{};
	inline bool WeaponMuzzleFlashFlag{};

	//
	// Functions
	//

	inline void InitConfig()
	{
		Weapons.clear();
		Weapons.shrink_to_fit();

		std::int_fast32_t Index{};

		while (true)
		{
			if (const std::string IniFile{ fmt::format("./DATA/Weapons/Weapon_{}_Data.ini", Index) }; Tools_ErrorHandling::CheckFileExistence(IniFile, ShowMessage, ContinueOnError))
			{
				Weapons.emplace_back();

				Weapons[Index].Number = Index;
				Weapons[Index].Name = Tools_INIFile::ReadValue<std::string>(IniFile, "DATA", "Name");
				Weapons[Index].Weight = Tools_INIFile::ReadValue<float>(IniFile, "DATA", "Weight");
				Weapons[Index].Capacity = Tools_INIFile::ReadValue<std::int_fast32_t>(IniFile, "DATA", "Capacity");
				Weapons[Index].PaceFactor = Tools_INIFile::ReadValue<float>(IniFile, "DATA", "PaceFactor");
				Weapons[Index].Damage = Tools_INIFile::ReadValue<std::int_fast32_t>(IniFile, "DATA", "Damage");

				if (const std::string WeaponTypeString{ Tools_INIFile::ReadValue<std::string>(IniFile, "DATA", "WeaponType") }; WeaponTypeString == "DirectHit")
				{
					Weapons[Index].Type = static_cast<std::int_fast32_t>(WeaponType::DirectHit);
				}

				Weapons[Index].CarriedAmmo = Tools_INIFile::ReadValue<std::int_fast32_t>(IniFile, "DATA", "CarriedAmmo");
				Weapons[Index].Cadence = Tools_INIFile::ReadValue<std::int_fast32_t>(IniFile, "DATA", "Cadence");
				Weapons[Index].WeaponRect.x = Tools_INIFile::ReadValue<std::int_fast32_t>(IniFile, "POSITION", "PosX");
				Weapons[Index].WeaponRect.y = Tools_INIFile::ReadValue<std::int_fast32_t>(IniFile, "POSITION", "PosY");
				Weapons[Index].FadeInOutSpeed = Tools_INIFile::ReadValue<std::int_fast32_t>(IniFile, "POSITION", "FadeInOutSpeed");
				Weapons[Index].MuzzleFlashDuration = Tools_INIFile::ReadValue<std::int_fast32_t>(IniFile, "MUZZLEFLASH", "MuzzleFlashDuration");
				Weapons[Index].MuzzleFlashRect.x = Tools_INIFile::ReadValue<std::int_fast32_t>(IniFile, "MUZZLEFLASH", "MuzzleFlashPosX");
				Weapons[Index].MuzzleFlashRect.y = Tools_INIFile::ReadValue<std::int_fast32_t>(IniFile, "MUZZLEFLASH", "MuzzleFlashPosY");

				// pre-load weapon
				Weapons[Index].LoadedRounds = Weapons[Index].Capacity;

				// Initial built of ammo info HUD strings

				Weapons[Index].HUDAmmoInfo = fmt::format("{0}/{0}", Weapons[Index].Capacity);
				Weapons[Index].HUDCarriedAmmoInfo = fmt::format("Carried:{}", Weapons[Index].CarriedAmmo);

				Weapons[Index].WeaponShader.LoadShader("Default");
				Weapons[Index].MuzzleFlashShader.LoadShader("Default");

				++Index;
			}
			else
			{
				Tools_Console::DisplayText(BRIGHT_WHITE, "No more weapon data found.\n");
				break;
			}
		}

		if (Weapons.empty())
		{
			Tools_ErrorHandling::DisplayError("No weapon data found!");
		}
	}

	inline void InitTextures()
	{
		for (auto&& Weapon : Weapons)
		{
			// Weapon textures
			if (const std::string WeaponTextureDataConfFile{ fmt::format("./DATA/Weapons/Weapon_{}_TexturesData.conf", Weapon.Number) }; Tools_ErrorHandling::CheckFileExistence(WeaponTextureDataConfFile, ShowMessage, StopOnError))
			{
				std::ifstream WeaponTexturesData(WeaponTextureDataConfFile);
				std::string Line;

				while (std::getline(WeaponTexturesData, Line))
				{
					const TextureStruct TempTextureWeapon{ GFX_ImageHandling::ImportImage(Line) };

					Weapon.WeaponRect.w = TempTextureWeapon.Width;
					Weapon.WeaponRect.h = TempTextureWeapon.Height;

					Weapon.WeaponShader.LoadTextureInGPU(TempTextureWeapon, &Weapon.WeaponTexture);
				}
			}

			// Muzzle flash
			if (const std::string MuzzleFlashTextureDataConfFile{ fmt::format("./DATA/Weapons/Weapon_{}_MuzzleFlashTexturesData.conf", Weapon.Number) }; Tools_ErrorHandling::CheckFileExistence(MuzzleFlashTextureDataConfFile, ShowMessage, StopOnError))
			{
				std::ifstream MuzzleFlashTextureData(MuzzleFlashTextureDataConfFile);
				std::string Line;

				while (std::getline(MuzzleFlashTextureData, Line))
				{
					const TextureStruct TempTextureMuzzleFlash{ GFX_ImageHandling::ImportImage(Line) };

					Weapon.MuzzleFlashRect.w = TempTextureMuzzleFlash.Width;
					Weapon.MuzzleFlashRect.h = TempTextureMuzzleFlash.Height;

					Weapon.MuzzleFlashShader.LoadTextureInGPU(TempTextureMuzzleFlash, &Weapon.MuzzleFlashTexture);
				}
			}
		}
	}

	inline void InitAudio()
	{
		for (auto&& Weapon : Weapons)
		{
			Weapon.Sounds.clear();
			Weapon.Sounds.shrink_to_fit();

			if (const std::string INIFile{ fmt::format("./DATA/Weapons/Weapon_{}_Data.ini", Weapon.Number) }; Tools_ErrorHandling::CheckFileExistence(INIFile, ShowMessage, StopOnError))
			{
				// Get SingleShot Audio (Index = 0)
				Weapon.Sounds.emplace_back(SFX_SDL::LoadAudioFile(Tools_INIFile::ReadValue<std::string>(INIFile, "AUDIO", "SingleShotAudio")));

				// Get Dry Fire Audio (Index = 1)
				Weapon.Sounds.emplace_back(SFX_SDL::LoadAudioFile(Tools_INIFile::ReadValue<std::string>(INIFile, "AUDIO", "DryFireAudio")));

				// Get Reload Audio (Index = 2)
				Weapon.Sounds.emplace_back(SFX_SDL::LoadAudioFile(Tools_INIFile::ReadValue<std::string>(INIFile, "AUDIO", "ReloadAudio")));

				// Get length of audio file in ms and store to weapon data
				Weapon.ReloadDuration = static_cast<std::int_fast32_t>(Weapon.Sounds[static_cast<std::int_fast32_t>(WeaponsSounds::Reload)]->alen / 1000) >> 2;
			}
		}
	}

	inline void CheckForHit()
	{
		if (Weapons[Player.SelectedWeapon].Type == static_cast<std::int_fast32_t>(WeaponType::DirectHit))
		{
			//
			// Follow a ray to the middle of the screen (=center of crosshair) to check if part of entity was hit
			//
			// Re-use code from CastGraphics and RenderEntities
			// but deleted anything used for drawing lines...
			//

			const float Camera{ (lwmf::ViewportWidthMid << 1) / static_cast<float>(lwmf::ViewportWidth) - 1 };
			const PointFloat RayDir{ Player.Dir.X + Plane.X * Camera , Player.Dir.Y + Plane.Y * Camera };
			PointInt MapPos{ static_cast<std::int_fast32_t>(Player.Pos.X), static_cast<std::int_fast32_t>(Player.Pos.Y) };
			const PointFloat DeltaDist{ std::abs(1.0F / RayDir.X), std::abs(1.0F / RayDir.Y) };
			PointFloat SideDist;
			PointInt Step;

			RayDir.X < 0.0F ? (Step.X = -1, SideDist.X = (Player.Pos.X - MapPos.X) * DeltaDist.X) : (Step.X = 1, SideDist.X = (MapPos.X + 1 - Player.Pos.X) * DeltaDist.X);
			RayDir.Y < 0.0F ? (Step.Y = -1, SideDist.Y = (Player.Pos.Y - MapPos.Y) * DeltaDist.Y) : (Step.Y = 1, SideDist.Y = (MapPos.Y + 1 - Player.Pos.Y) * DeltaDist.Y);

			bool Endloop{};

			while (!Endloop)
			{
				SideDist.X < SideDist.Y ? (SideDist.X += DeltaDist.X, MapPos.X += Step.X) : (SideDist.Y += DeltaDist.Y, MapPos.Y += Step.Y);

				// If wall was hit and no entity -> end while loop
				if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][MapPos.X][MapPos.Y] > 0)
				{
					Endloop = true;
				}

				//
				// No wall was hit in this step, so check for entity
				//

				if (!Endloop)
				{
					const float InverseMatrix{ 1.0F / (Plane.X * Player.Dir.Y - Player.Dir.X * Plane.Y) };
					const std::int_fast32_t NumberOfEntities{ static_cast<std::int_fast32_t>(Entities.size()) };

					for (std::int_fast32_t Index{}; Index < NumberOfEntities; ++Index)
					{
						if (!Entities[Game_EntityHandling::EntityOrder[Index]].IsDead && !Endloop)
						{
							const std::int_fast32_t TextureIndex{ Game_EntityHandling::GetEntityTextureIndex(Index) };
							const PointFloat EntityPos{ Entities[Game_EntityHandling::EntityOrder[Index]].Pos.X - Player.Pos.X, Entities[Game_EntityHandling::EntityOrder[Index]].Pos.Y - Player.Pos.Y };
							const float TransY{ InverseMatrix * (-Plane.Y * EntityPos.X + Plane.X * EntityPos.Y) };
							const std::int_fast32_t vScreen{ static_cast<std::int_fast32_t>(Entities[Game_EntityHandling::EntityOrder[Index]].MoveV / TransY) };
							const std::int_fast32_t EntitySizeTemp{ static_cast<std::int_fast32_t>(lwmf::ViewportHeight / TransY) };
							const std::int_fast32_t EntitySX{ static_cast<std::int_fast32_t>(lwmf::ViewportWidthMid * (1.0F + InverseMatrix * (Player.Dir.Y * EntityPos.X - Player.Dir.X * EntityPos.Y) / TransY)) };
							const std::int_fast32_t LineEndX{ (std::min)((EntitySizeTemp >> 1) + EntitySX, lwmf::ViewportWidth) };
							const std::int_fast32_t TextureY{ (((((lwmf::ViewportHeightMid - vScreen) << 8) - ((lwmf::ViewportHeight + VerticalLook) << 7) + (EntitySizeTemp << 7)) * EntitySize) / EntitySizeTemp) >> 8 };

							for (std::int_fast32_t x{ -(EntitySizeTemp >> 1) + EntitySX }; x < LineEndX; ++x)
							{
								// Only check if x = center of crosshair and if entity is in line of view
								// Check if entity was hit in colored area

								const std::int_fast32_t TextureX{ ((x - ((-EntitySizeTemp >> 1) + EntitySX)) * EntitySize / EntitySizeTemp) };

								if ((x == lwmf::ViewportWidthMid && TransY < Game_EntityHandling::ZBuffer[x]) &&
									((EntityAssets[Entities[Entities[Game_EntityHandling::EntityOrder[Index]].Number].TypeNumber].WalkingTextures[TextureIndex][Entities[Game_EntityHandling::EntityOrder[Index]].WalkAnimStep].Texture[TextureY * TextureSize + TextureX] & lwmf::AMask) != 0))
								{
									Game_EntityHandling::HandleEntityHit(Entities[Entities[Game_EntityHandling::EntityOrder[Index]].Number]);

									// Shot found its way, end loop
									Endloop = true;
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	inline void HandleAmmoBoxPickup()
	{
		if (Game_EntityHandling::EntityMap[static_cast<std::int_fast32_t>(Player.Pos.X)][static_cast<std::int_fast32_t>(Player.Pos.Y)] == Game_EntityHandling::EntityTypes::AmmoBox)
		{
			for (auto&& Entity : Entities)
			{
				if (Entity.Type == static_cast<std::int_fast32_t>(Game_EntityHandling::EntityTypes::AmmoBox) && static_cast<std::int_fast32_t>(Player.Pos.X) == static_cast<std::int_fast32_t>(Entity.Pos.X) && static_cast<std::int_fast32_t>(Player.Pos.Y) == static_cast<std::int_fast32_t>(Entity.Pos.Y))
				{
					Game_EntityHandling::PlayAudio(Entity.TypeNumber, Game_EntityHandling::EntitySounds::AmmoBoxPickup);
					Entity.IsDead = true;

					for (auto&& Weapon : Weapons)
					{
						if (const auto WP{ Entity.ContainedItem.find(Weapon.Name) }; Weapon.Name == WP->first)
						{
							Weapon.CarriedAmmo += WP->second;
							Weapon.HUDCarriedAmmoInfo = fmt::format("Carried:{}", Weapon.CarriedAmmo);
						}
					}
				}
			}
		}
	}

	inline void InitiateRapidFire()
	{
		if (CurrentFiringState == FiringState::None && CurrentWeaponState == WeaponState::Ready)
		{
			CurrentFiringState = FiringState::RapidFire;
		}
	}

	inline void ReleaseRapidFire()
	{
		CurrentFiringState = FiringState::None;
	}

	inline void InitiateSingleShot()
	{
		if (CurrentFiringState == FiringState::None && CurrentWeaponState == WeaponState::Ready)
		{
			CurrentFiringState = FiringState::SingleShot;
		}
	}

	inline void InitiateReload()
	{
		if (CurrentFiringState == FiringState::None && CurrentWeaponState == WeaponState::Ready)
		{
			// Is reload currently not initiated and is there carried ammo left?
			if (Weapons[Player.SelectedWeapon].ReloadCounter == 0 && Weapons[Player.SelectedWeapon].CarriedAmmo > 0)
			{
				Weapons[Player.SelectedWeapon].ReloadCounter = Weapons[Player.SelectedWeapon].ReloadDuration;
				CurrentWeaponState = WeaponState::ReloadInitiated;
				PlayAudio(Player.SelectedWeapon, WeaponsSounds::Reload);
			}
		}
	}

	inline void InitiateWeaponChangeUp()
	{
		if (!WeaponMuzzleFlashFlag && CurrentWeaponState == WeaponState::Ready)
		{
			CurrentWeaponState = WeaponState::ChangeUp;
		}
	}

	inline void InitiateWeaponChangeDown()
	{
		if (!WeaponMuzzleFlashFlag && CurrentWeaponState == WeaponState::Ready)
		{
			CurrentWeaponState = WeaponState::ChangeDown;
		}
	}

	inline void FireWeapon()
	{
		if (CurrentFiringState == FiringState::SingleShot || CurrentFiringState == FiringState::RapidFire)
		{
			// Is weapon loaded and ready?
			if (Weapons[Player.SelectedWeapon].LoadedRounds > 0 && Weapons[Player.SelectedWeapon].CadenceCounter <= 0)
			{
				Weapons[Player.SelectedWeapon].CadenceCounter = 1000 / Weapons[Player.SelectedWeapon].Cadence;

				PlayAudio(Player.SelectedWeapon, WeaponsSounds::Shot);

				// Set flag and duration for rendering muzzleflash (used in DrawPlayerWeapon)
				WeaponMuzzleFlashFlag = true;
				Weapons[Player.SelectedWeapon].MuzzleFlashCounter = Weapons[Player.SelectedWeapon].MuzzleFlashDuration;

				--Weapons[Player.SelectedWeapon].LoadedRounds;

				// Did the shot hit anything?
				CheckForHit();
			}
			// Weapon is empty
			else if (CurrentFiringState == FiringState::SingleShot)
			{
				PlayAudio(Player.SelectedWeapon, WeaponsSounds::Dryfire);
			}
		}

		// Build ammo info HUD string
		Weapons[Player.SelectedWeapon].LoadedRounds < 10 ? Weapons[Player.SelectedWeapon].HUDAmmoInfo = fmt::format("0{0}/{1}", Weapons[Player.SelectedWeapon].LoadedRounds, Weapons[Player.SelectedWeapon].Capacity) :
			Weapons[Player.SelectedWeapon].HUDAmmoInfo = fmt::format("{0}/{1}", Weapons[Player.SelectedWeapon].LoadedRounds, Weapons[Player.SelectedWeapon].Capacity);

		if (CurrentFiringState == FiringState::SingleShot)
		{
			CurrentFiringState = FiringState::None;
		}
	}

	inline void CheckReloadStatus()
	{
		if (CurrentWeaponState == WeaponState::ReloadInitiated)
		{
			// Is counter still running?
			if (Weapons[Player.SelectedWeapon].ReloadCounter > 0)
			{
				--Weapons[Player.SelectedWeapon].ReloadCounter;
			}
			// Counter is finished, reload weapon and calculate remaining ammo
			else
			{
				// More ammo than needed?
				if (const std::int_fast32_t TempAmmo{ Weapons[Player.SelectedWeapon].Capacity - Weapons[Player.SelectedWeapon].LoadedRounds }; Weapons[Player.SelectedWeapon].CarriedAmmo >= TempAmmo)
				{
					Weapons[Player.SelectedWeapon].LoadedRounds += TempAmmo;
					Weapons[Player.SelectedWeapon].CarriedAmmo -= TempAmmo;
				}
				// Not enough ammo to fill magazine completely?
				else
				{
					Weapons[Player.SelectedWeapon].LoadedRounds += Weapons[Player.SelectedWeapon].CarriedAmmo;
					Weapons[Player.SelectedWeapon].CarriedAmmo = 0;
				}

				Weapons[Player.SelectedWeapon].HUDAmmoInfo = fmt::format("{0}/{1}", Weapons[Player.SelectedWeapon].LoadedRounds, Weapons[Player.SelectedWeapon].Capacity);
				Weapons[Player.SelectedWeapon].HUDCarriedAmmoInfo = fmt::format("Carried: {}", Weapons[Player.SelectedWeapon].CarriedAmmo);

				// Reloading is finished
				CurrentWeaponState = WeaponState::Ready;
			}
		}
	}

	inline void ChangeWeapon()
	{
		if ((CurrentWeaponState == WeaponState::ChangeUp || CurrentWeaponState == WeaponState::ChangeDown) && WeaponHeightFadeInOut == 0)
		{
			WeaponHeightFadeInOut = Weapons[Player.SelectedWeapon].WeaponRect.h;
		}
		else if (WeaponHeightFadeInOut > 0)
		{
			WeaponHeightFadeInOut -= Weapons[Player.SelectedWeapon].FadeInOutSpeed;
			WeaponFadeInOutY += Weapons[Player.SelectedWeapon].FadeInOutSpeed;

			if (WeaponHeightFadeInOut <= 0)
			{
				// Change weapons in "circle" - begin at first weapon if reached last one and vice versa
				if (CurrentWeaponState == WeaponState::ChangeUp)
				{
					Player.SelectedWeapon < Weapons.back().Number ? ++Player.SelectedWeapon : Player.SelectedWeapon = 0;
				}
				else if (CurrentWeaponState == WeaponState::ChangeDown)
				{
					Player.SelectedWeapon > 0 ? --Player.SelectedWeapon : Player.SelectedWeapon = Weapons.back().Number;
				}

				WeaponFadeInOutY = Weapons[Player.SelectedWeapon].WeaponRect.h;
			}
		}
		else if (WeaponFadeInOutY > 0)
		{
			WeaponFadeInOutY -= Weapons[Player.SelectedWeapon].FadeInOutSpeed;

			if (WeaponFadeInOutY <= 0)
			{
				CurrentWeaponState = WeaponState::Ready;
				WeaponHeightFadeInOut = 0;
				WeaponFadeInOutY = 0;
			}
		}
	}

	inline void CountdownMuzzleFlashCounter()
	{
		if (--Weapons[Player.SelectedWeapon].MuzzleFlashCounter == 0)
		{
			WeaponMuzzleFlashFlag = false;
		}
	}

	inline void CountdownCadenceCounter()
	{
		if (Weapons[Player.SelectedWeapon].CadenceCounter > 0)
		{
			--Weapons[Player.SelectedWeapon].CadenceCounter;
		}
	}

	inline void DrawWeapon()
	{
		const float PaceWeightProduct{ WeaponPace * Weapons[Player.SelectedWeapon].Weight };

		// Swaypattern is some kind of simple Lissajous figure...
		const PointInt Sway{ Weapons[Player.SelectedWeapon].WeaponRect.x + static_cast<std::int_fast32_t>(std::cosf(PaceWeightProduct) * 6.0F), Weapons[Player.SelectedWeapon].WeaponRect.y + static_cast<std::int_fast32_t>(std::sinf(PaceWeightProduct * 1.5F) * 6.0F) }; //-V807

		// Draw muzzle flash if weapon is fired
		if (WeaponMuzzleFlashFlag)
		{
			Weapons[Player.SelectedWeapon].MuzzleFlashShader.RenderTexture(&Weapons[Player.SelectedWeapon].MuzzleFlashTexture, Sway.X + Weapons[Player.SelectedWeapon].MuzzleFlashRect.x, Sway.Y - Weapons[Player.SelectedWeapon].MuzzleFlashRect.y, Weapons[Player.SelectedWeapon].MuzzleFlashRect.w, Weapons[Player.SelectedWeapon].MuzzleFlashRect.h); //-V807
		}
		// Draw weapon
		Weapons[Player.SelectedWeapon].WeaponShader.RenderTexture(&Weapons[Player.SelectedWeapon].WeaponTexture, Sway.X, Sway.Y + WeaponFadeInOutY, Weapons[Player.SelectedWeapon].WeaponRect.w, Weapons[Player.SelectedWeapon].WeaponRect.h);
	}

	inline void PlayAudio(const std::int_fast32_t SelectedPlayerWeapon, const WeaponsSounds WeaponSound)
	{
		Mix_PlayChannel(-1, Weapons[SelectedPlayerWeapon].Sounds[static_cast<std::int_fast32_t>(WeaponSound)], 0);
	}


} // namespace Game_WeaponHandling