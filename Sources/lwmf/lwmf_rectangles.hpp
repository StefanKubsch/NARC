/*
****************************************************
*                                                  *
* lwmf_rectangles - lightweight media framework    *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <cstdint>
#include <algorithm>

#include "lwmf_general.hpp"
#include "lwmf_texture.hpp"
#include "lwmf_pixel.hpp"
#include "lwmf_lines.hpp"

namespace lwmf
{

	void Rectangle(TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Color);
	void FilledRectangle(TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);

	//
	// Functions
	//

	inline void Rectangle(TextureStruct& Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, const std::int_fast32_t Color)
	{
		--Width;
		--Height;

		// Exit early if rectangle would not be visible (to small or coords are out of texture boundaries)
		if ((Width <= 0 || Height <= 0) || (PosX > Texture.Width || PosX + Width < 0 || PosY > Texture.Height || PosY + Height < 0))
		{
			return;
		}

		Line(Texture, PosX, PosY, PosX + Width, PosY, Color);
		Line(Texture, PosX, PosY, PosX, PosY + Height, Color);
		Line(Texture, PosX, PosY + Height, PosX + Width, PosY + Height, Color);
		Line(Texture, PosX + Width, PosY, PosX + Width, PosY + Height, Color);
	}

	inline void FilledRectangle(TextureStruct& Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height, const std::int_fast32_t BorderColor, const std::int_fast32_t FillColor)
	{
		// Exit early if rectangle would not be visible (to small or coords are out of texture boundaries)
		if ((Width <= 0 || Height <= 0) || (PosX > Texture.Width || PosX + Width - 1 < 0 || PosY > Texture.Height || PosY + Height - 1 < 0))
		{
			return;
		}

		// First case; Rectangle = Full Screen/Texture
		if (PosX == 0 && PosY == 0 && Width == Texture.Width && Height == Texture.Height)
		{
			ClearTexture(Texture, FillColor);

			if (BorderColor != FillColor)
			{
				Rectangle(Texture, PosX, PosY, Width, Height, BorderColor);
			}

			return;
		}

		// Second case; we can use std::fill here...
		if (PosX == 0 && PosY >= 0 && PosY < Texture.Height && Width == Texture.Width && Height <= Texture.Height)
		{
			const auto Begin{ Texture.Pixels.begin() + PosY * Texture.Width };

			std::fill(Begin, Begin + Texture.Width * Height, FillColor);

			if (BorderColor != FillColor)
			{
				Rectangle(Texture, PosX, PosY, Width, Height, BorderColor);
			}

			return;
		}

		// Third case; all the rest...
		for (std::int_fast32_t y{ PosY }; y < PosY + Height; ++y)
		{
			Line(Texture, PosX, y, PosX + Width - 1, y, FillColor);
		}

		if (BorderColor != FillColor)
		{
			Rectangle(Texture, PosX, PosY, Width, Height, BorderColor);
		}
	}


} // namespace lwmf