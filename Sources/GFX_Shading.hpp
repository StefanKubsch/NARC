/*
***********************************************
*                                             *
* GFX_Shading.hpp                             *
*                                             *
* (c) 2017, 2018, 2019 Stefan Kubsch          *
***********************************************
*/

#pragma once

#include <intrin.h>
#include <cstdint>

namespace GFX_Shading
{


	std::int_fast32_t ShadeColor(std::int_fast32_t Color, float ShadeFactor, float Limit);
	std::int_fast32_t BlendColor(std::int_fast32_t Color1, std::int_fast32_t Color2, float Ratio);

	//
	// Functions
	//

	inline std::int_fast32_t ShadeColor(const std::int_fast32_t Color, const float ShadeFactor, const float Limit)
	{
		if (ShadeFactor > Limit)
		{
			return lwmf::AMask;
		}

		const float Weight{ (Limit - ShadeFactor) / Limit };

		return static_cast<std::int_fast32_t>((Color & lwmf::RMask) * Weight)
			| (static_cast<std::int_fast32_t>((Color & lwmf::GMask) * Weight) & lwmf::GMask)
			| (static_cast<std::int_fast32_t>((Color & lwmf::BMask) * Weight) & lwmf::BMask)
			| (static_cast<std::int_fast32_t>(Color & lwmf::AMask));
	}

	inline std::int_fast32_t BlendColor(const std::int_fast32_t Color1, const std::int_fast32_t Color2, const float Ratio)
	{
		const __m128i ResultVec{ _mm_cvttps_epi32(_mm_add_ps(
			_mm_mul_ps(_mm_setr_ps(static_cast<float>(Color1 & lwmf::RMask), static_cast<float>(Color1 & lwmf::GMask), static_cast<float>(Color1 & lwmf::BMask), 0.0F), _mm_set_ps1(1.0F - Ratio)),
			_mm_mul_ps(_mm_setr_ps(static_cast<float>(Color2 & lwmf::RMask), static_cast<float>(Color2 & lwmf::GMask), static_cast<float>(Color2 & lwmf::BMask), 0.0F), _mm_set_ps1(Ratio)))) };

		return _mm_extract_epi32(ResultVec, 0) | (_mm_extract_epi32(ResultVec, 1) & lwmf::GMask) | (_mm_extract_epi32(ResultVec, 2) & lwmf::BMask) | (Color2 & lwmf::AMask);
	}


} // namespace GFX_Shading
