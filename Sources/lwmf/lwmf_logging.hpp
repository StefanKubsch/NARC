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
#include <fstream>
#include <sstream>
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
		if (LoggingEnabled)
		{
			std::ios_base::sync_with_stdio(false);

			Logfile.open(Logfilename, std::ios::out | std::ios::trunc);

			if (Logfile.fail())
			{
				std::_Exit(EXIT_FAILURE);
			}

			Logfile << "lwmf logging\nlogging started at: " << GetTimeStamp() << std::string(150,'-') << "\n";
		}
	}

	Logging::~Logging()
	{
		if (LoggingEnabled)
		{
			if (Logfile.is_open())
			{
				Logfile << std::string(150, '-') << "\nlogging ended at: " << GetTimeStamp() << "\n";
			}
		}
	}

	inline void Logging::AddEntry(const LogLevel Level, const char* Filename, const std::string& Message)
	{
		if (LoggingEnabled)
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
					Logfile << LogLevelString << std::string(Filename) << ": " << Message << "\n";
				}
				else
				{
					Logfile << "\n" << GetTimeStamp() << LogLevelString << std::string(Filename) << ": " << Message << "\n";
					Logfile.close();

					throw std::runtime_error(Message);
				}
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