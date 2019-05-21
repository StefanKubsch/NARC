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

#include "Tools_Console.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Tools_INIFile.hpp"

namespace Tools_InitSDL
{


	void InitSDL();

	//
	// Functions
	//

	inline void InitSDL()
	{
		Tools_Console::DisplayText(BRIGHT_MAGENTA, "\nInitializing SDL subsystems...");
		SDL_Init(SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0 ? Tools_ErrorHandling::DisplayError(fmt::format("SDL init failed: {}", SDL_GetError())) : Tools_ErrorHandling::DisplayOK();
	}


} // namespace Tools_InitSDL
