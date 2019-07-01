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

#include "lwmf_general.hpp"
#include "lwmf_math.hpp"
#include "lwmf_texture.hpp"

namespace lwmf
{

	void SetPixel(TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Color);
	void SetPixelSafe(TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Color);
	std::int_fast32_t GetPixel(const TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y);
	void BoundaryFill(TextureStruct& Texture, IntPointStruct& CenterPoints, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);
	void Line(TextureStruct& Texture, std::int_fast32_t x1, std::int_fast32_t y1, std::int_fast32_t x2, std::int_fast32_t y2, std::int_fast32_t Color);
	void Rectangle(TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Color);
	void FilledRectangle(TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Color);
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
		if (static_cast<std::uint_fast32_t>(x) <= static_cast<std::uint_fast32_t>(Texture.Width) && static_cast<std::uint_fast32_t>(y) < static_cast<std::uint_fast32_t>(Texture.Height))
		{
			Texture.Pixels[y * Texture.Width + x] = Color;
		}
	}

	inline std::int_fast32_t GetPixel(const TextureStruct& Texture, const std::int_fast32_t x, const std::int_fast32_t y)
	{
		return Texture.Pixels[y * Texture.Width + x];
	}

	//
	// Flood Fill
	//

	inline void BoundaryFill(TextureStruct& Texture, IntPointStruct& CenterPoints, const std::int_fast32_t BorderColor, const std::int_fast32_t FillColor)
	{
		std::vector<IntPointStruct> Stack;
		Stack.push_back(CenterPoints);

		while (!Stack.empty())
		{
			CenterPoints = Stack.back();
			Stack.pop_back();

			std::int_fast32_t x1{ CenterPoints.X };

			while (x1 >= 0 && Texture.Pixels[CenterPoints.Y * Texture.Width + x1] != BorderColor)
			{
				--x1;
			}

			++x1;

			bool Above{};
			bool Below{};

			const std::int_fast32_t TempY{ CenterPoints.Y * Texture.Width };

			while (x1 < Texture.Width && Texture.Pixels[TempY + x1] != BorderColor)
			{
				Texture.Pixels[TempY + x1] = FillColor;

				if (!Above && CenterPoints.Y > 0 && Texture.Pixels[(CenterPoints.Y - 1) * Texture.Width + x1] != BorderColor)
				{
					Stack.push_back({ x1, CenterPoints.Y - 1 });
					Above = true;
				}
				else if (Above && CenterPoints.Y > 0 && Texture.Pixels[(CenterPoints.Y - 1) * Texture.Width + x1] != BorderColor)
				{
					Above = false;
				}

				if (!Below && CenterPoints.Y < Texture.Height - 1 && Texture.Pixels[(CenterPoints.Y + 1) * Texture.Width + x1] != BorderColor)
				{
					Stack.push_back({ x1, CenterPoints.Y + 1 });
					Below = true;
				}
				else if (Below && CenterPoints.Y < Texture.Height - 1 && Texture.Pixels[(CenterPoints.Y + 1) * Texture.Width + x1] != BorderColor)
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
		// Case 1: Straight horizontal line within screen boundaries
		if ((y1 == y2) && (x2 > x1) && (x1 >= 0 && x2 <= Texture.Width && y1 >= 0 && y1 < Texture.Height))
		{
			std::fill(Texture.Pixels.begin() + y1 * Texture.Width + x1, Texture.Pixels.begin() + y1 * Texture.Width + x2, Color);
		}
		// Case 2: Line is within screen boundaries, so no further checking if pixel can be set
		else if (static_cast<std::uint_fast32_t>(x1) <= static_cast<std::uint_fast32_t>(Texture.Width) && static_cast<std::uint_fast32_t>(y1) < static_cast<std::uint_fast32_t>(Texture.Height) 
			&& static_cast<std::uint_fast32_t>(x2) <= static_cast<std::uint_fast32_t>(Texture.Width) && static_cast<std::uint_fast32_t>(y2) < static_cast<std::uint_fast32_t>(Texture.Height))
		{
			bool LongerY{};
			std::int_fast32_t ShortLength{ y2 - y1 };
			std::int_fast32_t LongLength{ x2 - x1 };

			if (std::abs(ShortLength) > std::abs(LongLength))
			{
				std::swap(ShortLength, LongLength);
				LongerY = true;
			}

			const std::int_fast32_t Value{ LongLength == 0 ? 0 : (ShortLength << 16) / LongLength };

			if (LongerY)
			{
				if (LongLength > 0)
				{
					LongLength += y1;

					for (std::int_fast32_t j{ 0x8000 + (x1 << 16) }; y1 <= LongLength; ++y1)
					{
						Texture.Pixels[y1 * Texture.Width + (j >> 16)] = Color;
						j += Value;
					}

					return;
				}

				LongLength += y1;

				for (std::int_fast32_t j{ 0x8000 + (x1 << 16) }; y1 >= LongLength; --y1)
				{
					Texture.Pixels[y1 * Texture.Width + (j >> 16)] = Color;
					j -= Value;
				}

				return;
			}

			if (LongLength > 0)
			{
				LongLength += x1;

				for (std::int_fast32_t j{ 0x8000 + (y1 << 16) }; x1 <= LongLength; ++x1)
				{
					Texture.Pixels[(j >> 16) * Texture.Width + x1] = Color;
					j += Value;
				}

				return;
			}

			LongLength += x1;

			for (std::int_fast32_t j{ 0x8000 + (y1 << 16) }; x1 >= LongLength; --x1)
			{
				Texture.Pixels[(j >> 16) * Texture.Width + x1] = Color;
				j -= Value;
			}
		}
		// Case 3: Check each pixel if it´s within screen boundaries (slowest)
		else
		{
			const IntPointStruct d{ x2 - x1, y2 - y1 };
			const IntPointStruct d1{ std::abs(d.X), std::abs(d.Y) };

			if (std::int_fast32_t x{}, y{}; d1.Y <= d1.X)
			{
				std::int_fast32_t px{ (d1.Y << 1) - d1.X };
				std::int_fast32_t xe{};

				d.X >= 0 ? (x = x1, y = y1, xe = x2) : (x = x2, y = y2, xe = x1);
				SetPixelSafe(Texture, x, y, Color);

				for (std::int_fast32_t i{}; x < xe; ++i)
				{
					++x;
					px < 0 ? px += d1.Y << 1 : ((d.X < 0 && d.Y < 0) || (d.X > 0 && d.Y > 0) ? ++y : --y, px += (d1.Y - d1.X) << 1);
					SetPixelSafe(Texture, x, y, Color);
				}
			}
			else
			{
				std::int_fast32_t py{ (d1.X << 1) - d1.Y };
				std::int_fast32_t ye{};

				d.Y >= 0 ? (x = x1, y = y1, ye = y2) : (x = x2, y = y2, ye = y1);
				SetPixelSafe(Texture, x, y, Color);

				for (std::int_fast32_t i{}; y < ye; ++i)
				{
					++y;
					py <= 0 ? py += d1.X << 1 : ((d.X < 0 && d.Y < 0) || (d.X > 0 && d.Y > 0) ? ++x : --x, py += (d1.X - d1.Y) << 1);
					SetPixelSafe(Texture, x, y, Color);
				}
			}
		}

	}

	//
	// Rectangles
	//

	inline void Rectangle(TextureStruct& Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height, const std::int_fast32_t Color)
	{
		Line(Texture, PosX, PosY, PosX + Width, PosY, Color);
		Line(Texture, PosX, PosY, PosX, PosY + Height, Color);
		Line(Texture, PosX, PosY + Height, PosX + Width, PosY + Height, Color);
		Line(Texture, PosX + Width, PosY, PosX + Width, PosY + Height, Color);
	}

	inline void FilledRectangle(TextureStruct& Texture, const std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, const std::int_fast32_t Color)
	{
		for (std::int_fast32_t y{ PosY }; y <= PosY + Height; ++y)
		{
			const std::int_fast32_t TempWidth{ y * Texture.Width + PosX };
			std::fill(Texture.Pixels.begin() + TempWidth, Texture.Pixels.begin() + TempWidth + Width, Color);
		}
	}

	//
	// Circles
	//

	inline void Circle(TextureStruct& Texture, const std::int_fast32_t CenterX, const std::int_fast32_t CenterY, std::int_fast32_t Radius, const std::int_fast32_t Color)
	{
		std::int_fast32_t y{};
		std::int_fast32_t Error{};

		// if complete circle is within screen boundaries, there is no reason to use SetPixelSafe...
		if ((CenterX - Radius >= 0 && CenterX + Radius <= Texture.Width) && (CenterY - Radius >= 0 && CenterY + Radius < Texture.Height))
		{
			while (Radius >= y)
			{
				Texture.Pixels[(CenterY + y) * Texture.Width + CenterX + Radius] = Color;
				Texture.Pixels[(CenterY + Radius) * Texture.Width + CenterX + y] = Color;
				Texture.Pixels[(CenterY + Radius) * Texture.Width + CenterX - y] = Color;
				Texture.Pixels[(CenterY + y) * Texture.Width + CenterX - Radius] = Color;
				Texture.Pixels[(CenterY - y) * Texture.Width + CenterX - Radius] = Color;
				Texture.Pixels[(CenterY - Radius) * Texture.Width + CenterX - y] = Color;
				Texture.Pixels[(CenterY - Radius) * Texture.Width + CenterX + y] = Color;
				Texture.Pixels[(CenterY - y) * Texture.Width + CenterX + Radius] = Color;

				Error <= 0 ? (++y, Error += (y << 1) + 1) : (--Radius, Error -= (Radius << 1) + 1);
			}
		}
		// ...or use the "safe" version!
		else
		{
			while (Radius >= y)
			{
				SetPixelSafe(Texture, CenterX + Radius, CenterY + y, Color);
				SetPixelSafe(Texture, CenterX + y, CenterY + Radius, Color);
				SetPixelSafe(Texture, CenterX - y, CenterY + Radius, Color);
				SetPixelSafe(Texture, CenterX - Radius, CenterY + y, Color);
				SetPixelSafe(Texture, CenterX - Radius, CenterY - y, Color);
				SetPixelSafe(Texture, CenterX - y, CenterY - Radius, Color);
				SetPixelSafe(Texture, CenterX + y, CenterY - Radius, Color);
				SetPixelSafe(Texture, CenterX + Radius, CenterY - y, Color);

				Error <= 0 ? (++y, Error += (y << 1) + 1) : (--Radius, Error -= (Radius << 1) + 1);
			}
		}
	}

	inline void FilledCircle(TextureStruct& Texture, const std::int_fast32_t CenterX, const std::int_fast32_t CenterY, const std::int_fast32_t Radius, const std::int_fast32_t BorderColor, const std::int_fast32_t FillColor)
	{
		Circle(Texture, CenterX, CenterY, Radius, BorderColor);

		const std::int_fast32_t InnerRadius{ Radius - 1 };
		const std::int_fast32_t PowInnerRadius{ InnerRadius * InnerRadius };

		// if complete circle is within screen boundaries, there is no reason to use SetPixelSafe...
		if (CenterX - Radius >= 0 && CenterX + Radius <= Texture.Width && CenterY - Radius >= 0 && CenterY + Radius < Texture.Height)
		{
			for (std::int_fast32_t y{ -InnerRadius }; y <= InnerRadius; ++y)
			{
				const std::int_fast32_t PowY{ y * y };
				const std::int_fast32_t TempY{ (CenterY + y) * Texture.Width };

				for (std::int_fast32_t x{ -InnerRadius }; x <= InnerRadius; ++x)
				{
					if (x * x + PowY <= PowInnerRadius)
					{
						Texture.Pixels[TempY + CenterX + x] = FillColor;
					}
				}
			}
		}
		// ...or use the "safe" version!
		else
		{
			for (std::int_fast32_t y{ -InnerRadius }; y <= InnerRadius; ++y)
			{
				const std::int_fast32_t PowY{ y * y };

				for (std::int_fast32_t x{ -InnerRadius }; x <= InnerRadius; ++x)
				{
					if (x * x + PowY <= PowInnerRadius)
					{
						SetPixelSafe(Texture, CenterX + x, CenterY + y, FillColor);
					}
				}
			}
		}
	}

	//
	// Polygons
	//

	inline IntPointStruct GetPolygonCentroid(const std::vector<IntPointStruct>& Points)
	{
		float SignedArea{};
		FloatPointStruct Centroid;

		const std::int_fast32_t NumberOfPoints{ static_cast<std::int_fast32_t>(Points.size()) };

		for (std::int_fast32_t i{}; i < NumberOfPoints; ++i)
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
		std::int_fast32_t Counter{};
		IntPointStruct Point1{ Points[0] };
		const std::int_fast32_t NumberOfPoints{ static_cast<std::int_fast32_t>(Points.size()) };

		for (std::int_fast32_t i{ 1 }; i <= NumberOfPoints; ++i)
		{
			IntPointStruct Point2{ Points[i & (NumberOfPoints - 1)] };

			if ((Point.Y > (std::min)(Point1.Y, Point2.Y)) && (Point.Y <= (std::max)(Point1.Y, Point2.Y)) && (Point.X <= (std::max)(Point1.X, Point2.X)))
			{
				if ((Point1.Y != Point2.Y) && (Point1.X == Point2.X || Point.X <= (Point.Y - Point1.Y) * (Point2.X - Point1.X) / (Point2.Y - Point1.Y) + Point1.X))
				{
					++Counter;
				}
			}

			Point1 = Point2;
		}

		return (Counter & 1) == 0 ? false : true;
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
		Polygon(Texture, Points, BorderColor);
		IntPointStruct CentroidPoint{ GetPolygonCentroid(Points) };

		if (PointInsidePolygon(Points, CentroidPoint))
		{
			BoundaryFill(Texture, CentroidPoint, BorderColor, FillColor);
		}
	}


} // namespace lwmf