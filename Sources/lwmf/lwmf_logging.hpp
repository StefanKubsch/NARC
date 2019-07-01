/*
**************************************************
*                                                *
* lwmf_logging - lightweight media framework     *
*                                                *
* (C) 2019 - present by Stefan Kubsch            *
*                                                *
**************************************************
*/

#pragma once

// This macro will return the current filename without any path information
#define __FILENAME__ (std::strrchr(__FILE__, '\\') ? std::strrchr(__FILE__, '\\') + 1 : __FILE__)

#include <cstring>
#include <string>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>

namespace lwmf
{

	enum class LogLevel : std::int_fast32_t
	{
		Info,
		Debug,
		Warn,
		Error,
		Critical
	};

	class Logging final
	{
	public:
		Logging(const std::string& Logfilename);
		~Logging();
		void AddEntry(LogLevel Level, const char* Filename, const std::string& Message);

	private:
		std::string GetTimeStamp();

		std::ofstream Logfile;
	};

	Logging::Logging(const std::string& Logfilename)
	{
		Logfile.open(Logfilename);

		if (Logfile.fail())
		{
			std::_Exit(EXIT_FAILURE);
		}

		Logfile << "lwmf logging / (c) Stefan Kubsch\n";
		Logfile << "logging started at: " << GetTimeStamp();
		Logfile << "--------------------------------------------------------------------------------------------------------------" << std::endl;
	}

	Logging::~Logging()
	{
		if (Logfile.is_open())
		{
			Logfile << "--------------------------------------------------------------------------------------------------------------\n";
			Logfile << "logging ended at: " << GetTimeStamp() << std::endl;
		}
	}

	inline void Logging::AddEntry(const LogLevel Level, const char* Filename, const std::string& Message)
	{
		std::string LogLevelString;
		bool IsError{};

		switch (Level)
		{
			case LogLevel::Info:
			{
				LogLevelString = "** INFO ** ";
				break;
			}
			case LogLevel::Debug:
			{
				LogLevelString = "** DEBUG ** ";
				break;
			}
			case LogLevel::Warn:
			{
				LogLevelString = "** WARNING ** ";
				break;
			}
			case LogLevel::Error:
			{
				LogLevelString = "** ERROR ** ";
				IsError = true;
				break;
			}
			case LogLevel::Critical:
			{
				LogLevelString = "** CRITICAL ERROR ** ";
				IsError = true;
				break;
			}
			default: {}
		}

		if (Logfile.is_open())
		{
			if (!IsError)
			{
				Logfile << LogLevelString << std::string(Filename) << ": "<< Message << std::endl;
			}
			else
			{
				Logfile << "\n" << GetTimeStamp() << LogLevelString << std::string(Filename) << ": " << Message << std::endl;
				Logfile.close();

				throw std::runtime_error(Message);
			}
		}
	}

	inline std::string Logging::GetTimeStamp()
	{
		const auto CurrentTime{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
		char TimeString[26];
		ctime_s(TimeString, sizeof(TimeString), &CurrentTime);

		return std::string(TimeString);
	}


} // namespace lwmf