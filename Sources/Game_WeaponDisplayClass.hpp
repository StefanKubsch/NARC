/*
******************************************
*                                        *
* Game_WeaponDisplayClass.hpp	         *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
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
	lwmf::IntRectStruct WeaponHUDRect{};
	lwmf::ShaderClass CrosshairShader{};
};

inline void Game_WeaponDisplayClass::Init()
{
	NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init weapon hud...");

	if (const std::string INIFile{ GameConfigFolder + "HUDWeaponDisplayConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		// Crosshair settings
		const lwmf::TextureStruct TempTextureCrosshair{ GFX_ImageHandling::ImportImage(lwmf::ReadINIValue<std::string>(INIFile, "HUD", "CrosshairFileName")) };

		CrosshairShader.LoadShader("Default", Canvas);
		CrosshairShader.LoadStaticTextureInGPU(TempTextureCrosshair, &CrosshairShader.OGLTextureID, Canvas.WidthMid - (TempTextureCrosshair.Width >> 1), Canvas.HeightMid - (TempTextureCrosshair.Height >> 1), TempTextureCrosshair.Width, TempTextureCrosshair.Height);

		// Weapon HUD settings
		const lwmf::TextureStruct TempTextureWeaponHUD{ GFX_ImageHandling::ImportImage(lwmf::ReadINIValue<std::string>(INIFile, "HUD", "WeaponHUDFileName")) };

		WeaponHUDRect.X = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "HUD", "WeaponHUDPosX");
		WeaponHUDRect.Y = lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "HUD", "WeaponHUDPosY");
		WeaponHUDRect.Width = TempTextureWeaponHUD.Width;
		WeaponHUDRect.Height = TempTextureWeaponHUD.Height;

		WeaponHUDShader.LoadShader("Default", Canvas);
		WeaponHUDShader.LoadStaticTextureInGPU(TempTextureWeaponHUD, &WeaponHUDShader.OGLTextureID, WeaponHUDRect.X, WeaponHUDRect.Y, WeaponHUDRect.Width, WeaponHUDRect.Height);

		AmmoText.InitFont(GameConfigFolder + "HUDWeaponDisplayConfig.ini", "HUDAMMOFONT");
		CarriedAmmoText.InitFont(GameConfigFolder + "HUDWeaponDisplayConfig.ini", "HUDCARRIEDAMMOFONT");
		WeaponText.InitFont(GameConfigFolder + "HUDWeaponDisplayConfig.ini", "HUDWEAPONFONT");
	}
}

inline void Game_WeaponDisplayClass::Display()
{
	const lwmf::IntPointStruct WeaponTextPos{ WeaponHUDRect.X + WeaponText.GetOffset().X, WeaponHUDRect.Y + WeaponText.GetOffset().Y };
	const lwmf::IntPointStruct CarriedAmmoTextPos{ WeaponHUDRect.X + CarriedAmmoText.GetOffset().X, WeaponHUDRect.Y + CarriedAmmoText.GetOffset().Y };
	const lwmf::IntPointStruct AmmoTextPos{ WeaponHUDRect.X + AmmoText.GetOffset().X, WeaponHUDRect.Y + AmmoText.GetOffset().Y };

	CrosshairShader.RenderStaticTexture(&CrosshairShader.OGLTextureID, true, 1.0F);
	WeaponHUDShader.RenderStaticTexture(&WeaponHUDShader.OGLTextureID, true, 1.0F);

	WeaponText.RenderText(Weapons[Player.SelectedWeapon].Name, WeaponTextPos.X, WeaponTextPos.Y);
	CarriedAmmoText.RenderText(Weapons[Player.SelectedWeapon].HUDCarriedAmmoInfo, CarriedAmmoTextPos.X, CarriedAmmoTextPos.Y);
	AmmoText.RenderText(Weapons[Player.SelectedWeapon].HUDAmmoInfo, AmmoTextPos.X, AmmoTextPos.Y);
}