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
#include <intrin.h>

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
	std::int_fast32_t ShadeColor(std::int_fast32_t Color, float ShadeFactor, float Limit);
	std::int_fast32_t BlendColor(std::int_fast32_t Color1, std::int_fast32_t Color2, float Ratio);

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

	inline std::int_fast32_t ShadeColor(const std::int_fast32_t Color, const float ShadeFactor, const float Limit)
	{
		if (ShadeFactor > Limit)
		{
			return AMask;
		}

		const float Weight{ (Limit - ShadeFactor) / Limit };

		return static_cast<std::int_fast32_t>((Color & RMask) * Weight)
			| (static_cast<std::int_fast32_t>((Color & GMask) * Weight) & GMask)
			| (static_cast<std::int_fast32_t>((Color & BMask) * Weight) & BMask)
			| (static_cast<std::int_fast32_t>(Color & AMask));
	}

	inline std::int_fast32_t BlendColor(const std::int_fast32_t Color1, const std::int_fast32_t Color2, const float Ratio)
	{
		const __m128i ResultVec{ _mm_cvttps_epi32(_mm_add_ps(
			_mm_mul_ps(_mm_setr_ps(static_cast<float>(Color1 & RMask), static_cast<float>(Color1 & GMask), static_cast<float>(Color1 & BMask), 0.0F), _mm_set_ps1(1.0F - Ratio)),
			_mm_mul_ps(_mm_setr_ps(static_cast<float>(Color2 & RMask), static_cast<float>(Color2 & GMask), static_cast<float>(Color2 & BMask), 0.0F), _mm_set_ps1(Ratio)))) };

		return _mm_extract_epi32(ResultVec, 0) | (_mm_extract_epi32(ResultVec, 1) & GMask) | (_mm_extract_epi32(ResultVec, 2) & BMask) | (Color2 & AMask);
	}


} // namespace lwmf