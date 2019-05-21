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
#include "lwmf_pixelbuffer.hpp"

namespace lwmf
{


	void SetPixel(std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Color);
	void SetPixelSafe(std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Color);
	std::int_fast32_t GetPixel(std::int_fast32_t x, std::int_fast32_t y);
	void BoundaryFill(IntPointStruct& CenterPoints, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);
	void Line(std::int_fast32_t x1, std::int_fast32_t y1, std::int_fast32_t x2, std::int_fast32_t y2, std::int_fast32_t Color);
	void Rectangle(std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Color);
	void FilledRectangle(std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Color);
	void Circle(std::int_fast32_t CenterX, std::int_fast32_t CenterY, std::int_fast32_t Radius, std::int_fast32_t Color);
	void FilledCircle(std::int_fast32_t CenterX, std::int_fast32_t CenterY, std::int_fast32_t Radius, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);
	IntPointStruct GetPolygonCentroid(const std::vector<IntPointStruct>& Points);
	bool PointInsidePolygon(const std::vector<IntPointStruct>& Points, const IntPointStruct& Point);
	void Polygon(const std::vector<IntPointStruct>& Points, std::int_fast32_t BorderColor);
	void FilledPolygon(const std::vector<IntPointStruct>& Points, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);

	//
	// Functions
	//

	//
	// Pixel operations
	//

	inline void SetPixel(const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t Color)
	{
		PixelBuffer[y * ViewportWidth + x] = Color;
	}

	inline void SetPixelSafe(const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t Color)
	{
		if (x >= 0 && x <= ViewportWidth && y >= 0 && y < ViewportHeight)
		{
			PixelBuffer[y * ViewportWidth + x] = Color;
		}
	}

	inline std::int_fast32_t GetPixel(const std::int_fast32_t x, const std::int_fast32_t y)
	{
		return PixelBuffer[y * ViewportWidth + x];
	}

	//
	// Flood Fill
	//

	inline void BoundaryFill(IntPointStruct& CenterPoints, const std::int_fast32_t BorderColor, const std::int_fast32_t FillColor)
	{
		std::vector<IntPointStruct> Stack;
		Stack.push_back(CenterPoints);

		while (!Stack.empty())
		{
			CenterPoints = Stack.back();
			Stack.pop_back();

			std::int_fast32_t x1{ CenterPoints.X };

			while (x1 >= 0 && PixelBuffer[CenterPoints.Y * ViewportWidth + x1] != BorderColor)
			{
				--x1;
			}

			++x1;

			bool Above{};
			bool Below{};

			const std::int_fast32_t TempY{ CenterPoints.Y * ViewportWidth };

			while (x1 < ViewportWidth && PixelBuffer[TempY + x1] != BorderColor)
			{
				PixelBuffer[TempY + x1] = FillColor;

				if (!Above && CenterPoints.Y > 0 && PixelBuffer[(CenterPoints.Y - 1) * ViewportWidth + x1] != BorderColor)
				{
					Stack.push_back({ x1, CenterPoints.Y - 1 });
					Above = true;
				}
				else if (Above && CenterPoints.Y > 0 && PixelBuffer[(CenterPoints.Y - 1) * ViewportWidth + x1] != BorderColor)
				{
					Above = false;
				}

				if (!Below && CenterPoints.Y < ViewportHeight - 1 && PixelBuffer[(CenterPoints.Y + 1) * ViewportWidth + x1] != BorderColor)
				{
					Stack.push_back({ x1, CenterPoints.Y + 1 });
					Below = true;
				}
				else if (Below && CenterPoints.Y < ViewportHeight - 1 && PixelBuffer[(CenterPoints.Y + 1) * ViewportWidth + x1] != BorderColor)
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

	inline void Line(const std::int_fast32_t x1, const std::int_fast32_t y1, const std::int_fast32_t x2, const std::int_fast32_t y2, const std::int_fast32_t Color)
	{
		// Case 1: Straight horizontal line within screen boundaries
		if ((y1 == y2) && (x2 > x1) && (x1 >= 0 && x2 <= ViewportWidth && y1 >= 0 && y1 < ViewportHeight))
		{
			std::fill(PixelBuffer.begin() + y1 * ViewportWidth + x1, PixelBuffer.begin() + y1 * ViewportWidth + x2, Color);
		}
		// Case 2: Line is within screen boundaries, so no further checking if pixel can be set
		else if (x1 >= 0 && x1 <= ViewportWidth && y1 >= 0 && y1 < ViewportHeight && x2 >= 0 && x2 <= ViewportWidth && y2 >= 0 && y2 < ViewportHeight)
		{
			const IntPointStruct d{ x2 - x1, y2 - y1 };
			const IntPointStruct d1{ std::abs(d.X), std::abs(d.Y) };

			if (std::int_fast32_t x{}, y{}; d1.Y <= d1.X)
			{
				std::int_fast32_t px{ (d1.Y << 1) - d1.X };
				std::int_fast32_t xe{};

				d.X >= 0 ? (x = x1, y = y1, xe = x2) : (x = x2, y = y2, xe = x1);
				PixelBuffer[y * ViewportWidth + x] = Color;

				for (std::int_fast32_t i{}; x < xe; ++i)
				{
					++x;

					if (px < 0)
					{
						px += d1.Y << 1;
					}
					else
					{
						(d.X < 0 && d.Y < 0) || (d.X > 0 && d.Y > 0) ? ++y : --y;
						px += (d1.Y - d1.X) << 1;
					}

					PixelBuffer[y * ViewportWidth + x] = Color;
				}
			}
			else
			{
				std::int_fast32_t py{ (d1.X << 1) - d1.Y };
				std::int_fast32_t ye{};

				d.Y >= 0 ? (x = x1, y = y1, ye = y2) : (x = x2, y = y2, ye = y1);
				PixelBuffer[y * ViewportWidth + x] = Color;

				for (std::int_fast32_t i{}; y < ye; ++i)
				{
					++y;

					if (py <= 0)
					{
						py += d1.X << 1;
					}
					else
					{
						(d.X < 0 && d.Y < 0) || (d.X > 0 && d.Y > 0) ? ++x : --x;
						py += (d1.X - d1.Y) << 1;
					}

					PixelBuffer[y * ViewportWidth + x] = Color;
				}
			}
		}
		// Case 3: Check each pixel if it�s within screen boundaries (slowest)
		else
		{ //-V523
			const IntPointStruct d{ x2 - x1, y2 - y1 };
			const IntPointStruct d1{ std::abs(d.X), std::abs(d.Y) };

			if (std::int_fast32_t x{}, y{}; d1.Y <= d1.X)
			{
				std::int_fast32_t px{ (d1.Y << 1) - d1.X };
				std::int_fast32_t xe{};

				d.X >= 0 ? (x = x1, y = y1, xe = x2) : (x = x2, y = y2, xe = x1);
				SetPixelSafe(x, y, Color);

				for (std::int_fast32_t i{}; x < xe; ++i)
				{
					++x;

					if (px < 0)
					{
						px += d1.Y << 1;
					}
					else
					{
						(d.X < 0 && d.Y < 0) || (d.X > 0 && d.Y > 0) ? ++y : --y;
						px += (d1.Y - d1.X) << 1;
					}

					SetPixelSafe(x, y, Color);
				}
			}
			else
			{
				std::int_fast32_t py{ (d1.X << 1) - d1.Y };
				std::int_fast32_t ye{};

				d.Y >= 0 ? (x = x1, y = y1, ye = y2) : (x = x2, y = y2, ye = y1);
				SetPixelSafe(x, y, Color);

				for (std::int_fast32_t i{}; y < ye; ++i)
				{
					++y;

					if (py <= 0)
					{
						py += d1.X << 1;
					}
					else
					{
						(d.X < 0 && d.Y < 0) || (d.X > 0 && d.Y > 0) ? ++x : --x;
						py += (d1.X - d1.Y) << 1;
					}

					SetPixelSafe(x, y, Color);
				}
			}
		}

	}

	//
	// Rectangles
	//

	inline void Rectangle(const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height, const std::int_fast32_t Color)
	{
		Line(PosX, PosY, PosX + Width, PosY, Color);
		Line(PosX, PosY, PosX, PosY + Height, Color);
		Line(PosX, PosY + Height, PosX + Width, PosY + Height, Color);
		Line(PosX + Width, PosY, PosX + Width, PosY + Height, Color);
	}

	inline void FilledRectangle(const std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, const std::int_fast32_t Color)
	{
		for (std::int_fast32_t y{ PosY }; y <= PosY + Height; ++y)
		{
			const std::int_fast32_t TempWidth{ y * ViewportWidth + PosX };
			std::fill(PixelBuffer.begin() + TempWidth, PixelBuffer.begin() + TempWidth + Width, Color);
		}
	}

	//
	// Circles
	//

	inline void Circle(const std::int_fast32_t CenterX, const std::int_fast32_t CenterY, std::int_fast32_t Radius, const std::int_fast32_t Color)
	{
		std::int_fast32_t y{};
		std::int_fast32_t Error{};

		// if complete circle is within screen boundaries, there is no reason to use SetPixelSafe...
		if (CenterX - Radius >= 0 && CenterX + Radius <= ViewportWidth && CenterY - Radius >= 0 && CenterY + Radius < ViewportHeight)
		{
			while (Radius >= y)
			{
				PixelBuffer[(CenterY + y) * ViewportWidth + CenterX + Radius] = Color;
				PixelBuffer[(CenterY + Radius) * ViewportWidth + CenterX + y] = Color;
				PixelBuffer[(CenterY + Radius) * ViewportWidth + CenterX - y] = Color;
				PixelBuffer[(CenterY + y) * ViewportWidth + CenterX - Radius] = Color;
				PixelBuffer[(CenterY - y) * ViewportWidth + CenterX - Radius] = Color;
				PixelBuffer[(CenterY - Radius) * ViewportWidth + CenterX - y] = Color;
				PixelBuffer[(CenterY - Radius) * ViewportWidth + CenterX + y] = Color;
				PixelBuffer[(CenterY - y) * ViewportWidth + CenterX + Radius] = Color;

				Error <= 0 ? (++y, Error += (y << 1) + 1) : (--Radius, Error -= (Radius << 1) + 1);
			}
		}
		// ...or use the "safe" version!
		else
		{
			while (Radius >= y)
			{
				SetPixelSafe(CenterX + Radius, CenterY + y, Color);
				SetPixelSafe(CenterX + y, CenterY + Radius, Color);
				SetPixelSafe(CenterX - y, CenterY + Radius, Color);
				SetPixelSafe(CenterX - Radius, CenterY + y, Color);
				SetPixelSafe(CenterX - Radius, CenterY - y, Color);
				SetPixelSafe(CenterX - y, CenterY - Radius, Color);
				SetPixelSafe(CenterX + y, CenterY - Radius, Color);
				SetPixelSafe(CenterX + Radius, CenterY - y, Color);

				Error <= 0 ? (++y, Error += (y << 1) + 1) : (--Radius, Error -= (Radius << 1) + 1);
			}
		}
	}

	inline void FilledCircle(const std::int_fast32_t CenterX, const std::int_fast32_t CenterY, const std::int_fast32_t Radius, const std::int_fast32_t BorderColor, const std::int_fast32_t FillColor)
	{
		Circle(CenterX, CenterY, Radius, BorderColor);

		const std::int_fast32_t InnerRadius{ Radius - 1 };
		const std::int_fast32_t InnerRadiusPOW{ InnerRadius * InnerRadius };

		// if complete circle is within screen boundaries, there is no reason to use SetPixelSafe...
		if (CenterX - Radius >= 0 && CenterX + Radius <= ViewportWidth && CenterY - Radius >= 0 && CenterY + Radius < ViewportHeight)
		{
			for (std::int_fast32_t y{ -InnerRadius }; y <= InnerRadius; ++y)
			{
				const std::int_fast32_t YPOW{ y * y };

				for (std::int_fast32_t x{ -InnerRadius }; x <= InnerRadius; ++x)
				{
					if (x * x + YPOW <= InnerRadiusPOW)
					{
						PixelBuffer[(CenterY + y) * ViewportWidth + CenterX + x] = FillColor;
					}
				}
			}
		}
		// ...or use the "safe" version!
		else
		{
			for (std::int_fast32_t y{ -InnerRadius }; y <= InnerRadius; ++y)
			{
				const std::int_fast32_t YPOW{ y * y };

				for (std::int_fast32_t x{ -InnerRadius }; x <= InnerRadius; ++x)
				{
					if (x * x + YPOW <= InnerRadiusPOW)
					{
						SetPixelSafe(CenterX + x, CenterY + y, FillColor);
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

	inline void Polygon(const std::vector<IntPointStruct>& Points, const std::int_fast32_t BorderColor)
	{
		std::int_fast32_t Index{};

		for (Index; Index < Points.size() - 1; ++Index)
		{
			Line(Points[Index].X, Points[Index].Y, Points[Index + 1].X, Points[Index + 1].Y, BorderColor);
		}

		Line(Points[Index].X, Points[Index].Y, Points[0].X, Points[0].Y, BorderColor);
	}

	inline void FilledPolygon(const std::vector<IntPointStruct>& Points, const std::int_fast32_t BorderColor, const std::int_fast32_t FillColor)
	{
		Polygon(Points, BorderColor);
		IntPointStruct CentroidPoint{ GetPolygonCentroid(Points) };

		if (PointInsidePolygon(Points, CentroidPoint))
		{
			BoundaryFill(CentroidPoint, BorderColor, FillColor);
		}
	}


} // namespace lwmf