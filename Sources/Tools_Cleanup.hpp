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
#include <SDL_mixer.h>

namespace Tools_Cleanup
{


	void DestroySubsystems();

	//
	// Functions
	//

	inline void DestroySubsystems()
	{
		lwmf::UnregisterRawInputDevice(lwmf::HID_MOUSE);
		lwmf::UnregisterRawInputDevice(lwmf::HID_KEYBOARD);
		lwmf::DeleteOpenGLContext();

		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Quit SDL_Mixer");

		while (Mix_Init(0) != 0)
		{
			Mix_Quit();
		}

		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Quit SDL subsystems");
		SDL_Quit();
	}


} // namespace Tools_Cleanup