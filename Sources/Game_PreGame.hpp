/*
******************************************
*                                        *
* Game_PreGame.hpp                       *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once


#include <string>
#include <fstream>
#include <iostream>

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
		if (const std::string FileName{ "./DATA/GameConfig/IntroHeader.txt" }; Tools_ErrorHandling::CheckFileExistence(FileName, ContinueOnError))
		{
			std::ifstream Reader(FileName, std::ios::in);
			std::string Line;

			while (std::getline(Reader, Line))
			{
				std::cout << Line << "\n";
			}
		}
	}

	inline void SetOptions()
	{
		std::cout << "***************\n* SET OPTIONS *\n***************\n\n";

		SelectedLevel = NumberOfLevels > 1 ? Tools_Console::QuestionForValue("Please select Level (1 - " + std::to_string(NumberOfLevels) + "): ", 1, NumberOfLevels) : 1;
		VSync = Tools_Console::QuestionForYesNo("VSync (y/n, yes implies fullscreen mode!): ") == 'y' ? true : false;
	}


} // namespace Game_PreGame