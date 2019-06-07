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

#include <string>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>

namespace lwmf
{


	class Logging final
	{
	public:
		Logging(const std::string& Logfilename);
		~Logging();

		void AddEntry(const std::string& Text);
		void LogErrorAndThrowException(const std::string& ErrorMessage);

	private:
		std::string GetTimeStamp();

		std::ofstream Logfile;
	};

	Logging::Logging(const std::string& Logfilename)
	{
		Logfile.open(Logfilename);

		if (Logfile.fail())
		{
			throw std::runtime_error("Error creating logfile!");
		}

		Logfile << "lwmf logging / (c) Stefan Kubsch\n";
		Logfile << "logging started at: " << GetTimeStamp() << "\n";
		Logfile << "--------------------------------------------------------------------------------------------------------------" << std::endl;
	}

	Logging::~Logging()
	{
		Logfile << "--------------------------------------------------------------------------------------------------------------\n";
		Logfile << "logging ended at: " << GetTimeStamp() << std::endl;

		Logfile.close();
	}

	inline void Logging::AddEntry(const std::string& Text)
	{
		Logfile << "** " << Text << std::endl;
	}

	inline void Logging::LogErrorAndThrowException(const std::string& ErrorMessage)
	{
		Logfile << GetTimeStamp() << " - " << ErrorMessage << std::endl;
		Logfile.close();

		throw std::runtime_error(ErrorMessage);
	}

	inline std::string Logging::GetTimeStamp()
	{
		const auto CurrentTime{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
		char TimeString[26];
		ctime_s(TimeString, sizeof(TimeString), &CurrentTime);

		return std::string(TimeString);
	}


} // namespace lwmf