/*
****************************************************
*                                                  *
* lwmf_lines - lightweight media framework         *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <cstdint>
#include <algorithm>

#include "lwmf_general.hpp"
#include "lwmf_color.hpp"
#include "lwmf_texture.hpp"
#include "lwmf_pixel.hpp"

namespace lwmf
{


	std::int_fast32_t FindRegion(std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t x, std::int_fast32_t y);
	bool ClipLine(std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t x1, std::int_fast32_t y1, std::int_fast32_t x2, std::int_fast32_t y2, std::int_fast32_t& x3, std::int_fast32_t& y3, std::int_fast32_t& x4, std::int_fast32_t& y4);
	void Line(TextureStruct& Texture, std::int_fast32_t x1, std::int_fast32_t y1, std::int_fast32_t x2, std::int_fast32_t y2, std::int_fast32_t Color);
	void DrawPixelAA(TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Color, float Brightness);
	float FracPart(float x);
	void LineAA(TextureStruct& Texture, std::int_fast32_t x1, std::int_fast32_t y1, std::int_fast32_t x2, std::int_fast32_t y2, std::int_fast32_t Color);

	//
	// Functions
	//

	// FindRegion and ClipLine provide the "Cohen Sutherland Clipping" algorithm for lines
	// https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
	//
	// The algorithm is also described in "Computer Graphics - Principles and Practice, Second Edition in C" by Foley/van Dam/Feiner/Hughs
	// Chapter 3.12.3, page 113ff

	inline std::int_fast32_t FindRegion(const std::int_fast32_t Width, const std::int_fast32_t Height, const std::int_fast32_t x, const std::int_fast32_t y)
	{
		int_fast32_t Code{};

		if (y >= Height)
		{
			Code |= 1;
		}
		else if (y < 0)
		{
			Code |= 2;
		}

		if (x >= Width)
		{
			Code |= 4;
		}
		else if (x < 0)
		{
			Code |= 8;
		}

		return Code;
	}

	inline bool ClipLine(const std::int_fast32_t Width, const std::int_fast32_t Height, std::int_fast32_t x1, std::int_fast32_t y1, std::int_fast32_t x2, std::int_fast32_t y2, std::int_fast32_t& x3, std::int_fast32_t& y3, std::int_fast32_t& x4, std::int_fast32_t& y4)
	{
		bool Accept{};
		bool Done{};
		std::int_fast32_t Code1{ FindRegion(Width, Height, x1, y1) };
		std::int_fast32_t Code2{ FindRegion(Width, Height, x2, y2) };

		do
		{
			if ((Code1 | Code2) == 0)
			{
				Accept = true;
				Done = true;
			}
			else if ((Code1 & Code2) != 0)
			{
				Done = true;
			}
			else
			{
				IntPointStruct Point{};
				const std::int_fast32_t Codeout{ Code1 != 0 ? Code1 : Code2 };

				if ((Codeout & 1) != 0)
				{
					Point = { x1 + (x2 - x1) * (Height - y1) / (y2 - y1), Height - 1 };
				}
				else if ((Codeout & 2) != 0)
				{
					Point.X = x1 + (x2 - x1) * -y1 / (y2 - y1);
				}
				else if ((Codeout & 4) != 0)
				{
					Point = { Width - 1, y1 + (y2 - y1) * (Width - x1) / (x2 - x1) };
				}
				else
				{
					Point.Y = y1 + (y2 - y1) * -x1 / (x2 - x1);
				}

				if (Codeout == Code1)
				{
					x1 = Point.X;
					y1 = Point.Y;
					Code1 = FindRegion(Width, Height, x1, y1);
				}
				else
				{
					x2 = Point.X;
					y2 = Point.Y;
					Code2 = FindRegion(Width, Height, x2, y2);
				}
			}
		} while (!Done);

		if (Accept)
		{
			x3 = x1;
			x4 = x2;
			y3 = y1;
			y4 = y2;

			return true;
		}

		return false;
	}

	inline void Line(TextureStruct& Texture, std::int_fast32_t x1, std::int_fast32_t y1, std::int_fast32_t x2, std::int_fast32_t y2, const std::int_fast32_t Color)
	{
		// Exit early if coords are completely out of texture boundaries
		if ((x1 < 0 && x2 < 0) || (x1 > Texture.Width && x2 > Texture.Width) || (y1 < 0 && y2 < 0) || (y1 > Texture.Height && y2 > Texture.Height))
		{
			return;
		}

		// Case 1: Straight horizontal line within texture boundaries
		if (y1 == y2
			&& static_cast<std::uint_fast32_t>(y1) < static_cast<std::uint_fast32_t>(Texture.Height)
			&& static_cast<std::uint_fast32_t>(x1) < static_cast<std::uint_fast32_t>(Texture.Width)
			&& static_cast<std::uint_fast32_t>(x2) < static_cast<std::uint_fast32_t>(Texture.Width))
		{
			if (x1 > x2)
			{
				std::swap(x1, x2);
			}

			const auto Begin{ Texture.Pixels.begin() + y1 * Texture.Width };
			std::fill(Begin + x1, Begin + x2 + 1, Color);

			return;
		}

		// Case 2: Straight vertical line within texture boundaries
		if (x1 == x2
			&& static_cast<std::uint_fast32_t>(x1) < static_cast<std::uint_fast32_t>(Texture.Width)
			&& static_cast<std::uint_fast32_t>(y1) < static_cast<std::uint_fast32_t>(Texture.Height)
			&& static_cast<std::uint_fast32_t>(y2) < static_cast<std::uint_fast32_t>(Texture.Height))
		{
			if (y1 > y2)
			{
				std::swap(y1, y2);
			}

			for (std::int_fast32_t y{ y1 }; y <= y2; ++y)
			{
				Texture.Pixels[y * Texture.Width + x1] = Color;
			}

			return;
		}

		// Case 3: All other line variants

		// I use "EFLA" - "Extremely Fast Line Algorithm" variant E
		// Copyright 2001-2, By Po-Han Lin
		// http://www.edepot.com/algorithm.html
		//
		// I modified the algorithm slightly...

		// Clip line coordinates to fit into texture boundaries
		if (!ClipLine(Texture.Width, Texture.Height, x1, y1, x2, y2, x1, y1, x2, y2))
		{
			return;
		}

		std::int_fast32_t ShortLength{ y2 - y1 };
		std::int_fast32_t LongLength{ x2 - x1 };

		const bool Steep{ std::abs(ShortLength) > std::abs(LongLength) };

		if (Steep)
		{
			std::swap(ShortLength, LongLength);
		}

		const std::int_fast32_t DecInc{ LongLength == 0 ? 0 : (ShortLength << 16) / LongLength };

		if (Steep)
		{
			const std::int_fast32_t StartY{ 0x8000 + (x1 << 16) };

			if (LongLength > 0)
			{
				LongLength += y1;

				for (std::int_fast32_t j{ StartY }; y1 <= LongLength; ++y1)
				{
					Texture.Pixels[y1 * Texture.Width + (j >> 16)] = Color;
					j += DecInc;
				}

				return;
			}

			LongLength += y1;

			for (std::int_fast32_t j{ StartY }; y1 >= LongLength; --y1)
			{
				Texture.Pixels[y1 * Texture.Width + (j >> 16)] = Color;
				j -= DecInc;
			}

			return;
		}

		const std::int_fast32_t StartX{ 0x8000 + (y1 << 16) };

		if (LongLength > 0)
		{
			LongLength += x1;

			for (std::int_fast32_t j{ StartX }; x1 <= LongLength; ++x1)
			{
				Texture.Pixels[(j >> 16) * Texture.Width + x1] = Color;
				j += DecInc;
			}

			return;
		}

		LongLength += x1;

		for (std::int_fast32_t j{ StartX }; x1 >= LongLength; --x1)
		{
			Texture.Pixels[(j >> 16) * Texture.Width + x1] = Color;
			j -= DecInc;
		}
	}

	//
	// Anti-aliased Line
	//

	// This is my implementation of Xiaolin Wu´s line algorithm
	// https://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm

	inline void DrawPixelAA(TextureStruct& Texture, const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t Color, const float Brightness)
	{
		const ColorStruct ModColor{ INTtoRGBA(Color) };

		SetPixelSafe(Texture, x, y, RGBAtoINT(static_cast<std::int_fast32_t>(static_cast<float>(ModColor.Red) * Brightness),
			static_cast<std::int_fast32_t>(static_cast<float>(ModColor.Green) * Brightness),
			static_cast<std::int_fast32_t>(static_cast<float>(ModColor.Blue) * Brightness),
			ModColor.Alpha));
	}

	inline float FracPart(const float x)
	{
		const float n{ std::fabs(x) };
		return n - std::floorf(n);
	}

	inline void LineAA(TextureStruct& Texture, std::int_fast32_t x1, std::int_fast32_t y1, std::int_fast32_t x2, std::int_fast32_t y2, const std::int_fast32_t Color)
	{
		// Exit early if coords are completely out of texture boundaries
		if ((x1 < 0 && x2 < 0) || (x1 > Texture.Width && x2 > Texture.Width) || (y1 < 0 && y2 < 0) || (y1 > Texture.Height && y2 > Texture.Height))
		{
			return;
		}

		// Clip line coordinates to fit into texture boundaries
		if (!ClipLine(Texture.Width, Texture.Height, x1, y1, x2, y2, x1, y1, x2, y2))
		{
			return;
		}

		const bool Steep{ std::abs(y2 - y1) > std::abs(x2 - x1) };

		if (Steep)
		{
			std::swap(x1, y1);
			std::swap(x2, y2);
		}

		if (x1 > x2)
		{
			std::swap(x1, x2);
			std::swap(y1, y2);
		}

		const float Gradient{ static_cast<float>((y2 - y1)) / static_cast<float>((x2 - x1)) };
		float EndY{ static_cast<float>(y1) + Gradient };
		float GapX{ 1.0F - FracPart(static_cast<float>(x1) + 0.5F) };
		const std::int_fast32_t ypxl1{ static_cast<std::int_fast32_t>(std::nearbyintf(EndY)) };
		float FracEndY{ FracPart(EndY) };

		if (Steep)
		{
			DrawPixelAA(Texture, ypxl1, x1, Color, 1.0F - FracEndY * GapX);
			DrawPixelAA(Texture, ypxl1 + 1, x1, Color, FracEndY * GapX);
		}
		else
		{
			DrawPixelAA(Texture, x1, ypxl1, Color, 1.0F - FracEndY * GapX);
			DrawPixelAA(Texture, x1, ypxl1 + 1, Color, FracEndY * GapX);
		}

		float Intersection{ EndY + Gradient };
		EndY = static_cast<float>(y2) + Gradient;
		GapX = FracPart(static_cast<float>(x2) + 0.5F);
		const std::int_fast32_t ypxl2{ static_cast<std::int_fast32_t>(std::nearbyintf(EndY)) };
		FracEndY = FracPart(EndY);

		if (Steep)
		{
			DrawPixelAA(Texture, ypxl2, x2, Color, 1.0F - FracEndY * GapX);
			DrawPixelAA(Texture, ypxl2 + 1, x2, Color, FracEndY * GapX);
		}
		else
		{
			DrawPixelAA(Texture, x2, ypxl2, Color, 1.0F - FracEndY * GapX);
			DrawPixelAA(Texture, x2, ypxl2 + 1, Color, FracEndY * GapX);
		}

		for (std::int_fast32_t x{ x1 + 1 }; x < x2; ++x)
		{
			const std::int_fast32_t Integral{ static_cast<std::int_fast32_t>(std::nearbyintf(Intersection)) };
			const float FracInt{ FracPart(Intersection) };

			if (Steep)
			{
				DrawPixelAA(Texture, Integral, x, Color, 1.0F - FracInt);
				DrawPixelAA(Texture, Integral + 1, x, Color, FracInt);
			}
			else
			{
				DrawPixelAA(Texture, x, Integral, Color, 1.0F - FracInt);
				DrawPixelAA(Texture, x, Integral + 1, Color, FracInt);
			}

			Intersection += Gradient;
		}
	}


} // namespace lwmf

