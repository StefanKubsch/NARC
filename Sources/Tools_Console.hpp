/*
******************************************
*                                        *
* Tools_Console.hpp                      *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
******************************************
*/

#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include <regex>

namespace Tools_Console
{


	void CreateConsole();
	void CloseConsole();
	void ClearInputBuffer();
	char QuestionForYesNo(const std::string& Text);
	std::int_fast32_t QuestionForValue(const std::string& Text, std::int_fast32_t BeginRange, std::int_fast32_t EndRange);

	//
	// Functions
	//

	inline void CreateConsole()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Create console...");

		AllocConsole();
		SetConsoleTitle("NARC Console");

		freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
		freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
		freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);
	}

	inline void CloseConsole()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Close console...");

		fclose(stdout);
		fclose(stdin);
		fclose(stderr);
		FreeConsole();
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
			std::cout << Text;
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
			std::cout << Text;
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