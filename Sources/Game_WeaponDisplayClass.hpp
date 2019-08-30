/*
******************************************
*                                        *
* Game_WeaponDisplayClass.hpp	         *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "GFX_ImageHandling.hpp"
#include "GFX_TextClass.hpp"
#include "Game_DataStructures.hpp"

class Game_WeaponDisplayClass final
{
public:
	void Init();
	void Display();

private:
	GFX_TextClass AmmoText{};
	GFX_TextClass CarriedAmmoText{};
	GFX_TextClass WeaponText{};

	lwmf::ShaderClass WeaponHUDShader{};
	GLuint WeaponHUDTexture{};
	lwmf::IntRectStruct WeaponHUDRect{};

	lwmf::ShaderClass CrosshairShader{};
	GLuint CrosshairTexture{};
};

inline void Game_WeaponDisplayClass::Init()
{
	if (const std::string INIFile{ "./DATA/GameConfig/HUDWeaponDisplayConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		// Crosshair settings
		lwmf::TextureStruct TempTextureCrosshair{ GFX_ImageHandling::ImportImage(lwmf::ReadINIValue<std::string>(INIFile, "HUD", "CrosshairFileName")) };

		CrosshairShader.LoadShader("Default", ScreenTexture);
		CrosshairShader.LoadStaticTextureInGPU(TempTextureCrosshair, &CrosshairTexture, ScreenTexture.WidthMid - (TempTextureCrosshair.Width >> 1), ScreenTexture.HeightMid - (TempTextureCrosshair.Height >> 1), TempTextureCrosshair.Width, TempTextureCrosshair.Height);

		// Weapon HUD settings
		lwmf::TextureStruct TempTextureWeaponHUD{ GFX_ImageHandling::ImportImage(lwmf::ReadINIValue<std::string>(INIFile, "HUD", "WeaponHUDFileName")) };

		WeaponHUDRect.X = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "HUD", "WeaponHUDPosX");
		WeaponHUDRect.Y = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "HUD", "WeaponHUDPosY");
		WeaponHUDRect.Width = TempTextureWeaponHUD.Width;
		WeaponHUDRect.Height = TempTextureWeaponHUD.Height;

		WeaponHUDShader.LoadShader("Default", ScreenTexture);
		WeaponHUDShader.LoadStaticTextureInGPU(TempTextureWeaponHUD, &WeaponHUDTexture, WeaponHUDRect.X, WeaponHUDRect.Y, WeaponHUDRect.Width, WeaponHUDRect.Height);

		AmmoText.InitFont("./DATA/GameConfig/HUDWeaponDisplayConfig.ini", "HUDAMMOFONT");
		CarriedAmmoText.InitFont("./DATA/GameConfig/HUDWeaponDisplayConfig.ini", "HUDCARRIEDAMMOFONT");
		WeaponText.InitFont("./DATA/GameConfig/HUDWeaponDisplayConfig.ini", "HUDWEAPONFONT");
	}
}

inline void Game_WeaponDisplayClass::Display()
{
	CrosshairShader.RenderStaticTexture(&CrosshairTexture, 1.0F);
	WeaponHUDShader.RenderStaticTexture(&WeaponHUDTexture, 1.0F);

	WeaponText.RenderText(Weapons[Player.SelectedWeapon].Name, WeaponHUDRect.X + WeaponText.GetOffset().X, WeaponHUDRect.Y + WeaponText.GetOffset().Y);
	CarriedAmmoText.RenderText(Weapons[Player.SelectedWeapon].HUDCarriedAmmoInfo, WeaponHUDRect.X + CarriedAmmoText.GetOffset().X, WeaponHUDRect.Y + CarriedAmmoText.GetOffset().Y);
	AmmoText.RenderText(Weapons[Player.SelectedWeapon].HUDAmmoInfo, WeaponHUDRect.X + AmmoText.GetOffset().X, WeaponHUDRect.Y + AmmoText.GetOffset().Y);
}