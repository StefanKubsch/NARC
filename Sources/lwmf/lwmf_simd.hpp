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

namespace lwmf
{


	void CheckForSSESupport();

	//
	// Functions
	//

	inline void CheckForSSESupport()
	{
		LWMFSystemLog.AddEntry("Checking for SSE 4.2 Extensions...");

		std::int_fast32_t CPUInfo[4];
		__cpuid(CPUInfo, 1);

		if ((CPUInfo[2] & (1 << 20)) == 0)
		{
			LWMFSystemLog.LogErrorAndThrowException("SSE 4.2 is not supported on this computer!");
		}
	}


} // namespace lwmf
