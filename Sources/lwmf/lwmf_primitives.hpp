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
	void Line(TextureStruct& Texture, std::int_fast32_t x1, std::int_fast32_t y1, std::int_fast32_t x2, std::int_fast32_t y2, std::int_fast32_t Color);
	void Rectangle(TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Color);
	void FilledRectangle(TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);
	void Circle(TextureStruct& Texture, std::int_fast32_t CenterX, std::int_fast32_t CenterY, std::int_fast32_t Radius, std::int_fast32_t Color);
	void FilledCircle(TextureStruct& Texture, std::int_fast32_t CenterX, std::int_fast32_t CenterY, std::int_fast32_t Radius, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);
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

	inline void Line(TextureStruct& Texture, std::int_fast32_t x1, std::int_fast32_t y1, const std::int_fast32_t x2, const std::int_fast32_t y2, const std::int_fast32_t Color)
	{
		// Exit early if coords are out of visual boundaries
		if ((x1 < 0 && x2 < 0) || (x1 > Texture.Width && x2 > Texture.Width) || (y1 < 0 && y2 < 0) || (y1 > Texture.Height && y2 > Texture.Height))
		{
			return;
		}

		// Case 1: Straight horizontal line within screen boundaries
		if ((x1 >= 0 && y1 >= 0) && (x2 < Texture.Width && y1 < Texture.Height) && (y1 == y2) && (x2 > x1))
		{
			std::fill(Texture.Pixels.begin() + y1 * Texture.Width + x1, Texture.Pixels.begin() + y1 * Texture.Width + x2 + 1, Color);
			return;
		}

		// The two other variants use "EFLA" - "Extremely Fast Line Algorithm"
		// http://www.edepot.com/algorithm.html

		bool YLonger{};
		std::int_fast32_t ShortLength{ y2 - y1 };
		std::int_fast32_t LongLength{ x2 - x1 };

		if (std::abs(ShortLength) > std::abs(LongLength))
		{
			std::swap(ShortLength, LongLength);
			YLonger = true;
		}

		const std::int_fast32_t DecInc{ LongLength == 0 ? 0 : (ShortLength << 16) / LongLength };

		// Case 2: Line is within screen boundaries, so no further checking if pixel can be set
		if (static_cast<std::uint_fast32_t>(x1) < static_cast<std::uint_fast32_t>(Texture.Width) && static_cast<std::uint_fast32_t>(y1) < static_cast<std::uint_fast32_t>(Texture.Height)
			&& static_cast<std::uint_fast32_t>(x2) < static_cast<std::uint_fast32_t>(Texture.Width) && static_cast<std::uint_fast32_t>(y2) < static_cast<std::uint_fast32_t>(Texture.Height))
		{
			if (YLonger)
			{
				if (LongLength > 0)
				{
					LongLength += y1;

					for (std::int_fast32_t j{ 0x8000 + (x1 << 16) }; y1 <= LongLength; ++y1)
					{
						Texture.Pixels[y1 * Texture.Width + (j >> 16)] = Color;
						j += DecInc;
					}

					return;
				}

				LongLength += y1;

				for (std::int_fast32_t j{ 0x8000 + (x1 << 16) }; y1 >= LongLength; --y1)
				{
					Texture.Pixels[y1 * Texture.Width + (j >> 16)] = Color;
					j -= DecInc;
				}

				return;
			}

			if (LongLength > 0)
			{
				LongLength += x1;

				for (std::int_fast32_t j{ 0x8000 + (y1 << 16) }; x1 <= LongLength; ++x1)
				{
					Texture.Pixels[(j >> 16) * Texture.Width + x1] = Color;
					j += DecInc;
				}

				return;
			}

			LongLength += x1;

			for (std::int_fast32_t j{ 0x8000 + (y1 << 16) }; x1 >= LongLength; --x1)
			{
				Texture.Pixels[(j >> 16) * Texture.Width + x1] = Color;
				j -= DecInc;
			}
		}
		// Case 3: Check each pixel if it´s within screen boundaries (slowest)
		else
		{
			if (YLonger)
			{
				if (LongLength > 0)
				{
					LongLength += y1;

					for (std::int_fast32_t j{ 0x8000 + (x1 << 16) }; y1 <= LongLength; ++y1)
					{
						SetPixelSafe(Texture, j >> 16, y1, Color);
						j += DecInc;
					}

					return;
				}

				LongLength += y1;

				for (std::int_fast32_t j{ 0x8000 + (x1 << 16) }; y1 >= LongLength; --y1)
				{
					SetPixelSafe(Texture, j >> 16, y1, Color);
					j -= DecInc;
				}

				return;
			}

			if (LongLength > 0)
			{
				LongLength += x1;

				for (std::int_fast32_t j{ 0x8000 + (y1 << 16) }; x1 <= LongLength; ++x1)
				{
					SetPixelSafe(Texture, x1, j >> 16, Color);
					j += DecInc;
				}

				return;
			}

			LongLength += x1;

			for (std::int_fast32_t j{ 0x8000 + (y1 << 16) }; x1 >= LongLength; --x1)
			{
				SetPixelSafe(Texture, x1, j >> 16, Color);
				j -= DecInc;
			}
		}
	}

	//
	// Rectangles
	//

	inline void Rectangle(TextureStruct& Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, const std::int_fast32_t Color)
	{
		--Width;
		--Height;

		// Exit early if coords are out of visual boundaries
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
		// Exit early if coords are out of visual boundaries
		if (PosX > Texture.Width || PosX + Width - 1 < 0 || PosY > Texture.Height || PosY + Height - 1 < 0)
		{
			return;
		}

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
		// Exit early if coords are out of visual boundaries
		if (CenterX + Radius < 0 || CenterX - Radius > Texture.Width || CenterY + Radius < 0 || CenterY - Radius > Texture.Height)
		{
			return;
		}

		IntPointStruct Dot{ -Radius, 0 };
		std::int_fast32_t Error{ 2 - (Radius << 1) };

		// if complete circle is within screen boundaries, there is no reason to use SetPixelSafe...
		if ((CenterX - Radius >= 0 && CenterX + Radius < Texture.Width) && (CenterY - Radius >= 0 && CenterY + Radius < Texture.Height))
		{
			do
			{
				Texture.Pixels[((CenterY + Dot.Y) * Texture.Width) + (CenterX - Dot.X)] = Color;
				Texture.Pixels[((CenterY - Dot.X) * Texture.Width) + (CenterX - Dot.Y)] = Color;
				Texture.Pixels[((CenterY - Dot.Y) * Texture.Width) + (CenterX + Dot.X)] = Color;
				Texture.Pixels[((CenterY + Dot.X) * Texture.Width) + (CenterX + Dot.Y)] = Color;
				Radius = Error;

				if (Radius <= Dot.Y)
				{
					Error += (++Dot.Y << 1) + 1;
				}

				if (Radius > Dot.X || Error > Dot.Y)
				{
					Error += (++Dot.X << 1) + 1;
				}
			} while (Dot.X < 0);
		}
		// ...or use the "safe" version!
		else
		{
			do
			{
				SetPixelSafe(Texture, CenterX - Dot.X, CenterY + Dot.Y, Color);
				SetPixelSafe(Texture, CenterX - Dot.Y, CenterY - Dot.X, Color);
				SetPixelSafe(Texture, CenterX + Dot.X, CenterY - Dot.Y, Color);
				SetPixelSafe(Texture, CenterX + Dot.Y, CenterY + Dot.X, Color);
				Radius = Error;

				if (Radius <= Dot.Y)
				{
					Error += (++Dot.Y << 1) + 1;
				}

				if (Radius > Dot.X || Error > Dot.Y)
				{
					Error += (++Dot.X << 1) + 1;
				}
			} while (Dot.X < 0);
		}
	}

	inline void FilledCircle(TextureStruct& Texture, const std::int_fast32_t CenterX, const std::int_fast32_t CenterY, const std::int_fast32_t Radius, const std::int_fast32_t BorderColor, const std::int_fast32_t FillColor)
	{
		// Exit early if coords are out of visual boundaries
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