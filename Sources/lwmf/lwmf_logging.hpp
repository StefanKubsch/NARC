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

#include <cstdlib>
#include <cstring>
#include <string>
#include <array>
#include <map>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>

// #define LWMF_LOGGINGENABLED in your application if you want to write any logsfiles
#ifdef LWMF_LOGGINGENABLED
	constexpr bool LoggingEnabled{ true };
#else
	constexpr bool LoggingEnabled{ false };
#endif

// #define LWMF_THROWEXCEPTIONS in your application if you want to handle errors by exceptions
#ifdef LWMF_THROWEXCEPTIONS
	constexpr bool ThrowExceptions{ true };
#else
	constexpr bool ThrowExceptions{ false };
#endif

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
		Logging(const std::string& Logfilename) noexcept;
		~Logging() noexcept;
		void AddEntry(LogLevel Level, const char* Filename, const std::string& Message);

	private:
		static std::string GetTimeStamp();

		std::ofstream Logfile;
	};

	Logging::Logging(const std::string& Logfilename) noexcept
	{
		if (LoggingEnabled)
		{
			std::ios_base::sync_with_stdio(false);

			Logfile.open(Logfilename, std::ios::out | std::ios::trunc);

			if (Logfile.fail())
			{
				std::exit(EXIT_FAILURE);
			}

			Logfile << "lwmf logging\nlogging started at: " << GetTimeStamp() << std::string(150,'-') << "\n";
		}
	}

	Logging::~Logging() noexcept
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
		if (LoggingEnabled && Logfile.is_open())
		{
			std::map<LogLevel, std::string> ErrorTable
			{
				{ LogLevel::Info, "** INFO ** " },
				{ LogLevel::Debug, "** DEBUG ** " },
				{ LogLevel::Warn, "** WARNING ** " },
				{ LogLevel::Error, "** ERROR ** " },
				{ LogLevel::Critical, "** CRITICAL ERROR ** " },
			};

			const std::map<LogLevel, std::string>::iterator ItErrorTable{ ErrorTable.find(Level) };

			if (ItErrorTable != ErrorTable.end())
			{
				if (Level == LogLevel::Error || Level == LogLevel::Critical)
				{
					Logfile << "\n" << GetTimeStamp() << ItErrorTable->second << Filename << ": " << Message << "\n";
					Logfile.close();

					ThrowExceptions ? throw std::runtime_error(Message) : std::exit(EXIT_FAILURE);
				}
				else
				{
					Logfile << ItErrorTable->second << Filename << ": " << Message << "\n";
				}
			}
		}
	}

	inline std::string Logging::GetTimeStamp()
	{
		const auto CurrentTime{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
		std::array<char, 26> TimeString{};
		ctime_s(TimeString.data(), TimeString.size(), &CurrentTime);
		return std::string(TimeString.data());
	}


} // namespace lwmf