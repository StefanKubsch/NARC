/*
******************************************************************
*                                                                *
* Tools_Cleanup.hpp                                              *
*                                                                *
* (c) 2017, 2018, 2019 Stefan Kubsch                             *
******************************************************************
*/

#pragma once

#include <SDL.h>

namespace Tools_Cleanup
{


	void DestroySubsystems();
	void CloseAllAudio();

	//
	// Functions
	//

	inline void DestroySubsystems()
	{
		lwmf::UnregisterRawInputDevice(lwmf::HID_MOUSE);
		lwmf::UnregisterRawInputDevice(lwmf::HID_KEYBOARD);
		lwmf::DeleteOpenGLContext();

		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Quit SDL subsystems");
		SDL_Quit();
	}

	inline void CloseAllAudio()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Close all audio handles...");

		Game_LevelHandling::CloseBackgroundMusic();
		Game_EntityHandling::CloseAudio();
		Game_Doors::CloseAudio();
		Game_WeaponHandling::CloseAudio();
		Player.CloseAudio();
	}


} // namespace Tools_Cleanup