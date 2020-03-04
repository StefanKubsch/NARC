/*
****************************************************
*                                                  *
* lwmf_simd - lightweight media framework          *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <intrin.h>
#include <cstdint>
#include <array>

#include "lwmf_logging.hpp"

namespace lwmf
{


	void CheckForSSESupport();

	//
	// Functions
	//

	inline void CheckForSSESupport()
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, "Checking for SSE 4.2 Extensions...");

		std::array<std::int_fast32_t, 4> CPUInfo{};
		__cpuid(CPUInfo.data(), 1);

		if ((CPUInfo[2] & (1 << 20)) == 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, __LINE__, "SSE 4.2 is not supported on this computer!");
		}
		else
		{
			LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, "SSE 4.2 extension found!");
		}
	}


} // namespace lwmf
