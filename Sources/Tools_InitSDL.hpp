/*
******************************************
*                                        *
* Tools_InitSDL.hpp                      *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#define FMT_HEADER_ONLY

#include <SDL.h>
#include "fmt/format.h"

namespace Tools_InitSDL
{


	void InitSDL();

	//
	// Functions
	//

	inline void InitSDL()
	{
		lwmf::AddLogEntry("\nInitializing SDL subsystems...");

		if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0)
		{
			lwmf::LogErrorAndThrowException(fmt::format("SDL init failed: {}", SDL_GetError()));
		}
	}


} // namespace Tools_InitSDL
