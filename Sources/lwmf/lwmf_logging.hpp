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
#include <string_view>
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
		explicit Logging(const std::string& Logfilename);
		Logging(const Logging&) = delete;
		Logging(Logging&&) = delete;
		Logging& operator = (const Logging&) = delete;
		Logging& operator = (Logging&&) = delete;
		~Logging();

		void AddEntry(LogLevel Level, const char* Filename, std::int_fast32_t LineNumber, const std::string_view Message);

	private:
		static std::string GetTimeStamp();

		std::ofstream Logfile;
	};

	inline Logging::Logging(const std::string& Logfilename)
	{
		if (LoggingEnabled)
		{
			Logfile.open(Logfilename, std::ios::out | std::ios::trunc);

			if (Logfile.fail())
			{
				std::exit(EXIT_FAILURE);
			}

			Logfile << "lwmf logging\nlogging started at: " << GetTimeStamp() << std::string(150, '-') << std::endl;
		}
	}

	inline Logging::~Logging()
	{
		try
		{
			if (LoggingEnabled && Logfile.is_open())
			{
				Logfile << std::string(150, '-') << "\nlogging ended at: " << GetTimeStamp() << std::endl;
				Logfile.close();
			}
		}
		catch (...)
		{
			// Dummy, just catch a possible exception in case something goes wrong.
		}
	}

	inline void Logging::AddEntry(const LogLevel Level, const char* Filename, const std::int_fast32_t LineNumber, const std::string_view Message)
	{
		if (LoggingEnabled && Logfile.is_open())
		{
			std::map<LogLevel, std::string_view> ErrorTable
			{
				{ LogLevel::Info, "** INFO ** " },
				{ LogLevel::Debug, "** DEBUG ** " },
				{ LogLevel::Warn, "** WARNING ** " },
				{ LogLevel::Error, "** ERROR ** " },
				{ LogLevel::Critical, "** CRITICAL ERROR ** " }
			};

			const std::map<LogLevel, std::string_view>::iterator ItErrorTable{ ErrorTable.find(Level) };

			if (ItErrorTable != ErrorTable.end())
			{
				std::string MessageString{ ItErrorTable->second };
				MessageString += Filename;
				MessageString += "(";
				MessageString += std::to_string(LineNumber);
				MessageString += "): ";
				MessageString += Message;

				if (Level == LogLevel::Error || Level == LogLevel::Critical)
				{
					Logfile << "\n" << GetTimeStamp() << MessageString << std::endl;
					Logfile.close();

					ThrowExceptions ? throw std::runtime_error(std::string(Message)) : std::exit(EXIT_FAILURE);
				}
				else
				{
					Logfile << MessageString << std::endl;
				}
			}
		}
	}

	inline std::string Logging::GetTimeStamp()
	{
		const auto CurrentTime{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
		std::array<char, 26> TimeString{};
		ctime_s(TimeString.data(), TimeString.size(), &CurrentTime);
		return { TimeString.data() };
	}


} // namespace lwmf