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

		while (Mix_Init(0) != 0)
		{
			Mix_Quit();
		}

		SDL_Quit();
	}


} // namespace Tools_Cleanup