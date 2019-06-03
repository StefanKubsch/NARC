/*
******************************************
*                                        *
* Tools_Console.hpp                      *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#define FMT_HEADER_ONLY
#define FMT_STRING_ALIAS 1
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <cstdint>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include <regex>
#include "fmt/color.h"
#include "fmt/format.h"

#include "Game_GlobalDefinitions.hpp"

namespace Tools_Console
{


	void RedirectOutput();
	void ClearInputBuffer();
	char QuestionForYesNo(const std::string& Text);
	std::int_fast32_t QuestionForValue(const std::string& Text, std::int_fast32_t BeginRange, std::int_fast32_t EndRange);

	//
	// Functions
	//

	inline void RedirectOutput()
	{
		AllocConsole();
		SetConsoleTitle("NARC Console");

		DWORD ConsoleMode{};
		GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &ConsoleMode);
		SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ConsoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING);

		freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
		freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
		freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);

		// Don´t sync C and C++ standard streams since we only use C++ streams
		std::ios::sync_with_stdio(false);
		std::cin.tie(nullptr);
	}

	inline void ClearInputBuffer()
	{
		std::cin.clear();
		std::cin.ignore(std::cin.rdbuf()->in_avail(), '\n');
		std::cin.rdbuf()->in_avail();
	}

	inline char QuestionForYesNo(const std::string& Text)
	{
		char Response{ '\0' };

		while (Response != 'y' && Response != 'n')
		{
			fmt::print(Text);
			ClearInputBuffer();

			if (std::string Input; std::getline(std::cin, Input))
			{
				Response = Input.length() == 1 ? Input[0] : '\0';
			}

			Response = static_cast<char>(std::tolower(Response));
		}

		return Response;
	}

	inline std::int_fast32_t QuestionForValue(const std::string& Text, const std::int_fast32_t BeginRange, const std::int_fast32_t EndRange)
	{
		static const std::regex DigitsOnlyPattern(R"(\d+)", std::regex::optimize | std::regex::icase);

		while (true)
		{
			fmt::print(Text);
			ClearInputBuffer();

			if (std::string Input; std::getline(std::cin, Input))
			{
				if (std::regex_match(Input, DigitsOnlyPattern))
				{
					std::istringstream InputStream(Input);
					std::int_fast32_t InputValue{};
					InputStream >> InputValue;

					if (InputValue >= BeginRange && InputValue <= EndRange)
					{
						return InputValue;
					}
				}
			}
		}
	}


} // namespace Tools_Console