/*
****************************************************
*                                                  *
* lwmf_ellipses - lightweight media framework      *
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

namespace lwmf
{


	void DrawEllipsePoints(TextureStruct& Texture, const IntPointStruct& Point, const IntPointStruct& Center, std::int_fast32_t Color, bool SafeFlag);
	void Ellipse(TextureStruct& Texture, std::int_fast32_t CenterX, std::int_fast32_t CenterY, std::int_fast32_t RadiusX, std::int_fast32_t RadiusY, std::int_fast32_t Color);

	//
	// Functions
	//

	inline void DrawEllipsePoints(TextureStruct& Texture, const IntPointStruct& Point, const IntPointStruct& Center, const std::int_fast32_t Color, const bool SafeFlag)
	{
		if (SafeFlag)
		{
			Texture.Pixels[((Center.Y + Point.Y) * Texture.Width) + (Center.X + Point.X)] = Color;
			Texture.Pixels[((Center.Y + Point.Y) * Texture.Width) + (Center.X - Point.X)] = Color;
			Texture.Pixels[((Center.Y - Point.Y) * Texture.Width) + (Center.X + Point.X)] = Color;
			Texture.Pixels[((Center.Y - Point.Y) * Texture.Width) + (Center.X - Point.X)] = Color;
		}
		else
		{
			SetPixelSafe(Texture, Center.X + Point.X, Center.Y + Point.Y, Color);
			SetPixelSafe(Texture, Center.X - Point.X, Center.Y + Point.Y, Color);
			SetPixelSafe(Texture, Center.X + Point.X, Center.Y - Point.Y, Color);
			SetPixelSafe(Texture, Center.X - Point.X, Center.Y - Point.Y, Color);
		}
	}

	inline void Ellipse(TextureStruct& Texture, const std::int_fast32_t CenterX, const std::int_fast32_t CenterY, const std::int_fast32_t RadiusX, const std::int_fast32_t RadiusY, const std::int_fast32_t Color)
	{
		// Exit early if ellipse would not be visible (to small or coords out of texture boundaries)
		if ((RadiusX <= 0 && RadiusY <= 0) || CenterX + RadiusX < 0 || CenterX - RadiusX > Texture.Width || CenterY + RadiusY < 0 || CenterY - RadiusY > Texture.Height)
		{
			return;
		}

		// if complete ellipse is within texture boundaries, there is no reason to use SetPixelSafe...
		const bool SafeFlag{ ((CenterX - RadiusX >= 0 && CenterX + RadiusX < Texture.Width) && (CenterY - RadiusY >= 0 && CenterY + RadiusY < Texture.Height)) ? true : false };

		const IntPointStruct RadiusTemp{ RadiusX * RadiusX , RadiusY * RadiusY };
		const IntPointStruct TwoRadiusTemp{ RadiusTemp.X << 1, RadiusTemp.Y << 1 };
		IntPointStruct Point{ 0, RadiusY };
		IntPointStruct Temp{ TwoRadiusTemp.Y * Point.X, TwoRadiusTemp.X * Point.Y };
		float p1{ static_cast<float>(RadiusTemp.Y - (RadiusTemp.X * RadiusY) + (RadiusTemp.X >> 2)) };

		while (Temp.X <= Temp.Y)
		{
			DrawEllipsePoints(Texture, Point, { CenterX, CenterY }, Color, SafeFlag);

			++Point.X;

			if (p1 < 0.0F)
			{
				Temp.X = TwoRadiusTemp.Y * Point.X;
				p1 += static_cast<float>(Temp.X + RadiusTemp.Y);
			}
			else
			{
				--Point.Y;
				Temp = { TwoRadiusTemp.Y * Point.X, TwoRadiusTemp.X * Point.Y };
				p1 += static_cast<float>(Temp.X - Temp.Y + RadiusTemp.Y);
			}

			DrawEllipsePoints(Texture, Point, { CenterX, CenterY }, Color, SafeFlag);
		}

		float p2{ (static_cast<float>(RadiusTemp.Y) * (static_cast<float>(Point.X) + 0.5F) * (static_cast<float>(Point.X) + 0.5F)) + static_cast<float>((RadiusTemp.X * (Point.Y - 1) * (Point.Y - 1)) - (RadiusTemp.X * RadiusTemp.Y)) };
		Temp = { 0, 0 };

		while (Point.Y >= 0)
		{
			DrawEllipsePoints(Texture, Point, { CenterX, CenterY }, Color, SafeFlag);

			--Point.Y;

			if (p2 < 0.0F)
			{
				++Point.X;
				Temp = { TwoRadiusTemp.Y * Point.X, TwoRadiusTemp.X * Point.Y };;
				p2 += static_cast<float>(Temp.X - Temp.Y + RadiusTemp.X);
			}
			else
			{
				Temp.Y = TwoRadiusTemp.X * Point.Y;
				p2 -= static_cast<float>(Temp.Y + RadiusTemp.X);
			}

			DrawEllipsePoints(Texture, Point, { CenterX, CenterY }, Color, SafeFlag);
		}
	}


} // namespace lwmf