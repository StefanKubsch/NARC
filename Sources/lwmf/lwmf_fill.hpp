/*
****************************************************
*                                                  *
* lwmf_fill - lightweight media framework          *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <cstdint>
#include <vector>
#include <queue>

#include "lwmf_general.hpp"
#include "lwmf_texture.hpp"
#include "lwmf_pixel.hpp"

namespace lwmf
{


	void ScanlineFill(TextureStruct& Texture, const IntPointStruct& CenterPoint, std::int_fast32_t FillColor);

	//
	// Functions
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
				else if (Above && Points.Y > 0 && Texture.Pixels[(Points.Y - 1) * Texture.Width + x1] == FillColor)
				{
					Above = false;
				}

				if (!Below && Points.Y < Texture.Height - 1 && Texture.Pixels[(Points.Y + 1) * Texture.Width + x1] != FillColor)
				{
					Stack.push_back({ x1, Points.Y + 1 });
					Below = true;
				}
				else if (Below && Points.Y < Texture.Height - 1 && Texture.Pixels[(Points.Y + 1) * Texture.Width + x1] == FillColor)
				{
					Below = false;
				}

				++x1;
			}
		}
	}


} // namespace lwmf