/*
****************************************************
*                                                  *
* lwmf_primitives - lightweight media framework    *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <queue>

#include "lwmf_general.hpp"
#include "lwmf_math.hpp"
#include "lwmf_texture.hpp"

namespace lwmf
{

	void SetPixel(TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Color);
	void SetPixelSafe(TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Color);
	std::int_fast32_t GetPixel(const TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y);
	std::int_fast32_t GetPixelSafe(const TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y);
	void ScanlineFill(TextureStruct& Texture, const IntPointStruct& CenterPoint, std::int_fast32_t FillColor);
	std::int_fast32_t FindRegion(std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t x, std::int_fast32_t y);
	bool ClipLine(std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t x1, std::int_fast32_t y1, std::int_fast32_t x2, std::int_fast32_t y2, std::int_fast32_t& x3, std::int_fast32_t& y3, std::int_fast32_t& x4, std::int_fast32_t& y4);
	void Line(TextureStruct& Texture, std::int_fast32_t x1, std::int_fast32_t y1, std::int_fast32_t x2, std::int_fast32_t y2, std::int_fast32_t Color);
	void Rectangle(TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Color);
	void FilledRectangle(TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);
	void Circle(TextureStruct& Texture, std::int_fast32_t CenterX, std::int_fast32_t CenterY, std::int_fast32_t Radius, std::int_fast32_t Color);
	void FilledCircle(TextureStruct& Texture, std::int_fast32_t CenterX, std::int_fast32_t CenterY, std::int_fast32_t Radius, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);
	void Ellipse(TextureStruct& Texture, std::int_fast32_t CenterX, std::int_fast32_t CenterY, std::int_fast32_t RadiusX, std::int_fast32_t RadiusY, std::int_fast32_t Color);
	IntPointStruct GetPolygonCentroid(const std::vector<IntPointStruct>& Points);
	bool PointInsidePolygon(const std::vector<IntPointStruct>& Points, const IntPointStruct& Point);
	void Polygon(TextureStruct& Texture, const std::vector<IntPointStruct>& Points, std::int_fast32_t BorderColor);
	void FilledPolygon(TextureStruct& Texture, const std::vector<IntPointStruct>& Points, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);

	//
	// Functions
	//

	//
	// Pixel operations
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

	//
	// Fill
	//

	inline void ScanlineFill(TextureStruct& Texture, const IntPointStruct& CenterPoint, const std::int_fast32_t FillColor)
	{
		IntPointStruct Points{ CenterPoint };
		std::vector<IntPointStruct> Stack{};
		Stack.push_back(Points);

		while (!Stack.empty())
		{
			Points = Stack.back();
			Stack.pop_back();

			std::int_fast32_t x1{ Points.X };

			while (x1 >= 0 && Texture.Pixels[Points.Y * Texture.Width + x1] != FillColor)
			{
				--x1;
			}

			++x1;

			bool Above{};
			bool Below{};
			const std::int_fast32_t TempY{ Points.Y * Texture.Width };

			while (x1 < Texture.Width && Texture.Pixels[TempY + x1] != FillColor)
			{
				Texture.Pixels[TempY + x1] = FillColor;

				if (!Above && Points.Y > 0 && Texture.Pixels[(Points.Y - 1) * Texture.Width + x1] != FillColor)
				{
					Stack.push_back({ x1, Points.Y - 1 });
					Above = true;
				}
				else if (Above && Points.Y > 0 && Texture.Pixels[(Points.Y - 1) * Texture.Width + x1] != FillColor)
				{
					Above = false;
				}

				if (!Below && Points.Y < Texture.Height - 1 && Texture.Pixels[(Points.Y + 1) * Texture.Width + x1] != FillColor)
				{
					Stack.push_back({ x1, Points.Y + 1 });
					Below = true;
				}
				else if (Below && Points.Y < Texture.Height - 1 && Texture.Pixels[(Points.Y + 1) * Texture.Width + x1] != FillColor)
				{
					Below = false;
				}

				++x1;
			}
		}
	}

	//
	// Lines
	//

	// FindRegion and ClipLine provide the "Cohen Sutherland Clipping" algorithm for lines
	// https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm

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

		//
		// Clip line coordinates to fit into screen boundaries
		//

		if (!ClipLine(Texture.Width, Texture.Height, x1, y1, x2, y2, x1, y1, x2, y2))
		{
			return;
		}

		bool YLonger{};
		std::int_fast32_t ShortLength{ y2 - y1 };
		std::int_fast32_t LongLength{ x2 - x1 };

		if (std::abs(ShortLength) > std::abs(LongLength))
		{
			std::swap(ShortLength, LongLength);
			YLonger = true;
		}

		const std::int_fast32_t DecInc{ LongLength == 0 ? 0 : (ShortLength << 16) / LongLength };

		if (YLonger)
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
	// Rectangles
	//

	inline void Rectangle(TextureStruct& Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, const std::int_fast32_t Color)
	{
		--Width;
		--Height;

		// Exit early if coords are out of texture boundaries
		if (PosX > Texture.Width || PosX + Width < 0 || PosY > Texture.Height || PosY + Height < 0)
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
		// Exit early if coords are out of texture boundaries
		if (PosX > Texture.Width || PosX + Width - 1 < 0 || PosY > Texture.Height || PosY + Height - 1 < 0)
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

	//
	// Circles
	//

	inline void Circle(TextureStruct& Texture, const std::int_fast32_t CenterX, const std::int_fast32_t CenterY, std::int_fast32_t Radius, const std::int_fast32_t Color)
	{
		// Exit early if coords are out of texture boundaries
		if (CenterX + Radius < 0 || CenterX - Radius > Texture.Width || CenterY + Radius < 0 || CenterY - Radius > Texture.Height)
		{
			return;
		}

		IntPointStruct Point{ -Radius, 0 };
		std::int_fast32_t Error{ 2 - (Radius << 1) };
		bool SafeFlag{};

		// if complete circle is within texture boundaries, there is no reason to use SetPixelSafe...
		if ((CenterX - Radius >= 0 && CenterX + Radius < Texture.Width) && (CenterY - Radius >= 0 && CenterY + Radius < Texture.Height))
		{
			SafeFlag = true;
		}

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
		// Exit early if coords are out of texture boundaries
		if (CenterX + Radius < 0 || CenterX - Radius > Texture.Width || CenterY + Radius < 0 || CenterY - Radius > Texture.Height)
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

	//
	// Ellipses
	//

	inline void Ellipse(TextureStruct& Texture, const std::int_fast32_t CenterX, const std::int_fast32_t CenterY, const std::int_fast32_t RadiusX, const std::int_fast32_t RadiusY, const std::int_fast32_t Color)
	{
		// Exit early if coords are out of texture boundaries
		if (CenterX + RadiusX < 0 || CenterX - RadiusX > Texture.Width || CenterY + RadiusY < 0 || CenterY - RadiusY > Texture.Height)
		{
			return;
		}

		const std::int_fast32_t RadiusYTemp{ RadiusY * RadiusY };
		const std::int_fast32_t RadiusXTemp{ RadiusX * RadiusX };
		const std::int_fast32_t TwoRadiusYTemp{ RadiusYTemp << 1 };
		const std::int_fast32_t TwoRadiusXTemp{ RadiusXTemp << 1 };

		IntPointStruct Point{ 0, RadiusY };
		IntPointStruct Temp{ TwoRadiusYTemp * Point.X, TwoRadiusXTemp * Point.Y };
		bool SafeFlag{};

		// if complete ellipse is within texture boundaries, there is no reason to use SetPixelSafe...
		if ((CenterX - RadiusX >= 0 && CenterX + RadiusX < Texture.Width) && (CenterY - RadiusY >= 0 && CenterY + RadiusY < Texture.Height))
		{
			SafeFlag = true;
		}

		float p1{ static_cast<float>(RadiusYTemp - (RadiusXTemp * RadiusY) + (RadiusXTemp >> 2)) };

		while (Temp.X <= Temp.Y)
		{
			if (SafeFlag)
			{
				Texture.Pixels[((CenterY + Point.Y) * Texture.Width) + (CenterX + Point.X)] = Color;
				Texture.Pixels[((CenterY + Point.Y) * Texture.Width) + (CenterX - Point.X)] = Color;
				Texture.Pixels[((CenterY - Point.Y) * Texture.Width) + (CenterX + Point.X)] = Color;
				Texture.Pixels[((CenterY - Point.Y) * Texture.Width) + (CenterX - Point.X)] = Color;
			}
			else
			{
				SetPixelSafe(Texture, CenterX + Point.X, CenterY + Point.Y, Color);
				SetPixelSafe(Texture, CenterX - Point.X, CenterY + Point.Y, Color);
				SetPixelSafe(Texture, CenterX + Point.X, CenterY - Point.Y, Color);
				SetPixelSafe(Texture, CenterX - Point.X, CenterY - Point.Y, Color);
			}

			++Point.X;

			if (p1 < 0.0F)
			{
				Temp.X = TwoRadiusYTemp * Point.X;
				p1 = p1 + Temp.X + RadiusYTemp;
			}
			else
			{
				--Point.Y;
				Temp = { TwoRadiusYTemp * Point.X, TwoRadiusXTemp * Point.Y };
				p1 = p1 + Temp.X - Temp.Y + RadiusYTemp;
			}

			if (SafeFlag)
			{
				Texture.Pixels[((CenterY + Point.Y) * Texture.Width) + (CenterX + Point.X)] = Color;
				Texture.Pixels[((CenterY + Point.Y) * Texture.Width) + (CenterX - Point.X)] = Color;
				Texture.Pixels[((CenterY - Point.Y) * Texture.Width) + (CenterX + Point.X)] = Color;
				Texture.Pixels[((CenterY - Point.Y) * Texture.Width) + (CenterX - Point.X)] = Color;
			}
			else
			{
				SetPixelSafe(Texture, CenterX + Point.X, CenterY + Point.Y, Color);
				SetPixelSafe(Texture, CenterX - Point.X, CenterY + Point.Y, Color);
				SetPixelSafe(Texture, CenterX + Point.X, CenterY - Point.Y, Color);
				SetPixelSafe(Texture, CenterX - Point.X, CenterY - Point.Y, Color);
			}
		}

		float p2{ (RadiusYTemp * (Point.X + 0.5F) * (Point.X + 0.5F)) + (RadiusXTemp * (Point.Y - 1) * (Point.Y - 1)) - (RadiusXTemp * RadiusYTemp) };
		Temp = { 0, 0 };

		while (Point.Y >= 0)
		{
			if (SafeFlag)
			{
				Texture.Pixels[((CenterY + Point.Y) * Texture.Width) + (CenterX + Point.X)] = Color;
				Texture.Pixels[((CenterY + Point.Y) * Texture.Width) + (CenterX - Point.X)] = Color;
				Texture.Pixels[((CenterY - Point.Y) * Texture.Width) + (CenterX + Point.X)] = Color;
				Texture.Pixels[((CenterY - Point.Y) * Texture.Width) + (CenterX - Point.X)] = Color;
			}
			else
			{
				SetPixelSafe(Texture, CenterX + Point.X, CenterY + Point.Y, Color);
				SetPixelSafe(Texture, CenterX - Point.X, CenterY + Point.Y, Color);
				SetPixelSafe(Texture, CenterX + Point.X, CenterY - Point.Y, Color);
				SetPixelSafe(Texture, CenterX - Point.X, CenterY - Point.Y, Color);
			}

			--Point.Y;

			if (p2 < 0.0F)
			{
				++Point.X;
				Temp = { TwoRadiusYTemp * Point.X, TwoRadiusXTemp * Point.Y };;
				p2 = p2 + Temp.X - Temp.Y + RadiusXTemp;
			}
			else
			{
				Temp.Y = TwoRadiusXTemp * Point.Y;
				p2 = p2 - Temp.Y + RadiusXTemp;
			}

			if (SafeFlag)
			{
				Texture.Pixels[((CenterY + Point.Y) * Texture.Width) + (CenterX + Point.X)] = Color;
				Texture.Pixels[((CenterY + Point.Y) * Texture.Width) + (CenterX - Point.X)] = Color;
				Texture.Pixels[((CenterY - Point.Y) * Texture.Width) + (CenterX + Point.X)] = Color;
				Texture.Pixels[((CenterY - Point.Y) * Texture.Width) + (CenterX - Point.X)] = Color;
			}
			else
			{
				SetPixelSafe(Texture, CenterX + Point.X, CenterY + Point.Y, Color);
				SetPixelSafe(Texture, CenterX - Point.X, CenterY + Point.Y, Color);
				SetPixelSafe(Texture, CenterX + Point.X, CenterY - Point.Y, Color);
				SetPixelSafe(Texture, CenterX - Point.X, CenterY - Point.Y, Color);
			}
		}
	}

	//
	// Polygons
	//

	inline IntPointStruct GetPolygonCentroid(const std::vector<IntPointStruct>& Points)
	{
		float SignedArea{};
		FloatPointStruct Centroid{};
		const std::size_t NumberOfPoints{ Points.size() };

		for (std::size_t i{}; i < NumberOfPoints; ++i)
		{
			const FloatPointStruct AreaPoint{ static_cast<float>(Points[(i + 1) & (NumberOfPoints - 1)].X), static_cast<float>(Points[(i + 1) & (NumberOfPoints - 1)].Y) };
			const float Area{ Points[i].X * AreaPoint.Y - AreaPoint.X * Points[i].Y };
			SignedArea += Area;
			Centroid.X += (Points[i].X + AreaPoint.X) * Area;
			Centroid.Y += (Points[i].Y + AreaPoint.Y) * Area;
		}

		const float TempArea{ 3.0F * SignedArea };

		return { static_cast<std::int_fast32_t>(Centroid.X /= TempArea), static_cast<std::int_fast32_t>(Centroid.Y /= TempArea) };
	}

	inline bool PointInsidePolygon(const std::vector<IntPointStruct>& Points, const IntPointStruct& Point)
	{
		const std::size_t NumberOfPoints{ Points.size() };
		bool Result{};

		for (std::size_t i{}; i < NumberOfPoints; ++i)
		{
			const std::size_t j{ (i + 1) % NumberOfPoints };

			if (((Points[j].Y <= Point.Y && Point.Y < Points[i].Y) || (Points[i].Y <= Point.Y && Point.Y < Points[j].Y))
				&& (Point.X < Points[j].X + (Points[i].X - Points[j].X) * (Point.Y - Points[j].Y) / (Points[i].Y - Points[j].Y)))
			{
				Result = !Result;
			}
		}

		return Result;
	}

	inline void Polygon(TextureStruct& Texture, const std::vector<IntPointStruct>& Points, const std::int_fast32_t BorderColor)
	{
		std::int_fast32_t Index{};

		for (Index; Index < static_cast<std::int_fast32_t>(Points.size()) - 1; ++Index)
		{
			Line(Texture, Points[Index].X, Points[Index].Y, Points[Index + 1].X, Points[Index + 1].Y, BorderColor);
		}

		Line(Texture, Points[Index].X, Points[Index].Y, Points[0].X, Points[0].Y, BorderColor);
	}

	inline void FilledPolygon(TextureStruct& Texture, const std::vector<IntPointStruct>& Points, const std::int_fast32_t BorderColor, const std::int_fast32_t FillColor)
	{
		Polygon(Texture, Points, FillColor);

		if (const IntPointStruct CentroidPoint{ GetPolygonCentroid(Points) }; PointInsidePolygon(Points, CentroidPoint))
		{
			ScanlineFill(Texture, CentroidPoint, FillColor);
		}

		if (BorderColor != FillColor)
		{
			Polygon(Texture, Points, BorderColor);
		}
	}


} // namespace lwmf