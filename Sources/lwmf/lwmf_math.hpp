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
	template<>float CalcEuclidianDistance(float x1, float x2, float y1, float y2);
	template<typename T>T CalcChebyshevDistance(T x1, T x2, T y1, T y2);
	template<typename T>T CalcChebyshevDistance(std::int_fast32_t x1, std::int_fast32_t x2, std::int_fast32_t y1, std::int_fast32_t y2);
	template<typename T>T CalcManhattanDistance(T x1, T x2, T y1, T y2);
	template<typename T>T CalcManhattanDistance(std::int_fast32_t x1, std::int_fast32_t x2, std::int_fast32_t y1, std::int_fast32_t y2);

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

	template<>inline float CalcEuclidianDistance(const float x1, const float x2, const float y1, const float y2)
	{
		return std::hypotf(x1 - x2, y1 - y2);
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


} // namespace lwmf
