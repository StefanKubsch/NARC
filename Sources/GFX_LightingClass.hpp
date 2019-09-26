/*
***************************************************
*                                                 *
* GFX_LightingClass.hpp		                      *
*                                                 *
* (c) 2017, 2018, 2019 Stefan Kubsch              *
***************************************************
*/

#pragma once

#include <cstdint>

class GFX_LightingClass final
{
public:
	GFX_LightingClass(float PosX, float PosY, std::int_fast32_t Location, float Radius, float Intensity);
	float GetIntensity(float x, float y);

	// Location settings:
	// see LevelMapLayers in "Game_LevelHandling.hpp"
	std::int_fast32_t Location{};

private:
	lwmf::FloatPointStruct Pos{};
	float Radius{};
	float Intensity{};
};

inline GFX_LightingClass::GFX_LightingClass(const float PosX, const float PosY, const std::int_fast32_t Location, const float Radius, const float Intensity)
{
	this->Pos		= { PosX, PosY };
	this->Location	= Location;
	this->Radius	= Radius;
	this->Intensity	= Intensity;
}

inline float GFX_LightingClass::GetIntensity(const float x, const float y)
{
	const float Distance{ lwmf::CalcEuclidianDistance<float>(x, Pos.X, y, Pos.Y) };
	return Distance > Radius ? 0.0F : Intensity * ((Radius - Distance) / Radius);
}