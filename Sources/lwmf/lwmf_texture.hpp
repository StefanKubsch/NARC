/*
***************************************************
*                                                 *
* lwmf_texture - lightweight media framework      *
*                                                 *
* (C) 2019 - present by Stefan Kubsch             *
*                                                 *
***************************************************
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <algorithm>

#include "lwmf_general.hpp"

namespace lwmf
{

	struct TextureStruct final
	{
		std::vector<std::int_fast32_t> Pixels;
		std::int_fast32_t Width{};
		std::int_fast32_t Height{};
		std::int_fast32_t WidthMid{};
		std::int_fast32_t HeightMid{};
		std::int_fast32_t Size{};
		size_t Stride{};
	};

	void SetTextureMetrics(TextureStruct& Texture, std::int_fast32_t Width, std::int_fast32_t Height);
	void CropTexture(TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Width, std::int_fast32_t Height);
	void ResizeTexture(TextureStruct& Texture, std::int_fast32_t TargetWidth, std::int_fast32_t TargetHeight, std::int_fast32_t FilterMode);
	void BlitTexture(const TextureStruct& SourceTexture, TextureStruct& TargetTexture, std::int_fast32_t PosX, std::int_fast32_t PosY);
	void BlitTransTexture(const TextureStruct& SourceTexture, TextureStruct& TargetTexture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t TransparentColor);
	void ClearTexture(TextureStruct& Texture, std::int_fast32_t Color);

	//
	// Variables and constants
	//

	// Define Filtermodes for ResizeBMP
	constexpr std::int_fast32_t NEAREST{ 1 };
	constexpr std::int_fast32_t BILINEAR{ 2 };

	//
	// Functions
	//

	inline void SetTextureMetrics(TextureStruct& Texture, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		Texture.Width = Width;
		Texture.Height = Height;
		Texture.WidthMid = Width >> 1;
		Texture.HeightMid = Height >> 1;
		Texture.Size = Width * Height;
		Texture.Stride = static_cast<size_t>(Width) << 2;
	}

	inline void CropTexture(TextureStruct& Texture, const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		std::vector<std::int_fast32_t>TempBuffer(Width * Height);
		std::int_fast32_t SourceVerticalOffset{ y * Texture.Width };
		std::int_fast32_t DestVerticalOffset{};
		std::int_fast32_t DestTotalOffset{};
		std::int_fast32_t SourceTotalOffset{};

		for (std::int_fast32_t i{}; i < Height; ++i)
		{
			std::int_fast32_t DestHorizontalOffset{};
			std::int_fast32_t SourceHorizontalOffset{ x };

			for (std::int_fast32_t j{}; j < Width; ++j)
			{
				DestTotalOffset = DestVerticalOffset + DestHorizontalOffset;
				SourceTotalOffset = SourceVerticalOffset + SourceHorizontalOffset;
				TempBuffer[DestTotalOffset] = Texture.Pixels[SourceTotalOffset];
				++DestHorizontalOffset;
				++SourceHorizontalOffset;
			}

			DestVerticalOffset += Width;
			SourceVerticalOffset += Texture.Width;
		}

		Texture.Pixels = std::move(TempBuffer);
		SetTextureMetrics(Texture, Width, Height);
	}

	inline void ResizeTexture(TextureStruct& Texture, const std::int_fast32_t TargetWidth, const std::int_fast32_t TargetHeight, const std::int_fast32_t FilterMode)
	{
		std::vector<std::int_fast32_t> TempBuffer(TargetWidth * TargetHeight);

		if (FilterMode == NEAREST)
		{
			const IntPointStruct Ratio{ ((Texture.Width << 16) / TargetWidth) + 1, ((Texture.Height << 16) / TargetHeight) + 1 };

			for (std::int_fast32_t Offset{}, i{}; i < TargetHeight; ++i)
			{
				const std::int_fast32_t TempY{ ((i * Ratio.Y) >> 16) * Texture.Width };

				for (std::int_fast32_t j{}; j < TargetWidth; ++j)
				{
					TempBuffer[Offset++] = Texture.Pixels[TempY + ((j * Ratio.X) >> 16)];
				}
			}
		}
		else if (FilterMode == BILINEAR)
		{
			const FloatPointStruct Ratio{ static_cast<float>((Texture.Width - 1)) / TargetWidth, static_cast<float>((Texture.Height - 1)) / TargetHeight };

			for (std::int_fast32_t Offset{}, i{}; i < TargetHeight; ++i)
			{
				const std::int_fast32_t PosY{ static_cast<std::int_fast32_t>(Ratio.Y * i) };
				const std::int_fast32_t TempY{ PosY * Texture.Width };
				const float Height{ Ratio.Y * i - PosY };

				for (std::int_fast32_t j{}; j < TargetWidth; ++j)
				{
					const std::int_fast32_t PosX{ static_cast<std::int_fast32_t>(Ratio.X * j) };
					const std::int_fast32_t Index{ TempY + PosX };
					const std::int_fast32_t P1{ Texture.Pixels[Index] };
					const std::int_fast32_t P2{ Texture.Pixels[Index + 1] };
					const std::int_fast32_t P3{ Texture.Pixels[Index + Texture.Width] };
					const std::int_fast32_t P4{ Texture.Pixels[Index + Texture.Width + 1] };

					const float Width{ Ratio.X * j - PosX };
					const float t1{ (1.0F - Width) * (1.0F - Height) };
					const float t2{ Width * (1.0F - Height) };
					const float t3{ Height * (1.0F - Width) };
					const float t4{ Width * Height };

					TempBuffer[Offset++] = RGBAtoINT(
						static_cast<std::int_fast32_t>((P1 & 255) * t1 + (P2 & 255) * t2 + (P3 & 255) * t3 + (P4 & 255) * t4),
						static_cast<std::int_fast32_t>(((P1 >> 8) & 255) * t1 + ((P2 >> 8) & 255) * t2 + ((P3 >> 8) & 255) * t3 + ((P4 >> 8) & 255) * t4),
						static_cast<std::int_fast32_t>(((P1 >> 16) & 255) * t1 + ((P2 >> 16) & 255) * t2 + ((P3 >> 16) & 255) * t3 + ((P4 >> 16) & 255) * t4)
						, AMask);
				}
			}
		}

		Texture.Pixels = std::move(TempBuffer);
		SetTextureMetrics(Texture, TargetWidth, TargetHeight);
	}

	inline void BlitTexture(const TextureStruct& SourceTexture, TextureStruct& TargetTexture, const std::int_fast32_t PosX, std::int_fast32_t PosY)
	{
		if (PosX == 0 && PosY == 0 && TargetTexture.Width == SourceTexture.Width && TargetTexture.Height == SourceTexture.Height)
		{
			std::memcpy(TargetTexture.Pixels.data(), SourceTexture.Pixels.data(), SourceTexture.Stride * static_cast<size_t>(SourceTexture.Height));
		}
		else
		{
			for (std::int_fast32_t y{}; y < SourceTexture.Height; ++y, ++PosY)
			{
				std::memcpy(TargetTexture.Pixels.data() + static_cast<size_t>(PosY) * static_cast<size_t>(SourceTexture.Width) + static_cast<size_t>(PosX), SourceTexture.Pixels.data() + static_cast<size_t>(y) * static_cast<size_t>(SourceTexture.Width), SourceTexture.Stride);
			}
		}
	}

	inline void BlitTransTexture(const TextureStruct& SourceTexture, TextureStruct& TargetTexture, const std::int_fast32_t PosX, std::int_fast32_t PosY, const std::int_fast32_t TransparentColor)
	{
		if (PosX == 0 && PosY == 0 && TargetTexture.Width == SourceTexture.Width && TargetTexture.Height == SourceTexture.Height)
		{
			for (std::int_fast32_t i{}; i < SourceTexture.Size; ++i)
			{
				if (const std::int_fast32_t Color{ SourceTexture.Pixels[i] }; Color != TransparentColor)
				{
					TargetTexture.Pixels[i] = Color;
				}
			}
		}
		else
		{
			for (std::int_fast32_t y{}; y < SourceTexture.Height; ++y, ++PosY)
			{
				const std::int_fast32_t DestOffset{ PosY * TargetTexture.Width };
				const std::int_fast32_t SrcOffset{ y * SourceTexture.Width };

				for (std::int_fast32_t x{}; x < SourceTexture.Width; ++x)
				{
					if (const std::int_fast32_t Color{ SourceTexture.Pixels[SrcOffset + x] }; Color != TransparentColor)
					{
						TargetTexture.Pixels[DestOffset + PosX + x] = Color;
					}
				}
			}
		}
	}

	inline void ClearTexture(TextureStruct& Texture, const std::int_fast32_t Color)
	{
		std::fill(Texture.Pixels.begin(), Texture.Pixels.end(), Color);
	}


} // namespace lwmf