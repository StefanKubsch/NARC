/*
******************************************************
*                                                    *
* lwmf_perlinnoise - lightweight media framework     *
*                                                    *
* (C) 2019 - present by Stefan Kubsch                *
*                                                    *
******************************************************
*/

#pragma once

#include <cstdint>
#include <vector>

#include "lwmf_math.hpp"

namespace lwmf
{

	// Perlin noise, based on Ken Perlin´s famous and brilliant work
	//
	// https://cs.nyu.edu/~perlin/noise/

	class PerlinNoise
	{
	public:
		PerlinNoise();
		float Noise(float x, float y, float z);

	private:
		std::vector<std::int_fast32_t> Permutation
		{
			151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
			8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
			35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
			134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
			55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
			18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
			250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
			189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,
			43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,
			97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
			107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
			138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
		};

		float Fade(float t);
		float Gradient(std::int_fast32_t Hash, float x, float y, float z);
	};

	PerlinNoise::PerlinNoise()
	{
		Permutation.insert(Permutation.end(), Permutation.begin(), Permutation.end());
	}

	inline float PerlinNoise::Fade(const float t)
	{
		return t * t * t * (t * (t * 6.0F - 15.0F) + 10.0F);
	}

	inline float PerlinNoise::Gradient(const std::int_fast32_t Hash, const float x, const float y, const float z)
	{
		switch (Hash & 15)
		{
			case 0: return x + y;
			case 1: return -x + y;
			case 2: return x - y;
			case 3: return -x - y;
			case 4: return x + z;
			case 5: return -x + z;
			case 6: return x - z;
			case 7: return -x - z;
			case 8: return y + z;
			case 9: return -y + z; //-V1037
			case 10: return y - z;
			case 11: return -y - z; //-V1037
			case 12: return y + x;
			case 13: return -y + z;
			case 14: return y - x;
			case 15: return -y - z;
			default: return 0.0F;
		}
	}

	inline float PerlinNoise::Noise(float x, float y, float z)
	{
		const std::int_fast32_t X{ static_cast<std::int_fast32_t>(x) & 255 };
		const std::int_fast32_t Y{ static_cast<std::int_fast32_t>(y) & 255 };
		const std::int_fast32_t Z{ static_cast<std::int_fast32_t>(z) & 255 };

		x -= static_cast<std::int_fast32_t>(x);
		y -= static_cast<std::int_fast32_t>(y);
		z -= static_cast<std::int_fast32_t>(z);

		const float u{ Fade(x) };
		const float v{ Fade(y) };

		const std::int_fast32_t A{ Permutation[X] + Y };
		const std::int_fast32_t AA{ Permutation[A] + Z };
		const std::int_fast32_t AB{ Permutation[A + 1] + Z };
		const std::int_fast32_t B{ Permutation[X + 1] + Y };
		const std::int_fast32_t BA{ Permutation[B] + Z };
		const std::int_fast32_t BB{ Permutation[B + 1] + Z };

		return (Lerp<float>(Fade(z), Lerp<float>(v,
			Lerp<float>(u, Gradient(Permutation[AA], x, y, z),	Gradient(Permutation[BA], x - 1.0F, y, z)),
			Lerp<float>(u, Gradient(Permutation[AB], x, y - 1.0F, z), Gradient(Permutation[BB], x - 1.0F, y - 1.0F, z))),
			Lerp<float>(v, Lerp<float>(u, Gradient(Permutation[AA + 1], x, y, z - 1.0F), Gradient(Permutation[BA + 1], x - 1.0F, y, z - 1.0F)),
			Lerp<float>(u, Gradient(Permutation[AB + 1], x, y - 1.0F, z - 1.0F), Gradient(Permutation[BB + 1], x - 1.0F, y - 1.0F, z - 1.0F)))) + 1.0F) * 0.5F;
	}


} // namespace lwmf
