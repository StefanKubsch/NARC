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

#include <Windows.h>
#include <cstdint>

namespace lwmf
{


	//
	// Variables and constants
	//

	struct IntPointStruct final
	{
		std::int_fast32_t X{};
		std::int_fast32_t Y{};
	};

	struct FloatPointStruct final
	{
		float X{};
		float Y{};
	};

	struct IntRectStruct final
	{
		std::int_fast32_t X{};
		std::int_fast32_t Y{};
		std::int_fast32_t Width{};
		std::int_fast32_t Height{};
	};

	inline HINSTANCE WindowInstance;
	inline bool FullscreenFlag{};


} // namespace lwmf