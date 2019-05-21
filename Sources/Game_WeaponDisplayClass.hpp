/*
******************************************
*                                        *
* Game_WeaponDisplayClass.hpp	         *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#define FMT_HEADER_ONLY

#include <cstdint>
#include <string>
#include <SDL.h>
#include "fmt/format.h"

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"
#include "GFX_ImageHandling.hpp"
#include "GFX_TextClass.hpp"
#include "GFX_OpenGLShaderClass.hpp"
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

	GFX_OpenGLShaderClass WeaponHUDShader{};
	GLuint WeaponHUDTexture{};
	SDL_Rect WeaponHUDRect{};

	GFX_OpenGLShaderClass CrosshairShader{};
	GLuint CrosshairTexture{};
};

inline void Game_WeaponDisplayClass::Init()
{
	if (const std::string INIFile{ "./DATA/GameConfig/HUDWeaponDisplayConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, ShowMessage, StopOnError))
	{
		// Crosshair settings
		const TextureStruct TempTextureCrosshair{ GFX_ImageHandling::ImportImage(Tools_INIFile::ReadValue<std::string>(INIFile, "HUD", "CrosshairFileName")) };

		CrosshairShader.LoadShader("Default");
		CrosshairShader.LoadStaticTextureInGPU(TempTextureCrosshair, &CrosshairTexture, lwmf::ViewportWidthMid - (TempTextureCrosshair.Width >> 1), lwmf::ViewportHeightMid - (TempTextureCrosshair.Height >> 1), TempTextureCrosshair.Width, TempTextureCrosshair.Height);

		// Weapon HUD settings
		const TextureStruct TempTextureWeaponHUD{ GFX_ImageHandling::ImportImage(Tools_INIFile::ReadValue<std::string>(INIFile, "HUD", "WeaponHUDFileName")) };

		WeaponHUDRect.x = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "HUD", "WeaponHUDPosX");
		WeaponHUDRect.y = Tools_INIFile::ReadValue<std::int_fast32_t>(INIFile, "HUD", "WeaponHUDPosY");
		WeaponHUDRect.w = TempTextureWeaponHUD.Width;
		WeaponHUDRect.h = TempTextureWeaponHUD.Height;

		WeaponHUDShader.LoadShader("Default");
		WeaponHUDShader.LoadStaticTextureInGPU(TempTextureWeaponHUD, &WeaponHUDTexture, WeaponHUDRect.x, WeaponHUDRect.y, WeaponHUDRect.w, WeaponHUDRect.h);
	}

	AmmoText.InitFont("./DATA/GameConfig/HUDWeaponDisplayConfig.ini", "HUDAMMOFONT");
	CarriedAmmoText.InitFont("./DATA/GameConfig/HUDWeaponDisplayConfig.ini", "HUDCARRIEDAMMOFONT");
	WeaponText.InitFont("./DATA/GameConfig/HUDWeaponDisplayConfig.ini", "HUDWEAPONFONT");
}

inline void Game_WeaponDisplayClass::Display()
{
	CrosshairShader.RenderStaticTexture(&CrosshairTexture);
	WeaponHUDShader.RenderStaticTexture(&WeaponHUDTexture);

	WeaponText.RenderText(Weapons[Player.SelectedWeapon].Name, WeaponHUDRect.x + WeaponText.Offset.X, WeaponHUDRect.y + WeaponText.Offset.Y);
	CarriedAmmoText.RenderText(Weapons[Player.SelectedWeapon].HUDCarriedAmmoInfo, WeaponHUDRect.x + CarriedAmmoText.Offset.X, WeaponHUDRect.y + CarriedAmmoText.Offset.Y);
	AmmoText.RenderText(Weapons[Player.SelectedWeapon].HUDAmmoInfo, WeaponHUDRect.x + AmmoText.Offset.X, WeaponHUDRect.y + AmmoText.Offset.Y);
}