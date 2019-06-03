/*
***************************************************
*                                                 *
* Tools_INIFile.hpp                               *
*                                                 *
* (c) 2017, 2018, 2019 Stefan Kubsch              *
***************************************************
*/

#pragma once

#define FMT_HEADER_ONLY

#include <cstdint>
#include <string>
#include <regex>
#include <fstream>
#include <sstream>
#include <iostream>
#include "fmt/format.h"

namespace Tools_INIFile
{


	template<typename T>T ReadValue(const std::string& INIFileName, const std::string& Section, const std::string& Key);
	template<typename T>void WriteValue(const std::string& Section, const std::string& Key, T Value, const std::string& INIFileName);
	std::int_fast32_t GetRGBColor(const std::string& INIFileName, const std::string& Section);

	//
	// Functions
	//

	template<typename T>T ReadValue(const std::string& INIFileName, const std::string& Section, const std::string& Key)
	{
		lwmf::AddLogEntry(fmt::format("Reading value from INI file [{0}] / {1} ...", Section, Key));

		static const std::regex SectionTest(R"(\[(.*?)\])", std::regex::optimize | std::regex::icase);
		static const std::regex ValueTest(R"((\w+)=([^\#]+(?!\+{3})))", std::regex::optimize | std::regex::icase);

		T OutputVar{};
		std::ifstream INIFile(INIFileName);
		std::smatch Match;
		std::string CurrentSection;
		std::string Line;
		bool ValueFound{};

		while (std::getline(INIFile, Line))
		{
			// Remove all spaces from line
			Line.erase(std::remove(Line.begin(), Line.end(), ' '), Line.end());

			if (Line.length() > 0)
			{
				if (std::regex_search(Line, Match, SectionTest))
				{
					CurrentSection = Match[1];
				}
				else if (std::regex_search(Line, Match, ValueTest) && (CurrentSection == Section && Match[1] == Key))
				{
					lwmf::AddLogEntry("Value : " + std::string(Match[2]));

					// Convert Value to proper type
					std::istringstream Stream(Match[2]);
					std::string(typeid(T).name()) == "bool" ? Stream >> std::boolalpha >> OutputVar : Stream >> OutputVar;
					ValueFound = true;
					break;
				}
			}
		}

		if (!ValueFound)
		{
			lwmf::LogErrorAndThrowException(fmt::format("Value [{0}] / {1} not found!", Section, Key));
		}

		return OutputVar;
	}

	template<typename T>void WriteValue(const std::string& Section, const std::string& Key, const T Value, const std::string& INIFileName)
	{
		if (Tools_ErrorHandling::CheckFileExistence(INIFileName, HideMessage, StopOnError))
		{
			if (Debug)
			{
				lwmf::AddLogEntry(fmt::format("Writing value to INI file [{0}] / {1} : {2}", Section, Key, Value));
			}

			// Read INI file into vector of strings

			std::vector<std::string> VectorOfStrings;
			std::ifstream InputINIFile(INIFileName);
			std::string InputLine;

			while (std::getline(InputINIFile, InputLine))
			{
				VectorOfStrings.emplace_back(InputLine);
			}

			InputINIFile.close();

			// Modify proper line and write back INI file

			static const std::regex SectionTest(R"(\[(.*?)\])", std::regex::optimize | std::regex::icase);
			static const std::regex ValueTest(R"((\w+)=([^\#]+(?!\+{3})))", std::regex::optimize | std::regex::icase);

			std::string CurrentSection;
			std::ofstream OutputINIFile(INIFileName);
			std::smatch Match;

			for (auto&& Line : VectorOfStrings)
			{
				// Remove all spaces from line
				Line.erase(std::remove(Line.begin(), Line.end(), ' '), Line.end());

				if (std::regex_search(Line, Match, SectionTest))
				{
					CurrentSection = Match[1];
				}
				else if (std::regex_search(Line, Match, ValueTest) && (CurrentSection == Section && Match[1] == Key))
				{
					Line = fmt::format("{0}={1}", Key, Value);
				}

				OutputINIFile << fmt::format("{}\n", Line);
			}

			OutputINIFile.close();
		}
	}

	inline std::int_fast32_t GetRGBColor(const std::string& INIFileName, const std::string& Section)
	{
		return lwmf::RGBAtoINT(ReadValue<std::int_fast32_t>(INIFileName, Section, "Red"), ReadValue<std::int_fast32_t>(INIFileName, Section, "Green"), ReadValue<std::int_fast32_t>(INIFileName, Section, "Blue"), 255);
	}


} // namespace Tools_INIFile