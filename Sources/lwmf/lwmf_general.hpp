/*
******************************************************
*                                                    *
* lwmf_general - lightweight media framework         *
*                                                    *
* (C) 2019 - present by Stefan Kubsch                *
*                                                    *
******************************************************
*/

#pragma once

#include "Windows.h"
#include <cstdint>

namespace lwmf
{


	//
	// Variables and constants
	//

	struct IntPointStruct final
	{
		std::int_fast32_t x{};
		std::int_fast32_t y{};
	};

	struct FloatPointStruct final
	{
		float x{};
		float y{};
	};

	inline std::int_fast32_t FullscreenFlag{};

	inline std::int_fast32_t ViewportWidth{};
	inline std::int_fast32_t ViewportHeight{};
	inline std::int_fast32_t ViewportWidthMid{};
	inline std::int_fast32_t ViewportHeightMid{};

	inline HDC WindowHandle;
	inline HWND MainWindow;


} // namespace lwmf