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


	std::string GetTimeStamp();
	void StartLogging(const std::string& Logfilename);
	void EndLogging();
	void AddLogEntry(const std::string& Text);
	void LogErrorAndThrowException(const std::string& ErrorMessage);

	//
	// Variables and constants
	//

	inline std::ofstream Logfile;

	//
	// Functions
	//

	inline std::string GetTimeStamp()
	{
		const auto CurrentTime{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
		char TimeString[26];
		ctime_s(TimeString, sizeof(TimeString), &CurrentTime);

		return std::string(TimeString);
	}

	inline void StartLogging(const std::string& Logfilename)
	{
		Logfile.open(Logfilename);

		if (Logfile.fail())
		{
			throw std::runtime_error("Error creating logfile!");
		}

		Logfile << "lwmf log system / (c) Stefan Kubsch" << std::endl;
		Logfile << "logging started at: " << GetTimeStamp() << std::endl;
		Logfile << "--------------------------------------------------------------------------------------------------------------" << std::endl;
	}

	inline void EndLogging()
	{
		Logfile << "--------------------------------------------------------------------------------------------------------------" << std::endl;
		Logfile << "logging ended at: " << GetTimeStamp() << std::endl;

		Logfile.close();
	}

	inline void AddLogEntry(const std::string& Text)
	{
		Logfile << "** " << Text << std::endl;
	}

	inline void LogErrorAndThrowException(const std::string& ErrorMessage)
	{
		Logfile << GetTimeStamp() << " - " << ErrorMessage << std::endl;
		Logfile.close();

		throw std::runtime_error(ErrorMessage);
	}


} // namespace lwmf