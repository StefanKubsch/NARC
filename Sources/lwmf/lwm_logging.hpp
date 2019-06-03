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

namespace lwmf
{

	void LogErrorAndThrowException(const std::string& ErrorMessage);

	//
	// Variables and constants
	//

	std::string LogfileName ="lwmf_logfile.log";

	//
	// Functions
	//

	inline void LogErrorAndThrowException(const std::string& ErrorMessage)
	{
		std::ofstream Logfile;
		Logfile.open("LogfileName.txt");

		Logfile << ErrorMessage << std::endl;
		Logfile.close();

		throw std::runtime_error(ErrorMessage);
	}


} // namespace lwmf