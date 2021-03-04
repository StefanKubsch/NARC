/*
****************************************************
*                                                  *
* lwmf_circles - lightweight media framework       *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <cstdint>

#include "lwmf_general.hpp"
#include "lwmf_texture.hpp"
#include "lwmf_pixel.hpp"
#include "lwmf_lines.hpp"

namespace lwmf
{


	void Circle(TextureStruct& Texture, std::int_fast32_t CenterX, std::int_fast32_t CenterY, std::int_fast32_t Radius, std::int_fast32_t Color);
	void FilledCircle(TextureStruct& Texture, std::int_fast32_t CenterX, std::int_fast32_t CenterY, std::int_fast32_t Radius, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);

	//
	// Functions
	//

	inline void Circle(TextureStruct& Texture, const std::int_fast32_t CenterX, const std::int_fast32_t CenterY, std::int_fast32_t Radius, const std::int_fast32_t Color)
	{
		// Exit early if circle would not be visible (to small or coords are out of texture boundaries)
		if (Radius <= 0 || (CenterX + Radius < 0 || CenterX - Radius > Texture.Width || CenterY + Radius < 0 || CenterY - Radius > Texture.Height))
		{
			return;
		}

		// if complete circle is within texture boundaries, there is no reason to use SetPixelSafe...
		const bool SafeFlag{ ((CenterX - Radius >= 0 && CenterX + Radius < Texture.Width) && (CenterY - Radius >= 0 && CenterY + Radius < Texture.Height)) };

		IntPointStruct Point{ -Radius, 0 };
		std::int_fast32_t Error{ 2 - (Radius << 1) };

		do
		{
			if (SafeFlag)
			{
				Texture.Pixels[((CenterY + Point.Y) * Texture.Width) + (CenterX - Point.X)] = Color;
				Texture.Pixels[((CenterY - Point.X) * Texture.Width) + (CenterX - Point.Y)] = Color;
				Texture.Pixels[((CenterY - Point.Y) * Texture.Width) + (CenterX + Point.X)] = Color;
				Texture.Pixels[((CenterY + Point.X) * Texture.Width) + (CenterX + Point.Y)] = Color;
			}
			else
			{
				SetPixelSafe(Texture, CenterX - Point.X, CenterY + Point.Y, Color);
				SetPixelSafe(Texture, CenterX - Point.Y, CenterY - Point.X, Color);
				SetPixelSafe(Texture, CenterX + Point.X, CenterY - Point.Y, Color);
				SetPixelSafe(Texture, CenterX + Point.Y, CenterY + Point.X, Color);
			}

			Radius = Error;

			if (Radius <= Point.Y)
			{
				Error += (++Point.Y << 1) + 1;
			}

			if (Radius > Point.X || Error > Point.Y)
			{
				Error += (++Point.X << 1) + 1;
			}
		} while (Point.X < 0);
	}

	inline void FilledCircle(TextureStruct& Texture, const std::int_fast32_t CenterX, const std::int_fast32_t CenterY, const std::int_fast32_t Radius, const std::int_fast32_t BorderColor, const std::int_fast32_t FillColor)
	{
		// Exit early if circle would not be visible (to small or coords out of texture boundaries)
		if (Radius <= 0 || (CenterX + Radius < 0 || CenterX - Radius > Texture.Width || CenterY + Radius < 0 || CenterY - Radius > Texture.Height))
		{
			return;
		}

		std::int_fast32_t LargestX{ Radius };
		const std::int_fast32_t POWRadius{ Radius * Radius };

		for (std::int_fast32_t y{}; y <= Radius; ++y)
		{
			const std::int_fast32_t POWY{ y * y };

			for (std::int_fast32_t x{ LargestX }; x >= 0; --x)
			{
				if ((x * x) + POWY <= POWRadius)
				{
					Line(Texture, CenterX - x, CenterY + y, CenterX + x, CenterY + y, FillColor);
					Line(Texture, CenterX - x, CenterY - y, CenterX + x, CenterY - y, FillColor);
					LargestX = x;
					break;
				}
			}
		}

		if (BorderColor != FillColor)
		{
			Circle(Texture, CenterX, CenterY, Radius, BorderColor);
		}
	}


} // namespace lwmf