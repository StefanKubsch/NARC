/*
***********************************************
*                                             *
* Tools_SIMD.hpp                              *
*                                             *
* (c) 2017, 2018, 2019 Stefan Kubsch          *
***********************************************
*/

#pragma once

#include <intrin.h>
#include <cstdint>

#include "Tools_Console.hpp"
#include "Tools_ErrorHandling.hpp"

namespace Tools_SIMD
{


	void CheckForSSESupport();

	//
	// Functions
	//

	inline void CheckForSSESupport()
	{
		Tools_Console::DisplayText(BRIGHT_MAGENTA, "Checking for SSE 4.2 Extensions...");

		std::int_fast32_t CPUInfo[4];
		__cpuid(CPUInfo, 1);

		(CPUInfo[2] & (1 << 20)) != 0 ? Tools_Console::DisplayText(BRIGHT_GREEN, "SSE 4.2 is supported\n\n") : Tools_ErrorHandling::DisplayError("SSE 4.2 is not supported on this computer!");
	}


} // namespace Tools_SIMD
