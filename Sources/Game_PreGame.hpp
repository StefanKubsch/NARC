/*
******************************************
*                                        *
* Game_PreGame.hpp                       *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#define FMT_HEADER_ONLY

#include <string>
#include <fstream>
#include "fmt/format.h"

#include "Game_GlobalDefinitions.hpp"
#include "Tools_Console.hpp"
#include "Tools_ErrorHandling.hpp"

namespace Game_PreGame
{


	void ShowIntroHeader();
	void SetOptions();

	//
	// Functions
	//

	inline void ShowIntroHeader()
	{
		if (const std::string FileName{ "./DATA/GameConfig/IntroHeader.txt" }; Tools_ErrorHandling::CheckFileExistence(FileName, HideMessage, StopOnError))
		{
			std::ifstream Reader(FileName);
			std::string Line;

			while (std::getline(Reader, Line))
			{
				fmt::print("{}\n", Line);
			}
		}
	}

	inline void SetOptions()
	{
		fmt::print("***************\n* SET OPTIONS *\n***************\n\n");

		NumberOfLevels > 0 ? SelectedLevel = Tools_Console::QuestionForValue(fmt::format("Please select Level (0 - {}): ", NumberOfLevels), 0, NumberOfLevels) : SelectedLevel = 0;
		VSync = Tools_Console::QuestionForYesNo("VSync (y/n, yes implies fullscreen mode!): ") == 'y' ? true : false;
		fmt::print("\n");
	}


} // namespace Game_PreGame