/*
******************************************
* NARC                                   *
*                                        *
* "Not Another RayCaster"                *
*                                        *
* NARC.cpp                               *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
*                                        *
******************************************
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

// Disable "unreferenced formal parameter" warning in WinMain
#pragma warning(disable: 4100)

#include <cstdint>
#include <cmath>
#include <chrono>

// ****************************
// * TECHNICAL HEADERS FIRST! *
// ****************************

// lightweight media framework
#include "lwmf/lwmf.hpp"

// Establish logging for NARC itself - system-logging for lwmf is hardcoded!
lwmf::Logging NARCLog("NARC.log");

// "ScreenTexture" is the main render target in our game!
inline lwmf::TextureStruct ScreenTexture;
inline lwmf::ShaderClass ScreenTextureShader;

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
Game_MenuClass MainMenu;
Game_HealthBarClass HUDHealthBar;
Game_MinimapClass HUDMinimap;
Game_WeaponDisplayClass HUDWeaponDisplay;

std::int_fast32_t WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
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

	lwmf::Multithreading ThreadPool;

	// Main game loop
	// fixed timestep method
	// loop until ESC is pressed

	LengthOfFrame = 1000 / FrameLock;
	std::uint_fast64_t Lag{};
	std::chrono::time_point<std::chrono::steady_clock> EndTime{ std::chrono::steady_clock::now() };

	while (!QuitGameFlag)
	{
		lwmf::CatchMouse(lwmf::MainWindow);

		std::chrono::time_point<std::chrono::steady_clock> StartTime{ std::chrono::steady_clock::now() };
		auto ElapsedTime(std::chrono::duration_cast<std::chrono::milliseconds>(StartTime - EndTime));
		EndTime = StartTime;
		Lag += static_cast<std::uint_fast64_t>(ElapsedTime.count());

		HID_Gamepad::GameController.Refresh();

		static MSG Message{};

		while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
		{
			if (Message.message == WM_QUIT)
			{
				NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "MESSAGE: WM_QUIT received...");
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
			}

			Lag -= LengthOfFrame;
		}

		Game_EntityHandling::GetEntityDistance();

		// Sort entities front to back for check if weapon hit first entity in front of player
		SortEntities(Game_EntityHandling::SortOrder::FrontToBack);
		Game_WeaponHandling::FireWeapon();

		// Sort entities back to front to draw them in right order
		SortEntities(Game_EntityHandling::SortOrder::BackToFront);

		lwmf::ClearTexture(ScreenTexture, 0);

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
			lwmf::DisplayFPSCounter(ScreenTexture, ScreenTexture.Width - 70, 7, 0xFFFFFFFF);
		}

		if (Player.IsDead && !GamePausedFlag)
		{
			Game_Transitions::DeathSequence();
		}

		lwmf::ClearBuffer();

		Game_SkyboxHandling::Render();

		if (HUDMinimap.Enabled)
		{
			HUDMinimap.Display();
		}

		ScreenTextureShader.RenderLWMFTexture(ScreenTexture);
		Game_WeaponHandling::DrawWeapon();

		if (HUDEnabled)
		{
			HUDWeaponDisplay.Display();

			if (GameControllerFlag)
			{
				HID_Gamepad::XBoxControllerIconShader.RenderStaticTexture(&HID_Gamepad::XBoxControllerIconTexture);
			}
			else
			{
				HID_Mouse::MouseIconShader.RenderStaticTexture(&HID_Mouse::MouseIconTexture);
			}
		}

		if (GamePausedFlag)
		{
			MainMenu.Show();
		}

		lwmf::SwapBuffer();
	}

	Tools_Cleanup::CloseAllAudio();
	Tools_Cleanup::DestroySubsystems();
	NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Exit program...");
	return EXIT_SUCCESS;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// Gamecontroller handling
		case WM_KEYDOWN:
		{
			if (GameControllerFlag)
			{
				if (wParam == HID_Keyboard::MovePlayerForwardKey)
				{
					HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerForwardKey, true);
				}
				else if (wParam == HID_Keyboard::MovePlayerBackwardKey)
				{
					HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerBackwardKey, true);
				}

				if (wParam == HID_Keyboard::MovePlayerStrafeLeftKey)
				{
					HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeLeftKey, true);
				}
				else if (wParam == HID_Keyboard::MovePlayerStrafeRightKey)
				{
					HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeRightKey, true);
				}

				if (wParam == HID_Gamepad::VirtMouseLeft)
				{
					HID_Gamepad::GameController.RightStickPos.X = -1;
				}
				else if (wParam == HID_Gamepad::VirtMouseRight)
				{
					HID_Gamepad::GameController.RightStickPos.X = 1;
				}

				if (wParam == HID_Gamepad::VirtMouseUp)
				{
					HID_Gamepad::GameController.RightStickPos.Y = -1;
				}
				else if (wParam == HID_Gamepad::VirtMouseDown)
				{
					HID_Gamepad::GameController.RightStickPos.Y = 1;
				}

				if (wParam == HID_Gamepad::FireSingleShotKey)
				{
					Game_WeaponHandling::InitiateSingleShot();
				}
				else if (wParam == HID_Gamepad::RapidFireKey)
				{
					Game_WeaponHandling::InitiateRapidFire();
				}

				if (wParam == HID_Gamepad::ReloadWeaponKey)
				{
					Game_WeaponHandling::InitiateReload();
				}

				if (wParam == HID_Gamepad::ActionKey)
				{
					Game_Doors::TriggerDoor();
				}

				if (wParam == HID_Gamepad::ChangeWeaponUpKey)
				{
					Game_WeaponHandling::InitiateWeaponChangeUp();
				}
				else if (wParam == HID_Gamepad::ChangeWeaponDownKey)
				{
					Game_WeaponHandling::InitiateWeaponChangeDown();
				}
			}
			break;
		}
		case WM_KEYUP:
		{
			if (GameControllerFlag)
			{
				if (wParam == HID_Keyboard::MovePlayerForwardKey)
				{
					HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerForwardKey, false);
				}

				if (wParam == HID_Keyboard::MovePlayerBackwardKey)
				{
					HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerBackwardKey, false);
				}

				if (wParam == HID_Keyboard::MovePlayerStrafeLeftKey)
				{
					HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeLeftKey, false);
				}

				if (wParam == HID_Keyboard::MovePlayerStrafeRightKey)
				{
					HID_Keyboard::SetKeyState(HID_Keyboard::MovePlayerStrafeRightKey, false);
				}

				if (wParam == HID_Gamepad::VirtMouseLeft)
				{
					HID_Gamepad::GameController.RightStickPos.X = 0;
				}

				if (wParam == HID_Gamepad::VirtMouseRight)
				{
					HID_Gamepad::GameController.RightStickPos.X = 0;
				}

				if (wParam == HID_Gamepad::VirtMouseUp)
				{
					HID_Gamepad::GameController.RightStickPos.Y = 0;
				}

				if (wParam == HID_Gamepad::VirtMouseDown)
				{
					HID_Gamepad::GameController.RightStickPos.Y = 0;
				}

				if (wParam == HID_Gamepad::RapidFireKey)
				{
					Game_WeaponHandling::ReleaseRapidFire();
				}
			}
			break;
		}
		// Mouse and keyboard handling
		case WM_INPUT:
		{
			static RAWINPUT RawDev;
			static UINT DataSize{ sizeof(RawDev) };
			static UINT HeaderSize{ sizeof(RAWINPUTHEADER) };
			HRAWINPUT Handle{ reinterpret_cast<HRAWINPUT>(lParam) };
			GetRawInputData(Handle, RID_INPUT, &RawDev, &DataSize, HeaderSize);

			switch (RawDev.header.dwType)
			{
				case RIM_TYPEMOUSE:
				{
					HID_Mouse::MousePos.X = RawDev.data.mouse.lLastX;
					HID_Mouse::MousePos.Y = RawDev.data.mouse.lLastY;

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
						if (RawDev.data.keyboard.VKey == VK_ESCAPE)
						{
							MainMenu.LevelUp();
							break;
						}

						if (GamePausedFlag)
						{
							if (RawDev.data.keyboard.VKey == VK_DOWN)
							{
								MainMenu.ItemDown();
								break;
							}

							if (RawDev.data.keyboard.VKey == VK_UP)
							{
								MainMenu.ItemUp();
								break;
							}

							if (RawDev.data.keyboard.VKey == VK_RETURN)
							{
								MainMenu.ItemSelect();
								break;
							}
						}

						if (RawDev.data.keyboard.VKey == VK_SPACE)
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
							if (NumberOfLevels > 0)
							{
								SelectedLevel < NumberOfLevels ? ++SelectedLevel : SelectedLevel = 0;
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
		case MM_MCINOTIFY:
		{
			switch (wParam)
			{
				case MCI_NOTIFY_SUCCESSFUL:
				{
					// "rewind" player footsteps audio if played completely
					if (lParam == static_cast<LPARAM>(Player.Sounds[static_cast<std::int_fast32_t>(Game_PlayerClass::PlayerSounds::FootSteps)].GetDeviceID()))
					{
						Player.Sounds[static_cast<std::int_fast32_t>(Game_PlayerClass::PlayerSounds::FootSteps)].RewindToStart();
					}

					// "rewind" and restart background music
					if (Game_LevelHandling::BackgroundMusicEnabled && lParam == static_cast<LPARAM>(Game_LevelHandling::BackgroundMusic.GetDeviceID()))
					{
						Game_LevelHandling::BackgroundMusic.RewindToStart();
						Game_LevelHandling::BackgroundMusic.Play(lwmf::MP3::PlayModes::NOTIFY);
					}
					break;
				}
				default: {}
			}
			break;
		}
		default: {}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void InitAndLoadGameConfig()
{
	Tools_Console::CreateConsole();
	Game_Config::GatherNumberOfLevels();
	Game_PreGame::ShowIntroHeader();
	Game_PreGame::SetOptions();
	lwmf::CheckForSSESupport();
	Game_Config::Init();
	Game_Raycaster::Init();
	GFX_Window::Init();
	Game_WeaponHandling::InitConfig();
	Game_WeaponHandling::InitTextures();
	Game_WeaponHandling::InitAudio();
	HUDWeaponDisplay.Init();
	HUDHealthBar.Init();
	HUDMinimap.Init();
	Game_SkyboxHandling::Init();
	Game_Doors::InitDoorAssets();
	HID_Keyboard::Init();
	HID_Mouse::Init();
	HID_Gamepad::Init();
	Game_Transitions::Init();
	MainMenu.Init();
	Tools_Console::CloseConsole();
}

void InitAndLoadLevel()
{
	Game_Transitions::LevelTransition();

	Game_LevelHandling::CloseBackgroundMusic();
	Game_EntityHandling::CloseAudio();
	Player.CloseAudio();

	Game_LevelHandling::InitConfig();
	Game_LevelHandling::InitMapData();
	Game_LevelHandling::InitLights();
	Game_LevelHandling::InitTextures();
	Game_LevelHandling::InitBackgroundMusic();

	Game_PathFinding::GenerateFlattenedMap(Game_PathFinding::FlattenedMap, Game_LevelHandling::LevelMapWidth, Game_LevelHandling::LevelMapHeight);

	Game_Doors::InitDoors();
	Game_SkyboxHandling::ClearSkyBox();
	Game_SkyboxHandling::LoadSkyboxImage();
	HUDMinimap.PreRender();
	Player.InitConfig();
	Player.InitAudio();
	Game_EntityHandling::InitEntityAssets();
	Game_EntityHandling::InitEntities();

	Game_LevelHandling::PlayBackgroundMusic();
	Game_Raycaster::RefreshSettings();

	Game_EntityHandling::EntityMap[static_cast<std::int_fast32_t>(Player.Pos.X)][static_cast<std::int_fast32_t>(Player.Pos.Y)] = Game_EntityHandling::EntityTypes::Player;
}

void MovePlayerAndCheckCollision()
{
	if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][Player.FuturePos.X][Player.FuturePos.Y] != 0
		|| Game_EntityHandling::EntityMap[Player.FuturePos.X][Player.FuturePos.Y] == Game_EntityHandling::EntityTypes::Enemy
		|| Game_EntityHandling::EntityMap[Player.FuturePos.X][Player.FuturePos.Y] == Game_EntityHandling::EntityTypes::Neutral
		|| Game_EntityHandling::EntityMap[Player.FuturePos.X][Player.FuturePos.Y] == Game_EntityHandling::EntityTypes::Turret)
	{
		Player.Pos.X += Player.StepWidth.X;
		Player.Pos.Y += Player.StepWidth.Y;
	}
	else
	{
		Game_WeaponHandling::WeaponPaceFlag = !Game_WeaponHandling::WeaponPaceFlag;
		Game_WeaponHandling::WeaponPaceFlag ? Game_WeaponHandling::WeaponPace += Weapons[Player.SelectedWeapon].PaceFactor : Game_WeaponHandling::WeaponPace;
		Game_WeaponHandling::HandleAmmoBoxPickup();
		Player.PlayAudio(Game_PlayerClass::PlayerSounds::FootSteps);
	}
}

void ControlPlayerMovement()
{
	Game_EntityHandling::EntityMap[static_cast<std::int_fast32_t>(Player.Pos.X)][static_cast<std::int_fast32_t>(Player.Pos.Y)] = Game_EntityHandling::EntityTypes::Clear;
	Game_WeaponHandling::WeaponPaceFlag = false;

	if (GameControllerFlag)
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
		const float RotationX{ GameControllerFlag ? HID_Gamepad::GameController.RotationXLimit * (HID_Gamepad::GameController.RightStick.X / InputSensitivity) : HID_Mouse::MousePos.X * InputSensitivity * (lwmf::PI / 180.0F) };
		const float oldDirX{ Player.Dir.X };
		const float TmpCos{ std::cosf(-RotationX) };
		const float TmpSin{ std::sinf(-RotationX) };

		Player.Dir.X = Player.Dir.X * TmpCos - Player.Dir.Y * TmpSin;
		Player.Dir.Y = oldDirX * TmpSin + Player.Dir.Y * TmpCos;
		const float oldPlaneX{ Plane.X };
		Plane.X = Plane.X * TmpCos - Plane.Y * TmpSin;
		Plane.Y = oldPlaneX * TmpSin + Plane.Y * TmpCos;
	}

	if (HID_Mouse::MousePos.Y != HID_Mouse::OldMousePos.Y)
	{
		// Check if "future" view is in given boundaries and apply if true!
		if (HID_Mouse::MousePos.Y < 0 && VerticalLookCamera + VerticalLookStep * -(HID_Mouse::MousePos.Y * InputSensitivity) < VerticalLookUpLimit)
		{
			VerticalLookCamera += VerticalLookStep * -(HID_Mouse::MousePos.Y * InputSensitivity);
		}
		else if (HID_Mouse::MousePos.Y > 0 && -(VerticalLookCamera - VerticalLookStep * (HID_Mouse::MousePos.Y * InputSensitivity)) < VerticalLookDownLimit)
		{
			VerticalLookCamera -= VerticalLookStep * (HID_Mouse::MousePos.Y * InputSensitivity);
		}

		VerticalLook = static_cast<std::int_fast32_t>(ScreenTexture.Height * VerticalLookCamera);

		if ((VerticalLook & 1) != 0)
		{
			VerticalLook < 0 ? VerticalLook += -1 : VerticalLook += 1;
		}
	}

	HID_Mouse::OldMousePos = HID_Mouse::MousePos;

	if (HID_Keyboard::GetKeyState(HID_Keyboard::MovePlayerForwardKey))
	{
		const lwmf::FloatPointStruct StepTemp{ Player.Dir.X * Player.MoveSpeed, Player.Dir.Y * Player.MoveSpeed };

		Player.Pos.X += StepTemp.X;
		Player.Pos.Y += StepTemp.Y;
		Player.FuturePos.X = static_cast<std::int_fast32_t>(Player.Pos.X + Player.Dir.X * Player.CollisionDetectionFactor);
		Player.FuturePos.Y = static_cast<std::int_fast32_t>(Player.Pos.Y + Player.Dir.Y * Player.CollisionDetectionFactor);
		Player.StepWidth.X = -StepTemp.X;
		Player.StepWidth.Y = -StepTemp.Y;

		MovePlayerAndCheckCollision();
	}
	else if (HID_Keyboard::GetKeyState(HID_Keyboard::MovePlayerBackwardKey))
	{
		const lwmf::FloatPointStruct StepTemp{ Player.Dir.X * Player.MoveSpeed, Player.Dir.Y * Player.MoveSpeed };

		Player.Pos.X -= StepTemp.X;
		Player.Pos.Y -= StepTemp.Y;
		Player.FuturePos.X = static_cast<std::int_fast32_t>(Player.Pos.X - Player.Dir.X * Player.CollisionDetectionFactor);
		Player.FuturePos.Y = static_cast<std::int_fast32_t>(Player.Pos.Y - Player.Dir.Y * Player.CollisionDetectionFactor);
		Player.StepWidth = StepTemp;

		MovePlayerAndCheckCollision();
	}

	if (HID_Keyboard::GetKeyState(HID_Keyboard::MovePlayerStrafeRightKey))
	{
		const lwmf::FloatPointStruct StepTemp{ Plane.X * Player.MoveSpeed, Plane.Y * Player.MoveSpeed };

		Player.Pos.X += StepTemp.X;
		Player.Pos.Y += StepTemp.Y;
		Player.FuturePos.X = static_cast<std::int_fast32_t>(Player.Pos.X + Plane.X * Player.CollisionDetectionFactor);
		Player.FuturePos.Y = static_cast<std::int_fast32_t>(Player.Pos.Y + Plane.Y * Player.CollisionDetectionFactor);
		Player.StepWidth.X = -StepTemp.X;
		Player.StepWidth.Y = -StepTemp.Y;

		MovePlayerAndCheckCollision();
	}
	else if (HID_Keyboard::GetKeyState(HID_Keyboard::MovePlayerStrafeLeftKey))
	{
		const lwmf::FloatPointStruct StepTemp{ Plane.X * Player.MoveSpeed, Plane.Y * Player.MoveSpeed };

		Player.Pos.X -= StepTemp.X;
		Player.Pos.Y -= StepTemp.Y;
		Player.FuturePos.X = static_cast<std::int_fast32_t>(Player.Pos.X - Plane.X * Player.CollisionDetectionFactor);
		Player.FuturePos.Y = static_cast<std::int_fast32_t>(Player.Pos.Y - Plane.Y * Player.CollisionDetectionFactor);
		Player.StepWidth = StepTemp;

		MovePlayerAndCheckCollision();
	}

	Game_EntityHandling::EntityMap[static_cast<std::int_fast32_t>(Player.Pos.X)][static_cast<std::int_fast32_t>(Player.Pos.Y)] = Game_EntityHandling::EntityTypes::Player;
}