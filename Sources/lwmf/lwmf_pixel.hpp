/*
****************************************************
*                                                  *
* lwmf_pixel - lightweight media framework         *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <cstdint>

#include "lwmf_texture.hpp"

namespace lwmf
{

	void SetPixel(TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Color);
	void SetPixelSafe(TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Color);
	std::int_fast32_t GetPixel(const TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y);
	std::int_fast32_t GetPixelSafe(const TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y);

	//
	// Functions
	//

	inline void SetPixel(TextureStruct& Texture, const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t Color)
	{
		Texture.Pixels[y * Texture.Width + x] = Color;
	}

	inline void SetPixelSafe(TextureStruct& Texture, const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t Color)
	{
		if (static_cast<std::uint_fast32_t>(x) >= static_cast<std::uint_fast32_t>(Texture.Width) || static_cast<std::uint_fast32_t>(y) >= static_cast<std::uint_fast32_t>(Texture.Height))
		{
			return;
		}

		Texture.Pixels[y * Texture.Width + x] = Color;
	}

	inline std::int_fast32_t GetPixel(const TextureStruct& Texture, const std::int_fast32_t x, const std::int_fast32_t y)
	{
		return Texture.Pixels[y * Texture.Width + x];
	}

	inline std::int_fast32_t GetPixelSafe(const TextureStruct& Texture, const std::int_fast32_t x, const std::int_fast32_t y)
	{
		if (static_cast<std::uint_fast32_t>(x) >= static_cast<std::uint_fast32_t>(Texture.Width) || static_cast<std::uint_fast32_t>(y) >= static_cast<std::uint_fast32_t>(Texture.Height))
		{
			return 0x00000000;
		}

		return Texture.Pixels[y * Texture.Width + x];
	}


} // namespace lwmf