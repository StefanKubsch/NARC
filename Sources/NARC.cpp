/*
******************************************
* NARC                                   *
*                                        *
* "Not Another RayCaster"                *
*                                        *
* NARC.cpp                               *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
*                                        *
******************************************
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <cstdint>
#include <cmath>
#include <chrono>

// Uncomment to find memory leaks in debug mode
//
// Have a look here:
// https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2013/x98tx3cf(v=vs.120)
//
// #define _CRTDBG_MAP_ALLOC
// #include <stdlib.h>
// #include <crtdbg.h>

// ****************************
// * TECHNICAL HEADERS FIRST! *
// ****************************

// lightweight media framework
#define LWMF_LOGGINGENABLED
#define LWMF_THROWEXCEPTIONS
#include "./lwmf/lwmf.hpp"

// Establish logging for NARC itself - system-logging for lwmf is hardcoded!
lwmf::Logging NARCLog("NARC.log");

// "Canvas" is the main render target in our game!
inline lwmf::TextureStruct Canvas{};
inline lwmf::ShaderClass CanvasShader{};

#include "Game_Folder.hpp"
#include "Game_GlobalDefinitions.hpp"
#include "Tools_Console.hpp"
#include "Tools_ErrorHandling.hpp"
#include "GFX_ImageHandling.hpp"
#include "GFX_Window.hpp"
#include "GFX_TextClass.hpp"
#include "GFX_LightingClass.hpp"
#include "HID_Keyboard.hpp"
#include "HID_Mouse.hpp"
#include "HID_Gamepad.hpp"

// *************************************
// * NOW DATA & GAME RELEVANT HEADERS! *
// *************************************

#include "Game_PlayerClass.hpp"
#include "Game_DataStructures.hpp"
#include "Game_PreGame.hpp"
#include "Game_Config.hpp"
#include "Game_LevelHandling.hpp"
#include "Game_SkyboxHandling.hpp"
#include "Game_PathFinding.hpp"
#include "Game_EntityHandling.hpp"
#include "Game_Effects.hpp"
#include "Game_Doors.hpp"
#include "Game_WeaponHandling.hpp"
#include "Game_HealthBarClass.hpp"
#include "Game_MinimapClass.hpp"
#include "Game_WeaponDisplayClass.hpp"
#include "Game_Transitions.hpp"
#include "Game_MenuClass.hpp"
#include "Game_Raycaster.hpp"
#include "Tools_Cleanup.hpp"

//
// Declare functions
//

void InitAndLoadGameConfig();
void InitAndLoadLevel();
void MovePlayerAndCheckCollision();
void ControlPlayerMovement();

// Init objects
inline Game_MenuClass MainMenu;
inline Game_HealthBarClass HUDHealthBar;
inline Game_MinimapClass HUDMinimap;
inline Game_WeaponDisplayClass HUDWeaponDisplay;

inline bool HUDEnabled{ true };

std::int_fast32_t WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nShowCmd);

	lwmf::WindowInstance = hInstance;

	try
	{
		InitAndLoadGameConfig();
		InitAndLoadLevel();
	}
	catch (const std::runtime_error&)
	{
		return EXIT_FAILURE;
	}

	NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init multithreading threadpool...");
	lwmf::Multithreading ThreadPool;

	const std::int_fast32_t BlackNoAlpha{ lwmf::RGBAtoINT(0, 0, 0, 0) };
	const std::int_fast32_t White{ lwmf::RGBAtoINT(255, 255, 255, 255) };

	// Main game loop
	// fixed timestep method
	// loop until ESC is pressed

	LengthOfFrame = 1000 / FrameLock;
	std::uint_fast32_t Lag{};
	auto EndTime{ std::chrono::steady_clock::now() };

	while (!QuitGameFlag)
	{
		lwmf::CatchMouse(lwmf::MainWindow);

		if (Game_LevelHandling::BackgroundMusicEnabled && !Game_LevelHandling::BackgroundMusic[0].IsFinished())
		{
			Game_LevelHandling::PlayBackgroundMusic(0);
		}

		const auto StartTime{ std::chrono::steady_clock::now() };
		const auto ElapsedTime(std::chrono::duration_cast<std::chrono::milliseconds>(StartTime - EndTime));
		EndTime = StartTime;
		Lag += static_cast<std::uint_fast32_t>(ElapsedTime.count());

		HID_Gamepad::GameController.Refresh();

		MSG Message{};

		while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
		{
			if (Message.message == WM_QUIT)
			{
				NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "MESSAGE: WM_QUIT received...");
				break;
			}

			DispatchMessage(&Message);
		}

		while (Lag >= LengthOfFrame)
		{
			if (!GamePausedFlag)
			{
				ControlPlayerMovement();
				Game_EntityHandling::MoveEntities();
				Game_Doors::OpenCloseDoors();
				Game_WeaponHandling::ChangeWeapon();
				Game_WeaponHandling::CheckReloadStatus();
				Game_WeaponHandling::CountdownMuzzleFlashCounter();
				Game_WeaponHandling::CountdownCadenceCounter();
				Game_Effects::CountdownBloodstainCounter();
			}

			Lag -= LengthOfFrame;
		}

		Game_EntityHandling::GetEntityDistance();

		// Sort entities front to back for check if weapon hit first entity in front of player
		SortEntities(Game_EntityHandling::SortOrder::FrontToBack);
		Game_WeaponHandling::FireWeapon();

		// Sort entities back to front to draw them in right order
		SortEntities(Game_EntityHandling::SortOrder::BackToFront);

		lwmf::ClearTexture(Canvas, BlackNoAlpha);
		lwmf::FPSCounter();

		ThreadPool.AddThread(&Game_Raycaster::CastGraphics, Game_Raycaster::Renderpart::WallLeft);
		ThreadPool.AddThread(&Game_Raycaster::CastGraphics, Game_Raycaster::Renderpart::WalLRight);
		ThreadPool.AddThread(&Game_Raycaster::CastGraphics, Game_Raycaster::Renderpart::Floor);
		ThreadPool.AddThread(&Game_Raycaster::CastGraphics, Game_Raycaster::Renderpart::Ceiling);
		ThreadPool.WaitForThreads();

		Game_EntityHandling::RenderEntities();

		if (HUDEnabled)
		{
			HUDHealthBar.Display();
			lwmf::DisplayFPSCounter(Canvas, Canvas.Width - 70, 7, White);
		}

		if (Player.IsDead && !GamePausedFlag)
		{
			Game_Transitions::DeathSequence();
		}

		lwmf::ClearBuffer();

		Game_SkyboxHandling::Render();

		if (HUDMinimap.Enabled)
		{
			// Display realtime data (entities, waypoints etc.)
			HUDMinimap.DisplayRealtimeMap();
		}

		CanvasShader.RenderLWMFTexture(Canvas, true, 1.0F);
		Game_WeaponHandling::DrawWeapon();

		if (HUDEnabled)
		{
			HUDWeaponDisplay.Display();

			(GameControllerFlag && HID_Gamepad::GameController.ControllerID != -1) ? HID_Gamepad::XBoxControllerIconShader.RenderStaticTexture(&HID_Gamepad::XBoxControllerIconShader.OGLTextureID, true, 1.0F) :
				HID_Mouse::MouseIconShader.RenderStaticTexture(&HID_Mouse::MouseIconShader.OGLTextureID, true, 1.0F);
		}

		if (HUDMinimap.Enabled)
		{
			// Display pre-rendered overlay (Walls, Doors etc.)
			// This is a static texture
			HUDMinimap.DisplayPreRenderedMap();
		}

		Game_Effects::DrawBloodstain();

		if (GamePausedFlag)
		{
			MainMenu.Show();
		}

		// Render everything to screen!
		lwmf::SwapBuffer();
	}

	// Cleanup everything and exit the program...

	Tools_Cleanup::CloseAllAudio();
	Tools_Cleanup::DestroySubsystems();
	NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Exit program...");

	// Uncomment to find memory leaks in debug mode
	//
	// Have a look here:
	// https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2013/x98tx3cf(v=vs.120)
	//
	// _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// _CrtDumpMemoryLeaks();

	return EXIT_SUCCESS;
}

inline LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// Gamecontroller handling
		case WM_KEYDOWN:
		{
			if (wParam == static_cast<WPARAM>(HID_Keyboard::MovePlayerForwardKey))
			{
				HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerForwardKey, true);
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Keyboard::MovePlayerBackwardKey))
			{
				HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerBackwardKey, true);
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Keyboard::MovePlayerStrafeLeftKey))
			{
				HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeLeftKey, true);
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Keyboard::MovePlayerStrafeRightKey))
			{
				HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeRightKey, true);
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::VirtMouseLeftKey))
			{
				HID_Gamepad::GameController.RightStickPos.X = -1;
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::VirtMouseRightKey))
			{
				HID_Gamepad::GameController.RightStickPos.X = 1;
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::VirtMouseUpKey))
			{
				HID_Gamepad::GameController.RightStickPos.Y = -1;
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::VirtMouseDownKey))
			{
				HID_Gamepad::GameController.RightStickPos.Y = 1;
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::FireSingleShotKey))
			{
				Game_WeaponHandling::InitiateSingleShot();
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::RapidFireKey))
			{
				Game_WeaponHandling::InitiateRapidFire();
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Keyboard::ReloadWeaponKey))
			{
				Game_WeaponHandling::InitiateReload();
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Keyboard::ActionKey))
			{
				Game_Doors::TriggerDoor();
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::ChangeWeaponUpKey))
			{
				Game_WeaponHandling::InitiateWeaponChangeUp();
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::ChangeWeaponDownKey))
			{
				Game_WeaponHandling::InitiateWeaponChangeDown();
				break;
			}
			break;
		}
		case WM_KEYUP:
		{
			if (wParam == static_cast<WPARAM>(HID_Keyboard::MovePlayerForwardKey))
			{
				HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerForwardKey, false);
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Keyboard::MovePlayerBackwardKey))
			{
				HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerBackwardKey, false);
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Keyboard::MovePlayerStrafeLeftKey))
			{
				HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeLeftKey, false);
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Keyboard::MovePlayerStrafeRightKey))
			{
				HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeRightKey, false);
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::VirtMouseLeftKey))
			{
				HID_Gamepad::GameController.RightStickPos.X = 0;
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::VirtMouseRightKey))
			{
				HID_Gamepad::GameController.RightStickPos.X = 0;
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::VirtMouseUpKey))
			{
				HID_Gamepad::GameController.RightStickPos.Y = 0;
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::VirtMouseDownKey))
			{
				HID_Gamepad::GameController.RightStickPos.Y = 0;
				break;
			}

			if (wParam == static_cast<WPARAM>(HID_Gamepad::RapidFireKey))
			{
				Game_WeaponHandling::ReleaseRapidFire();
				break;
			}
			break;
		}
		// Mouse and keyboard handling
		case WM_INPUT:
		{
			RAWINPUT RawDev{};
			UINT DataSize{ sizeof(RAWINPUT) };
			UINT HeaderSize{ sizeof(RAWINPUTHEADER) };
			HRAWINPUT Handle{ reinterpret_cast<HRAWINPUT>(lParam) };
			GetRawInputData(Handle, RID_INPUT, &RawDev, &DataSize, HeaderSize);

			switch (RawDev.header.dwType)
			{
				case RIM_TYPEMOUSE:
				{
					HID_Mouse::MousePos = { RawDev.data.mouse.lLastX, RawDev.data.mouse.lLastY };

					if ((RawDev.data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN) != 0)
					{
						Game_WeaponHandling::InitiateSingleShot();
						break;
					}

					if ((RawDev.data.mouse.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN) != 0)
					{
						Game_WeaponHandling::InitiateRapidFire();
						break;
					}

					if ((RawDev.data.mouse.ulButtons & RI_MOUSE_RIGHT_BUTTON_UP) != 0)
					{
						Game_WeaponHandling::ReleaseRapidFire();
						break;
					}

					if ((RawDev.data.mouse.usButtonFlags & RI_MOUSE_WHEEL) != 0)
					{
						if ((*reinterpret_cast<short*>(& RawDev.data.mouse.usButtonData)) / WHEEL_DELTA < 0)
						{
							Game_WeaponHandling::InitiateWeaponChangeDown();
							break;
						}

						if ((*reinterpret_cast<short*>(&RawDev.data.mouse.usButtonData)) / WHEEL_DELTA > 0)
						{
							Game_WeaponHandling::InitiateWeaponChangeUp();
							break;
						}
					}
					break;
				}
				case RIM_TYPEKEYBOARD:
				{
					if (RawDev.data.keyboard.Message == WM_KEYDOWN || RawDev.data.keyboard.Message == WM_SYSKEYDOWN)
					{
						if (RawDev.data.keyboard.VKey == HID_Keyboard::PauseKey)
						{
							MainMenu.LevelUp();
							break;
						}

						if (GamePausedFlag)
						{
							if (RawDev.data.keyboard.VKey == HID_Keyboard::MenuItemDownKey)
							{
								MainMenu.ItemDown();
								break;
							}

							if (RawDev.data.keyboard.VKey == HID_Keyboard::MenuItemUpKey)
							{
								MainMenu.ItemUp();
								break;
							}

							if (RawDev.data.keyboard.VKey == HID_Keyboard::MenuItemSelectKey)
							{
								MainMenu.ItemSelect();
								break;
							}
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::ActionKey)
						{
							Game_Doors::TriggerDoor();
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::ReloadWeaponKey)
						{
							Game_WeaponHandling::InitiateReload();
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::HUDKey)
						{
							HUDEnabled = !HUDEnabled;
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::MiniMapKey)
						{
							HUDMinimap.Enabled = !HUDMinimap.Enabled;
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::DecreaseMouseSensitivityKey)
						{
							HID_Mouse::ChangeMouseSensitivity('-');
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::IncreaseMouseSensitivityKey)
						{
							HID_Mouse::ChangeMouseSensitivity('+');
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::SelectNextLevelKey)
						{
							if (NumberOfLevels > StartLevel)
							{
								SelectedLevel < NumberOfLevels ? ++SelectedLevel : SelectedLevel = StartLevel;

								try
								{
									InitAndLoadLevel();
								}
								catch (const std::runtime_error&)
								{
									return EXIT_FAILURE;
								}

								break;
							}
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::SwitchLightingKey)
						{
							Game_LevelHandling::LightingFlag = !Game_LevelHandling::LightingFlag;
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::MovePlayerForwardKey)
						{
							HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerForwardKey, true);
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::MovePlayerBackwardKey)
						{
							HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerBackwardKey, true);
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::MovePlayerStrafeLeftKey)
						{
							HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeLeftKey, true);
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::MovePlayerStrafeRightKey)
						{
							HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeRightKey, true);
							break;
						}
					}

					if (RawDev.data.keyboard.Message == WM_KEYUP || RawDev.data.keyboard.Message == WM_SYSKEYUP)
					{
						if (RawDev.data.keyboard.VKey == HID_Keyboard::MovePlayerForwardKey)
						{
							HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerForwardKey, false);
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::MovePlayerBackwardKey)
						{
							HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerBackwardKey, false);
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::MovePlayerStrafeLeftKey)
						{
							HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeLeftKey, false);
							break;
						}

						if (RawDev.data.keyboard.VKey == HID_Keyboard::MovePlayerStrafeRightKey)
						{
							HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeRightKey, false);
							break;
						}
					}
					break;
				}
				default: {}
			}
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		default: {}
	}

	return DefWindowProcA(hWnd, message, wParam, lParam);
}

inline void InitAndLoadGameConfig()
{
	lwmf::CheckForSSESupport();
	Game_Config::Init();
	Game_Config::GatherNumberOfLevels();

	Tools_Console::CreateConsole();
	Game_PreGame::ShowIntroHeader();
	Game_PreGame::SetOptions();
	Tools_Console::CloseConsole();

	GFX_Window::Init();
	HID_Keyboard::Init();
	HID_Mouse::Init();
	HID_Gamepad::Init();
	MainMenu.Init();
	Game_Transitions::Init();

	Game_Raycaster::Init();
	Game_WeaponHandling::InitConfig();
	Game_WeaponHandling::InitTextures();
	Game_WeaponHandling::InitAudio();
	Game_Effects::InitEffects();
	HUDWeaponDisplay.Init();
	HUDHealthBar.Init();
	HUDMinimap.Init();
	Game_SkyboxHandling::Init();
	Game_Doors::InitDoorAssets();
}

inline void InitAndLoadLevel()
{
	Game_Transitions::LevelTransition();
	Game_LevelHandling::InitConfig();
	Game_LevelHandling::InitMapData();
	Game_LevelHandling::InitLights();
	Game_LevelHandling::InitTextures();
	Game_LevelHandling::InitBackgroundMusic();

	Game_PathFinding::GenerateFlattenedMap(Game_PathFinding::FlattenedMap, Game_LevelHandling::LevelMapWidth, Game_LevelHandling::LevelMapHeight);

	Game_Doors::InitDoors();
	Game_SkyboxHandling::LoadSkyboxImage();
	HUDMinimap.PreRender();
	Player.InitConfig();
	Player.InitAudio();
	Game_EntityHandling::InitEntityAssets();
	Game_EntityHandling::InitEntities();
	Game_Raycaster::RefreshSettings();

	Game_EntityHandling::EntityMap[static_cast<std::int_fast32_t>(Player.Pos.X)][static_cast<std::int_fast32_t>(Player.Pos.Y)] = EntityTypes::Player;
}

inline void MovePlayerAndCheckCollision()
{
	if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][Player.FuturePos.X][static_cast<std::int_fast32_t>(Player.Pos.Y)] == 0
		&& Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][static_cast<std::int_fast32_t>(Player.Pos.X)][Player.FuturePos.Y] == 0
		&& Game_EntityHandling::EntityMap[Player.FuturePos.X][Player.FuturePos.Y] != EntityTypes::Enemy
		&& Game_EntityHandling::EntityMap[Player.FuturePos.X][Player.FuturePos.Y] != EntityTypes::Neutral
		&& Game_EntityHandling::EntityMap[Player.FuturePos.X][Player.FuturePos.Y] != EntityTypes::Turret)
	{
		Player.Pos.X += Player.StepWidth.X;
		Player.Pos.Y += Player.StepWidth.Y;

		Game_WeaponHandling::WeaponPaceFlag = !Game_WeaponHandling::WeaponPaceFlag;
		Game_WeaponHandling::WeaponPaceFlag ? Game_WeaponHandling::WeaponPace += Weapons[Player.SelectedWeapon].PaceFactor : Game_WeaponHandling::WeaponPace;
		Game_WeaponHandling::HandleAmmoBoxPickup();

		if (!Player.Sounds[static_cast<std::int_fast32_t>(Game_PlayerClass::PlayerSounds::FootSteps)].IsFinished())
		{
			Player.PlayAudio(Game_PlayerClass::PlayerSounds::FootSteps);
		}
	}
}

inline void ControlPlayerMovement()
{
	Game_EntityHandling::EntityMap[static_cast<std::int_fast32_t>(Player.Pos.X)][static_cast<std::int_fast32_t>(Player.Pos.Y)] = EntityTypes::Clear;
	Game_WeaponHandling::WeaponPaceFlag = false;

	if (GameControllerFlag && HID_Gamepad::GameController.ControllerID != -1)
	{
		switch (HID_Gamepad::GameController.RightStickPos.X)
		{
			case -1:
			{
				--HID_Mouse::MousePos.X;
				break;
			}
			case 1:
			{
				++HID_Mouse::MousePos.X;
				break;
			}
			default:
			{
				HID_Mouse::MousePos.X = 0;
			}
		}

		switch (HID_Gamepad::GameController.RightStickPos.Y)
		{
			case -1:
			{
				--HID_Mouse::MousePos.Y;
				break;
			}
			case 1:
			{
				++HID_Mouse::MousePos.Y;
				break;
			}
			default:
			{
				HID_Mouse::MousePos.Y = 0;
			}
		}
	}

	const float InputSensitivity{ GameControllerFlag ? HID_Gamepad::GameController.Sensitivity : HID_Mouse::MouseSensitivity };

	if (HID_Mouse::MousePos.X != HID_Mouse::OldMousePos.X)
	{
		const float RotationX{ GameControllerFlag ? HID_Gamepad::GameController.RotationXLimit * (HID_Gamepad::GameController.RightStick.X / InputSensitivity) : HID_Mouse::MousePos.X * InputSensitivity * lwmf::RAD2DEG };
		const float TempCos{ std::cosf(-RotationX) };
		const float TempSin{ std::sinf(-RotationX) };

		Player.Dir = { Player.Dir.X * TempCos - Player.Dir.Y * TempSin, Player.Dir.X * TempSin + Player.Dir.Y * TempCos };
		Plane = { Plane.X * TempCos - Plane.Y * TempSin, Plane.X * TempSin + Plane.Y * TempCos };
	}

	if (HID_Mouse::MousePos.Y != HID_Mouse::OldMousePos.Y)
	{
		// Check if "future" view is in given boundaries and apply if true!
		if (const float LookTemp1{ VerticalLookStep * -(HID_Mouse::MousePos.Y * InputSensitivity) }; HID_Mouse::MousePos.Y < 0 && VerticalLookCamera + LookTemp1 < VerticalLookUpLimit)
		{
			VerticalLookCamera += LookTemp1;
		}
		else if (const float LookTemp2{ VerticalLookStep * (HID_Mouse::MousePos.Y * InputSensitivity) }; HID_Mouse::MousePos.Y > 0 && -(VerticalLookCamera - LookTemp2) < VerticalLookDownLimit)
		{
			VerticalLookCamera -= LookTemp2;
		}

		VerticalLook = static_cast<std::int_fast32_t>(Canvas.Height * VerticalLookCamera);

		if ((VerticalLook & 1) != 0)
		{
			VerticalLook < 0 ? VerticalLook += -1 : VerticalLook += 1;
		}
	}

	HID_Mouse::OldMousePos = HID_Mouse::MousePos;

	if (HID_Keyboard::GetKeyState(HID_Keyboard::MovePlayerForwardKey))
	{
		Player.FuturePos = { static_cast<std::int_fast32_t>(Player.Pos.X + Player.Dir.X * Player.CollisionDetectionFactor), static_cast<std::int_fast32_t>(Player.Pos.Y + Player.Dir.Y * Player.CollisionDetectionFactor) };
		Player.StepWidth = { Player.Dir.X * Player.MoveSpeed, Player.Dir.Y * Player.MoveSpeed };
		MovePlayerAndCheckCollision();
	}
	else if (HID_Keyboard::GetKeyState(HID_Keyboard::MovePlayerBackwardKey))
	{
		Player.FuturePos = { static_cast<std::int_fast32_t>(Player.Pos.X - Player.Dir.X * Player.CollisionDetectionFactor), static_cast<std::int_fast32_t>(Player.Pos.Y - Player.Dir.Y * Player.CollisionDetectionFactor) };
		Player.StepWidth = { -(Player.Dir.X * Player.MoveSpeed), -(Player.Dir.Y * Player.MoveSpeed) };
		MovePlayerAndCheckCollision();
	}

	if (HID_Keyboard::GetKeyState(HID_Keyboard::MovePlayerStrafeRightKey))
	{
		Player.FuturePos = { static_cast<std::int_fast32_t>(Player.Pos.X + Plane.X * Player.CollisionDetectionFactor), static_cast<std::int_fast32_t>(Player.Pos.Y + Plane.Y * Player.CollisionDetectionFactor) };
		Player.StepWidth = { Plane.X * Player.MoveSpeed, Plane.Y * Player.MoveSpeed };
		MovePlayerAndCheckCollision();
	}
	else if (HID_Keyboard::GetKeyState(HID_Keyboard::MovePlayerStrafeLeftKey))
	{
		Player.FuturePos = { static_cast<std::int_fast32_t>(Player.Pos.X - Plane.X * Player.CollisionDetectionFactor), static_cast<std::int_fast32_t>(Player.Pos.Y - Plane.Y * Player.CollisionDetectionFactor) };
		Player.StepWidth = { -(Plane.X * Player.MoveSpeed), -(Plane.Y * Player.MoveSpeed) };
		MovePlayerAndCheckCollision();
	}

	Game_EntityHandling::EntityMap[static_cast<std::int_fast32_t>(Player.Pos.X)][static_cast<std::int_fast32_t>(Player.Pos.Y)] = EntityTypes::Player;
}