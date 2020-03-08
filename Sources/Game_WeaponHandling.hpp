/*
******************************************
*                                        *
* Game_WeaponHandling.hpp                *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include <fstream>
#include <charconv>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
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
	void CloseAudio();

	//
	// Variables and constants
	//

	constexpr std::int_fast32_t MaximumCarriedAmmoDigits{ 3 };
	constexpr std::int_fast32_t MaximumAmmoCapacityDigits{ 3 };
	constexpr std::int_fast32_t MaximumLoadedRoundsDigits{ 3 };

	inline WeaponState CurrentWeaponState{};
	inline FiringState CurrentFiringState{};

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
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init weapons...");

		Weapons.clear();
		Weapons.shrink_to_fit();

		std::int_fast32_t Index{};

		while (true)
		{
			std::string INIFile{ AssetsWeaponsFolder };
			INIFile += "Weapon_";
			INIFile += std::to_string(Index);
			INIFile += "_Data.ini";

			if (Tools_ErrorHandling::CheckFileExistence(INIFile, ContinueOnError))
			{
				Weapons.emplace_back();

				Weapons[Index].Number = Index;
				Weapons[Index].Name = lwmf::ReadINIValue<std::string>(INIFile, "DATA", "Name");
				Weapons[Index].Weight = lwmf::ReadINIValue<float>(INIFile, "DATA", "Weight");
				Weapons[Index].Capacity = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "DATA", "Capacity");
				Weapons[Index].PaceFactor = lwmf::ReadINIValue<float>(INIFile, "DATA", "PaceFactor");
				Weapons[Index].Damage = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "DATA", "Damage");

				if (const std::string WeaponTypeString{ lwmf::ReadINIValue<std::string>(INIFile, "DATA", "WeaponType") }; WeaponTypeString == "DirectHit")
				{
					Weapons[Index].Type = static_cast<std::int_fast32_t>(WeaponType::DirectHit);
				}

				Weapons[Index].CarriedAmmo = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "DATA", "CarriedAmmo");
				Weapons[Index].Cadence = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "DATA", "Cadence");
				Weapons[Index].WeaponRect.X = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "POSITION", "PosX");
				Weapons[Index].WeaponRect.Y = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "POSITION", "PosY");
				Weapons[Index].FadeInOutSpeed = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "POSITION", "FadeInOutSpeed");
				Weapons[Index].MuzzleFlashDuration = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "MUZZLEFLASH", "MuzzleFlashDuration");
				Weapons[Index].MuzzleFlashRect.X = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "MUZZLEFLASH", "MuzzleFlashPosX");
				Weapons[Index].MuzzleFlashRect.Y = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "MUZZLEFLASH", "MuzzleFlashPosY");

				// pre-load weapon
				Weapons[Index].LoadedRounds = Weapons[Index].Capacity;

				// Initial built of ammo info HUD strings
				std::array<char, MaximumAmmoCapacityDigits> CapacityString{};
				std::to_chars(CapacityString.data(), CapacityString.data() + CapacityString.size(), Weapons[Index].Capacity);
				Weapons[Index].HUDAmmoInfo = std::string(CapacityString.data()) + "/" + std::string(CapacityString.data());

				std::array<char, MaximumCarriedAmmoDigits> CarriedAmmoString{};
				std::to_chars(CarriedAmmoString.data(), CarriedAmmoString.data() + CarriedAmmoString.size(), Weapons[Index].CarriedAmmo);
				Weapons[Index].HUDCarriedAmmoInfo = "Carried:" + std::string(CarriedAmmoString.data());

				// Load Shader
				Weapons[Index].WeaponShader.LoadShader("Default", ScreenTexture);
				Weapons[Index].MuzzleFlashShader.LoadShader("Default", ScreenTexture);

				++Index;
			}
			else
			{
				NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "No more weapon data found.");
				break;
			}
		}

		if (Weapons.empty())
		{
			NARCLog.AddEntry(lwmf::LogLevel::Error, __FILENAME__, __LINE__, "No weapon data found!");
		}
	}

	inline void InitTextures()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Load weapon textures...");

		for (auto&& Weapon : Weapons)
		{
			// Weapon textures
			std::string WeaponTextureDataConfFile{ AssetsWeaponsFolder };
			WeaponTextureDataConfFile += "Weapon_";
			WeaponTextureDataConfFile += std::to_string(Weapon.Number);
			WeaponTextureDataConfFile += "_TexturesData.conf";

			if (Tools_ErrorHandling::CheckFileExistence(WeaponTextureDataConfFile, StopOnError))
			{
				std::ifstream WeaponTexturesData(WeaponTextureDataConfFile, std::ios::in);

				std::string Line;

				while (std::getline(WeaponTexturesData, Line))
				{
					const lwmf::TextureStruct TempTextureWeapon{ GFX_ImageHandling::ImportImage(Line) };

					Weapon.WeaponRect.Width = TempTextureWeapon.Width;
					Weapon.WeaponRect.Height = TempTextureWeapon.Height;

					Weapon.WeaponShader.LoadTextureInGPU(TempTextureWeapon, &Weapon.WeaponShader.OGLTextureID);
				}
			}

			// Muzzle flash
			std::string MuzzleFlashTextureDataConfFile{ AssetsWeaponsFolder };
			MuzzleFlashTextureDataConfFile += "Weapon_";
			MuzzleFlashTextureDataConfFile += std::to_string(Weapon.Number);
			MuzzleFlashTextureDataConfFile += "_MuzzleFlashTexturesData.conf";

			if (Tools_ErrorHandling::CheckFileExistence(MuzzleFlashTextureDataConfFile, StopOnError))
			{
				std::ifstream MuzzleFlashTextureData(MuzzleFlashTextureDataConfFile, std::ios::in);

				std::string Line;

				while (std::getline(MuzzleFlashTextureData, Line))
				{
					const lwmf::TextureStruct TempTextureMuzzleFlash{ GFX_ImageHandling::ImportImage(Line) };

					Weapon.MuzzleFlashRect.Width = TempTextureMuzzleFlash.Width;
					Weapon.MuzzleFlashRect.Height = TempTextureMuzzleFlash.Height;

					Weapon.MuzzleFlashShader.LoadTextureInGPU(TempTextureMuzzleFlash, &Weapon.MuzzleFlashShader.OGLTextureID);
				}
			}
		}
	}

	inline void InitAudio()
	{
		CloseAudio();

		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Load weapon audio...");

		for (auto&& Weapon : Weapons)
		{
			Weapon.Sounds.clear();
			Weapon.Sounds.shrink_to_fit();

			std::string INIFile{ AssetsWeaponsFolder };
			INIFile += "Weapon_";
			INIFile += std::to_string(Weapon.Number);
			INIFile += "_Data.ini";

			if (Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
			{
				Weapon.Sounds.emplace_back();
				Weapon.Sounds[static_cast<std::int_fast32_t>(WeaponsSounds::Shot)].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "SingleShotAudio"));

				Weapon.Sounds.emplace_back();
				Weapon.Sounds[static_cast<std::int_fast32_t>(WeaponsSounds::Dryfire)].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "DryFireAudio"));

				Weapon.Sounds.emplace_back();
				Weapon.Sounds[static_cast<std::int_fast32_t>(WeaponsSounds::Reload)].Load(lwmf::ReadINIValue<std::string>(INIFile, "AUDIO", "ReloadAudio"));

				Weapon.ReloadDuration = static_cast<std::int_fast32_t>(Weapon.Sounds[static_cast<std::int_fast32_t>(WeaponsSounds::Reload)].GetDuration() / static_cast<double>(FrameLock));
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

			const float Camera{ (ScreenTexture.WidthMid << 1) / static_cast<float>(ScreenTexture.Width) - 1 };
			const lwmf::FloatPointStruct RayDir{ Player.Dir.X + Plane.X * Camera , Player.Dir.Y + Plane.Y * Camera };
			lwmf::FloatPointStruct MapPos{ std::floorf(Player.Pos.X), std::floorf(Player.Pos.Y) };
			const lwmf::FloatPointStruct DeltaDist{ std::abs(1.0F / RayDir.X), std::abs(1.0F / RayDir.Y) };
			lwmf::FloatPointStruct SideDist{};
			lwmf::FloatPointStruct Step{};

			RayDir.X < 0.0F ? (Step.X = -1.0F, SideDist.X = (Player.Pos.X - MapPos.X) * DeltaDist.X) : (Step.X = 1.0F, SideDist.X = (MapPos.X + 1.0F - Player.Pos.X) * DeltaDist.X);
			RayDir.Y < 0.0F ? (Step.Y = -1.0F, SideDist.Y = (Player.Pos.Y - MapPos.Y) * DeltaDist.Y) : (Step.Y = 1.0F, SideDist.Y = (MapPos.Y + 1.0F - Player.Pos.Y) * DeltaDist.Y);

			bool Endloop{};

			while (!Endloop)
			{
				SideDist.X < SideDist.Y ? (SideDist.X += DeltaDist.X, MapPos.X += Step.X) : (SideDist.Y += DeltaDist.Y, MapPos.Y += Step.Y);

				// If wall was hit and no entity -> end while loop
				if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][static_cast<std::int_fast32_t>(MapPos.X)][static_cast<std::int_fast32_t>(MapPos.Y)] > 0)
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
							const lwmf::FloatPointStruct EntityPos{ Entities[Game_EntityHandling::EntityOrder[Index]].Pos.X - Player.Pos.X, Entities[Game_EntityHandling::EntityOrder[Index]].Pos.Y - Player.Pos.Y };
							const float TransY{ InverseMatrix * (-Plane.Y * EntityPos.X + Plane.X * EntityPos.Y) };
							const std::int_fast32_t vScreen{ static_cast<std::int_fast32_t>(Entities[Game_EntityHandling::EntityOrder[Index]].MoveV / TransY) };
							const std::int_fast32_t EntitySizeTemp{ static_cast<std::int_fast32_t>(ScreenTexture.Height / TransY) };
							const std::int_fast32_t EntitySX{ static_cast<std::int_fast32_t>(ScreenTexture.WidthMid * (1.0F + InverseMatrix * (Player.Dir.Y * EntityPos.X - Player.Dir.X * EntityPos.Y) / TransY)) };
							const std::int_fast32_t LineEndX{ (std::min)((EntitySizeTemp >> 1) + EntitySX, ScreenTexture.Width) };
							const std::int_fast32_t TextureY{ (((((ScreenTexture.HeightMid - vScreen) << 8) - ((ScreenTexture.Height + VerticalLook) << 7) + (EntitySizeTemp << 7)) * EntitySize) / EntitySizeTemp) >> 8 };

							for (std::int_fast32_t x{ -(EntitySizeTemp >> 1) + EntitySX }; x < LineEndX; ++x)
							{
								// Only check if x = center of crosshair and if entity is in line of view
								// Check if entity was hit in colored area

								const std::int_fast32_t TextureX{ ((x - ((-EntitySizeTemp >> 1) + EntitySX)) * EntitySize / EntitySizeTemp) };

								if ((x == ScreenTexture.WidthMid && TransY < Game_EntityHandling::ZBuffer[x]) &&
									((EntityAssets[Entities[Entities[Game_EntityHandling::EntityOrder[Index]].Number].TypeNumber].WalkingTextures[TextureIndex][Entities[Game_EntityHandling::EntityOrder[Index]].WalkAnimStep].Pixels[TextureY * TextureSize + TextureX] & lwmf::AMask) != 0))
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
		if (Game_EntityHandling::EntityMap[static_cast<std::int_fast32_t>(Player.Pos.X)][static_cast<std::int_fast32_t>(Player.Pos.Y)] == EntityTypes::AmmoBox)
		{
			for (auto&& Entity : Entities)
			{
				if (Entity.Type == EntityTypes::AmmoBox && static_cast<std::int_fast32_t>(Player.Pos.X) == static_cast<std::int_fast32_t>(Entity.Pos.X) && static_cast<std::int_fast32_t>(Player.Pos.Y) == static_cast<std::int_fast32_t>(Entity.Pos.Y))
				{
					Game_EntityHandling::PlayAudio(Entity.TypeNumber, Game_EntityHandling::EntitySounds::AmmoBoxPickup);
					Entity.IsDead = true;
					Entity.IsPickedUp = true;

					for (auto&& Weapon : Weapons)
					{
						if (const auto WP{ Entity.ContainedItem.find(Weapon.Name) }; Weapon.Name == WP->first)
						{
							Weapon.CarriedAmmo += WP->second;
							std::array<char, MaximumCarriedAmmoDigits> CarriedAmmoString{};
							std::to_chars(CarriedAmmoString.data(), CarriedAmmoString.data() + CarriedAmmoString.size(), Weapon.CarriedAmmo);
							Weapon.HUDCarriedAmmoInfo = "Carried:" + std::string(CarriedAmmoString.data());

							break;
						}
					}

					break;
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
				Weapons[Player.SelectedWeapon].CadenceCounter = (3600 / Weapons[Player.SelectedWeapon].Cadence) ;

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
		Weapons[Player.SelectedWeapon].LoadedRounds < 10 ? Weapons[Player.SelectedWeapon].HUDAmmoInfo = "0" + std::to_string(Weapons[Player.SelectedWeapon].LoadedRounds) + "/" + std::to_string(Weapons[Player.SelectedWeapon].Capacity) :
			Weapons[Player.SelectedWeapon].HUDAmmoInfo = std::to_string(Weapons[Player.SelectedWeapon].LoadedRounds) + "/" + std::to_string(Weapons[Player.SelectedWeapon].Capacity);

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

				// Update HUD informations
				std::array<char, MaximumLoadedRoundsDigits> LoadedRoundsString{};
				std::to_chars(LoadedRoundsString.data(), LoadedRoundsString.data() + LoadedRoundsString.size(), Weapons[Player.SelectedWeapon].LoadedRounds);
				std::array<char, MaximumAmmoCapacityDigits> CapacityString{};
				std::to_chars(CapacityString.data(), CapacityString.data() + CapacityString.size(), Weapons[Player.SelectedWeapon].Capacity);
				Weapons[Player.SelectedWeapon].HUDAmmoInfo = std::string(LoadedRoundsString.data()) + "/" + std::string(CapacityString.data());

				std::array<char, MaximumCarriedAmmoDigits> CarriedAmmoString{};
				std::to_chars(CarriedAmmoString.data(), CarriedAmmoString.data() + CarriedAmmoString.size(), Weapons[Player.SelectedWeapon].CarriedAmmo);
				Weapons[Player.SelectedWeapon].HUDCarriedAmmoInfo = "Carried:" + std::string(CarriedAmmoString.data());

				// Reloading is finished
				CurrentWeaponState = WeaponState::Ready;
			}
		}
	}

	inline void ChangeWeapon()
	{
		if ((CurrentWeaponState == WeaponState::ChangeUp || CurrentWeaponState == WeaponState::ChangeDown) && WeaponHeightFadeInOut == 0)
		{
			WeaponHeightFadeInOut = Weapons[Player.SelectedWeapon].WeaponRect.Height;
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

				WeaponFadeInOutY = Weapons[Player.SelectedWeapon].WeaponRect.Height;
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
		const lwmf::IntPointStruct Sway{ Weapons[Player.SelectedWeapon].WeaponRect.X + static_cast<std::int_fast32_t>(std::cosf(PaceWeightProduct) * 6.0F), Weapons[Player.SelectedWeapon].WeaponRect.Y + static_cast<std::int_fast32_t>(std::sinf(PaceWeightProduct * 1.5F) * 6.0F) }; //-V807

		// Draw muzzle flash if weapon is fired
		if (WeaponMuzzleFlashFlag)
		{
			Weapons[Player.SelectedWeapon].MuzzleFlashShader.RenderTexture(&Weapons[Player.SelectedWeapon].MuzzleFlashShader.OGLTextureID, Sway.X + Weapons[Player.SelectedWeapon].MuzzleFlashRect.X, Sway.Y - Weapons[Player.SelectedWeapon].MuzzleFlashRect.Y, Weapons[Player.SelectedWeapon].MuzzleFlashRect.Width, Weapons[Player.SelectedWeapon].MuzzleFlashRect.Height, true, 1.0F); //-V807
		}
		// Draw weapon
		Weapons[Player.SelectedWeapon].WeaponShader.RenderTexture(&Weapons[Player.SelectedWeapon].WeaponShader.OGLTextureID, Sway.X, Sway.Y + WeaponFadeInOutY, Weapons[Player.SelectedWeapon].WeaponRect.Width, Weapons[Player.SelectedWeapon].WeaponRect.Height, true, 1.0F);
	}

	inline void PlayAudio(const std::int_fast32_t SelectedPlayerWeapon, const WeaponsSounds WeaponSound)
	{
		Weapons[SelectedPlayerWeapon].Sounds[static_cast<std::int_fast32_t>(WeaponSound)].Play();
	}

	inline void CloseAudio()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Close weapon audio...");

		for (auto&& Weapon : Weapons)
		{
			for (auto&& Sound : Weapon.Sounds)
			{
				Sound.Close();
			}
		}
	}


} // namespace Game_WeaponHandling