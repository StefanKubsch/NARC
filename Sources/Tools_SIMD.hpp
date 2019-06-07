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

namespace Tools_SIMD
{


	void CheckForSSESupport();

	//
	// Functions
	//

	inline void CheckForSSESupport()
	{
		NARCLog.AddEntry("Checking for SSE 4.2 Extensions...");

		std::int_fast32_t CPUInfo[4];
		__cpuid(CPUInfo, 1);

		if ((CPUInfo[2] & (1 << 20)) == 0)
		{
			NARCLog.LogErrorAndThrowException("SSE 4.2 is not supported on this computer!");
		}
	}


} // namespace Tools_SIMD
