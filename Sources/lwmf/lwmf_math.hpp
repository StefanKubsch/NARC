/*
******************************************************
*                                                    *
* lwmf_math - lightweight media framework            *
*                                                    *
* (C) 2019 - present by Stefan Kubsch                *
*                                                    *
******************************************************
*/

#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

#include "lwmf_general.hpp"

namespace lwmf
{


	template<typename T>T Lerp(T t, T a, T b);
	template<typename T>T CalcEuclidianDistance(T x1, T x2, T y1, T y2);
	template<typename T>T CalcChebyshevDistance(T x1, T x2, T y1, T y2);
	template<typename T>T CalcChebyshevDistance(std::int_fast32_t x1, std::int_fast32_t x2, std::int_fast32_t y1, std::int_fast32_t y2);
	template<typename T>T CalcManhattanDistance(T x1, T x2, T y1, T y2);
	template<typename T>T CalcManhattanDistance(std::int_fast32_t x1, std::int_fast32_t x2, std::int_fast32_t y1, std::int_fast32_t y2);
	float FastAtan2Approx(float y, float x);
	std::uint32_t XorShift32();

	//
	// Variables and constants
	//

	constexpr float PI{ 3.14159265358979F };
	constexpr float DoublePI{ PI * 2.0F };
	constexpr float HalfPI{ PI / 2.0F };
	constexpr float OneQrtPI{ PI / 4.0F };
	constexpr float RAD2DEG{ PI / 180.0F };
	constexpr float ThreeQrtPI{ 3.0F * (PI / 4.0F) };
	inline const float SQRT1_2{ 1.0F / std::sqrtf(2.0F) };

	//
	// Functions
	//

	template<typename T>T Lerp(const T t, const T a, const T b)
	{
		return a + t * (b - a);
	}

	template<typename T>T CalcEuclidianDistance(const T x1, const T x2, const T y1, const T y2)
	{
		return std::hypot(x1 - x2, y1 - y2);
	}

	template<typename T>T CalcChebyshevDistance(T x1, T x2, T y1, T y2)
	{
		return (std::max)(std::abs(x1 - x2), std::abs(y1 - y2));
	}

	template<typename T>T CalcChebyshevDistance(const std::int_fast32_t x1, const std::int_fast32_t x2, const std::int_fast32_t y1, const std::int_fast32_t y2)
	{
		return static_cast<T>((std::max)(std::abs(x1 - x2), std::abs(y1 - y2)));
	}

	template<typename T>T CalcManhattanDistance(T x1, T x2, T y1, T y2)
	{
		return (std::abs(x1 - x2) + std::abs(y1 - y2));
	}

	template<typename T>T CalcManhattanDistance(const std::int_fast32_t x1, const std::int_fast32_t x2, const std::int_fast32_t y1, const std::int_fast32_t y2)
	{
		return static_cast<T>(std::abs(x1 - x2) + std::abs(y1 - y2));
	}

	// atan2 approximation
	// https://www.dsprelated.com/showarticle/1052.php
	// Modified and optimized in some details

	inline float FastAtan2Approx(const float y, const float x)
	{
		constexpr float n1{ 0.97239411F };
		constexpr float n2{ -0.19194795F };
		float Result{};

		if (std::fabsf(x) > FLT_EPSILON)
		{
			const union { float flVal; std::uint_fast32_t nVal; } tYSign = { y };

			if (std::fabsf(x) >= std::fabsf(y))
			{
				union { float flVal; std::uint_fast32_t nVal; } tOffset = { lwmf::PI };
				const union { float flVal; std::uint_fast32_t nVal; } tXSign = { x };
				tOffset.nVal |= tYSign.nVal & 0x80000000U;
				tOffset.nVal *= tXSign.nVal >> 31;
				Result = tOffset.flVal;
				const float z{ y / x };
				Result += (n1 + n2 * z * z) * z;
			}
			else
			{
				union { float flVal; std::uint_fast32_t nVal; } tOffset = { lwmf::HalfPI };
				tOffset.nVal |= tYSign.nVal & 0x80000000U;
				Result = tOffset.flVal;
				const float z{ x / y };
				Result -= (n1 + n2 * z * z) * z;
			}
		}
		else if (y > 0.0F)
		{
			Result = lwmf::HalfPI;
		}
		else if (y < 0.0F)
		{
			Result = -lwmf::HalfPI;
		}

		return Result;
	}

	// Simple random number generator based on XorShift
	// https://en.wikipedia.org/wiki/Xorshift

	inline std::uint32_t XorShift32()
	{
		static std::uint32_t Seed{ 7 };

		Seed ^= Seed << 13;
		Seed ^= Seed >> 17;
		return Seed ^= Seed << 5;
	}


} // namespace lwmf
