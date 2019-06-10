/*
***************************************************
*                                                 *
* Tools_INIFile.hpp                               *
*                                                 *
* (c) 2017, 2018, 2019 Stefan Kubsch              *
***************************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <regex>
#include <fstream>
#include <sstream>
#include <iostream>

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
		NARCLog.AddEntry("Reading value from INI file [" + Section + "] / " + Key + "...");

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
					NARCLog.AddEntry("   Value : " + std::string(Match[2]));

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
			NARCLog.LogErrorAndThrowException("Value [" + Section + "] / " + Key + " not found!");
		}

		return OutputVar;
	}

	template<typename T>void WriteValue(const std::string& Section, const std::string& Key, const T Value, const std::string& INIFileName)
	{
		if (Tools_ErrorHandling::CheckFileExistence(INIFileName, HideMessage, StopOnError))
		{
			NARCLog.AddEntry("Writing value to INI file [" + Section + "] / " + Key);

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
					Line << Key << "=" << Value << "\n";
				}

				OutputINIFile << Line;
			}

			OutputINIFile.close();
		}
	}

	inline std::int_fast32_t GetRGBColor(const std::string& INIFileName, const std::string& Section)
	{
		return lwmf::RGBAtoINT(ReadValue<std::int_fast32_t>(INIFileName, Section, "Red"), ReadValue<std::int_fast32_t>(INIFileName, Section, "Green"), ReadValue<std::int_fast32_t>(INIFileName, Section, "Blue"), 255);
	}


} // namespace Tools_INIFile