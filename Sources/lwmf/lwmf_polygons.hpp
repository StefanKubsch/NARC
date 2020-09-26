/*
****************************************************
*                                                  *
* lwmf_polygons - lightweight media framework      *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <cstdint>
#include <vector>

#include "lwmf_general.hpp"
#include "lwmf_texture.hpp"
#include "lwmf_pixel.hpp"
#include "lwmf_lines.hpp"
#include "lwmf_fill.hpp"

namespace lwmf
{


	FloatPointStruct GetPolygonCentroid(const std::vector<FloatPointStruct>& Points);
	bool PointInsidePolygon(const std::vector<FloatPointStruct>& Points, const FloatPointStruct& Point);
	void Polygon(TextureStruct& Texture, const std::vector<IntPointStruct>& Points, std::int_fast32_t BorderColor);
	void FilledPolygon(TextureStruct& Texture, const std::vector<IntPointStruct>& Points, std::int_fast32_t BorderColor, std::int_fast32_t FillColor);

	//
	// Functions
	//

	inline FloatPointStruct GetPolygonCentroid(const std::vector<FloatPointStruct>& Points)
	{
		float SignedArea{};
		FloatPointStruct Centroid{};
		const std::size_t NumberOfPoints{ Points.size() };

		for (std::size_t i{}; i < NumberOfPoints; ++i)
		{
			const FloatPointStruct AreaPoint{ Points[(i + 1) & (NumberOfPoints - 1)].X, Points[(i + 1) & (NumberOfPoints - 1)].Y };
			const float Area{ Points[i].X * AreaPoint.Y - AreaPoint.X * Points[i].Y };
			SignedArea += Area;
			Centroid.X += (Points[i].X + AreaPoint.X) * Area;
			Centroid.Y += (Points[i].Y + AreaPoint.Y) * Area;
		}

		SignedArea *= 3.0F;

		return { Centroid.X / SignedArea, Centroid.Y / SignedArea };
	}

	inline bool PointInsidePolygon(const std::vector<FloatPointStruct>& Points, const FloatPointStruct& Point)
	{
		bool Result{};
		const std::size_t NumberOfPoints{ Points.size() };

		for (std::size_t i{}, j{ NumberOfPoints - 1 }; i < NumberOfPoints; j = i++)
		{
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
		const std::size_t NumberOfPoints{ Points.size() };

		// Exit early, if we have less than three points - then it´s not a polygon...
		if (NumberOfPoints < 3)
		{
			return;
		}

		for (std::size_t i{}; i < NumberOfPoints - 1; ++i)
		{
			Line(Texture, Points[i].X, Points[i].Y, Points[i + 1].X, Points[i + 1].Y, BorderColor);
		}

		// Connect last point with first point
		Line(Texture, Points[NumberOfPoints - 1].X, Points[NumberOfPoints - 1].Y, Points[0].X, Points[0].Y, BorderColor);
	}

	inline void FilledPolygon(TextureStruct& Texture, const std::vector<IntPointStruct>& Points, const std::int_fast32_t BorderColor, const std::int_fast32_t FillColor)
	{
		const std::size_t NumberOfPoints{ Points.size() };

		// Exit early, if we have less than three points - then it´s not a polygon...
		if (NumberOfPoints < 3)
		{
			return;
		}

		// We need to draw a polygon (lines only) with the fillcolor first, so we get some proper boundaries
		Polygon(Texture, Points, FillColor);

		// From now on, we work with floats to get the polygon centroid
		// Also we check if the found point is REALLY inside the polygon
		//
		// This works better when using floats rather than ints
		//

		std::vector<FloatPointStruct> FloatPoints{ NumberOfPoints };

		for (std::size_t i{}; i < NumberOfPoints; ++i)
		{
			FloatPoints[i] = { static_cast<float>(Points[i].X), static_cast<float>(Points[i].Y) };
		}

		if (const FloatPointStruct CentroidPoint{ GetPolygonCentroid(FloatPoints) }; PointInsidePolygon(FloatPoints, CentroidPoint))
		{
			// Now we can fill -> back to int!
			ScanlineFill(Texture, { static_cast<std::int_fast32_t>(CentroidPoint.X), static_cast<std::int_fast32_t>(CentroidPoint.Y) }, FillColor);
		}

		// Last step - draw a border if needed
		Polygon(Texture, Points, BorderColor);
	}


} // namespace lwmf
