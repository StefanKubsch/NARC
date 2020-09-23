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

	inline void Rectangle(TextureStruct& Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height, const std::int_fast32_t Color)
	{
		// Exit early if rectangle would not be visible (to small or coords are out of texture boundaries)
		if ((Width - 1 <= 0 || Height - 1 <= 0) || (PosX > Texture.Width || PosX + Width - 1 < 0 || PosY > Texture.Height || PosY + Height - 1 < 0))
		{
			return;
		}

		Line(Texture, PosX, PosY, PosX + Width - 1, PosY, Color);
		Line(Texture, PosX, PosY, PosX, PosY + Height - 1, Color);
		Line(Texture, PosX, PosY + Height - 1, PosX + Width - 1, PosY + Height - 1, Color);
		Line(Texture, PosX + Width - 1, PosY, PosX + Width - 1, PosY + Height - 1, Color);
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
		}
		// All the rest; we can use std::fill here...
		else
		{
			const std::int_fast32_t StartX{ (PosX < 0 && (std::abs(0 - PosX) < Width)) ? std::abs(0 - PosX) : 0 };
			const std::int_fast32_t TargetHeight{ (PosY + Height >= Texture.Height) ? Texture.Height - PosY : Height };
			const std::int_fast32_t TargetWidth{ (PosX + Width >= Texture.Width) ? Texture.Width - PosX : Width };

			for (std::int_fast32_t y{}, TempPosY{ PosY }; y < TargetHeight; ++y, ++TempPosY)
			{
				if (static_cast<std::uint_fast32_t>(TempPosY) < static_cast<std::uint_fast32_t>(Texture.Height))
				{
					const auto Begin{ Texture.Pixels.begin() + TempPosY * Texture.Width + PosX + StartX };
					std::fill(Begin, Begin + TargetWidth - StartX, FillColor);
				}
			}
		}

		// Draw a border if needed...
		if (BorderColor != FillColor)
		{
			Rectangle(Texture, PosX, PosY, Width, Height, BorderColor);
		}
	}


} // namespace lwmf