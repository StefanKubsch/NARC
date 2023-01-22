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

// Example for usage:
//
// Create
// lwmf::Logging LogName("LogFilename.log");
//
// Add entry
// LogName.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Here is out message");
//
// Will be closed on proper exit of program automatically

// This macro will return the current filename without any path information
#define __FILENAME__ (std::strrchr(__FILE__, '\\') ? std::strrchr(__FILE__, '\\') + 1 : __FILE__)

#include <cstdlib>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

// #define LWMF_LOGGINGENABLED in your application if you want to write any logsfiles
#ifdef LWMF_LOGGINGENABLED
	inline constexpr bool LoggingEnabled{ true };
#else
	inline constexpr bool LoggingEnabled{ false };
#endif

// #define LWMF_THROWEXCEPTIONS in your application if you want to handle errors by exceptions
#ifdef LWMF_THROWEXCEPTIONS
	inline constexpr bool ThrowExceptions{ true };
#else
	inline constexpr bool ThrowExceptions{ false };
#endif

namespace lwmf
{


	enum class LogLevel : std::int_fast32_t
	{
		Info,
		Trace,
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

		void AddEntry(LogLevel Level, const char* Filename, std::int_fast32_t LineNumber, std::string_view Message);

	private:
		static std::string_view GetLocalTime();

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

			Logfile << "lwmf logging\nlogging started at: " << GetLocalTime() << "\n" << std::string(180, '-') << std::endl;
		}
	}

	inline Logging::~Logging()
	{
		try
		{
			if (LoggingEnabled && Logfile.is_open())
			{
				Logfile << std::string(180, '-') << "\nlogging ended at: " << GetLocalTime() << std::endl;
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
				{ LogLevel::Info, "[INFO]" },
				{ LogLevel::Trace, "[TRACE]" },
				{ LogLevel::Debug, "[DEBUG]" },
				{ LogLevel::Warn, "[WARNING]" },
				{ LogLevel::Error, "[ERROR]" },
				{ LogLevel::Critical, "[CRITICAL ERROR]" }
			};

			const std::map<LogLevel, std::string_view>::iterator ItErrorTable{ ErrorTable.find(Level) };

			if (ItErrorTable != ErrorTable.end())
			{
				std::string MessageString{ GetLocalTime() };
				MessageString += " - ";
				MessageString += ItErrorTable->second;
				MessageString += " - ";
				MessageString += Filename;
				MessageString += "(";
				MessageString += std::to_string(LineNumber);
				MessageString += "): ";
				MessageString += Message;

				if (Level == LogLevel::Error || Level == LogLevel::Critical)
				{
					Logfile << "\n" << GetLocalTime() << "\n" << MessageString << std::endl;
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

	inline std::string_view Logging::GetLocalTime()
	{
		struct std::tm TimeObject {};
		const std::time_t CurrentTime{ std::time(nullptr) };

		localtime_s(&TimeObject, &CurrentTime);

		// Format of time following ISO 8601
		// https://de.wikipedia.org/wiki/ISO_8601

		const std::string TimeFormat{ "%Y-%m-%dT%H:%M:%S" };

		std::ostringstream ReturnString;
		ReturnString << std::put_time(&TimeObject, TimeFormat.c_str());
		return ReturnString.str();
	}


} // namespace lwmf