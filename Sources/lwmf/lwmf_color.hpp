/*
****************************************************
*                                                  *
* lwmf_color - lightweight media framework         *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <cstdint>

namespace lwmf
{


	struct ColorStruct final
	{
		std::int_fast32_t Red{};
		std::int_fast32_t Green{};
		std::int_fast32_t Blue{};
		std::int_fast32_t Alpha{};
	};

	std::int_fast32_t RGBAtoINT(std::int_fast32_t Red, std::int_fast32_t Green, std::int_fast32_t Blue, std::int_fast32_t Alpha);
	ColorStruct INTtoRGBA(std::int_fast32_t Color);

	//
	// Variables and constants
	//

	// Define little-endian bitmasks
	constexpr std::uint_fast32_t RMask{ 0x000000FF };
	constexpr std::uint_fast32_t GMask{ 0x0000FF00 };
	constexpr std::uint_fast32_t BMask{ 0x00FF0000 };
	constexpr std::uint_fast32_t AMask{ 0xFF000000 };

	//
	// Functions
	//

	inline std::int_fast32_t RGBAtoINT(const std::int_fast32_t Red, const std::int_fast32_t Green, const std::int_fast32_t Blue, const std::int_fast32_t Alpha)
	{
		return Red + (Green << 8) + (Blue << 16) + (Alpha << 24);
	}

	inline ColorStruct INTtoRGBA(const std::int_fast32_t Color)
	{
		return { static_cast<std::int_fast32_t>(Color & RMask), static_cast<std::int_fast32_t>(Color & GMask), static_cast<std::int_fast32_t>(Color & BMask), static_cast<std::int_fast32_t>(Color & AMask) };
	}


} // namespace lwmf