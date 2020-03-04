/*
******************************************
*                                        *
* Game_PreGame.hpp                       *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
******************************************
*/

#pragma once


#include <string>
#include <cstring>
#include <array>
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
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Show intro header...");

		std::string FileName{ GameConfigFolder };
		FileName += "IntroHeader.txt";

		if (Tools_ErrorHandling::CheckFileExistence(FileName, ContinueOnError))
		{
			std::ifstream IntroHeaderFile(FileName, std::ios::in);

			std::string Line;

			while (std::getline(IntroHeaderFile, Line))
			{
				std::cout << Line << "\n";
			}
		}
	}

	inline void SetOptions()
	{
		std::cout << "***************\n* SET OPTIONS *\n***************\n\n";

		SelectedLevel = NumberOfLevels > StartLevel ? Tools_Console::QuestionForValue("Please select Level (" + std::to_string(StartLevel) + " - " + std::to_string(NumberOfLevels) + "): ", StartLevel, NumberOfLevels) : StartLevel;
		VSync = Tools_Console::QuestionForYesNo("VSync (y/n, yes implies fullscreen mode!): ") == 'y' ? true : false;
	}


} // namespace Game_PreGame