/*
******************************************************************
*                                                                *
* Tools_Cleanup.hpp                                              *
*                                                                *
* (c) 2017, 2018, 2019 Stefan Kubsch                             *
******************************************************************
*/

#pragma once

#include "Game_LevelHandling.hpp"
#include "Game_EntityHandling.hpp"
#include "Game_Doors.hpp"
#include "Game_WeaponHandling.hpp"

namespace Tools_Cleanup
{


	void DestroySubsystems();
	void CloseAllAudio();

	//
	// Functions
	//

	inline void DestroySubsystems()
	{
		lwmf::UnregisterRawInputDevice(lwmf::DeviceIdentifier::HID_MOUSE);
		lwmf::UnregisterRawInputDevice(lwmf::DeviceIdentifier::HID_KEYBOARD);
		lwmf::DeleteOpenGLContext();
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