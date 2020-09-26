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
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>

#include "lwmf_general.hpp"
#include "lwmf_logging.hpp"

namespace lwmf
{


	struct TextureStruct final
	{
		std::vector<std::int_fast32_t> Pixels{};
		std::int_fast32_t Size{};
		std::int_fast32_t Width{};
		std::int_fast32_t Height{};
		std::int_fast32_t WidthMid{};
		std::int_fast32_t HeightMid{};
	};

	enum class FilterModes
	{
		NEAREST,
		BILINEAR
	};

	void SetTextureMetrics(TextureStruct& Texture, std::int_fast32_t Width, std::int_fast32_t Height);
	void CreateTexture(TextureStruct& Texture, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Color);
	void CropTexture(TextureStruct& Texture, std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Width, std::int_fast32_t Height);
	void ResizeTexture(TextureStruct& Texture, std::int_fast32_t TargetWidth, std::int_fast32_t TargetHeight, FilterModes FilterMode);
	void BlitTexture(const TextureStruct& SourceTexture, TextureStruct& TargetTexture, std::int_fast32_t PosX, std::int_fast32_t PosY);
	void BlitTransTexture(const TextureStruct& SourceTexture, TextureStruct& TargetTexture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t TransparentColor);
	void RotateTexture(TextureStruct& Texture, std::int_fast32_t RotCenterX, std::int_fast32_t RotCenterY, float Angle);
	void ClearTexture(TextureStruct& Texture, std::int_fast32_t Color);

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
	}

	inline void CreateTexture(TextureStruct& Texture, const std::int_fast32_t Width, const std::int_fast32_t Height, const std::int_fast32_t Color)
	{
		// Exit early if texture size would be zero
		if (Width <= 0 || Height <= 0)
		{
			return;
		}

		SetTextureMetrics(Texture, Width, Height);
		Texture.Pixels.clear();
		Texture.Pixels.shrink_to_fit();
		Texture.Pixels.resize(static_cast<std::size_t>(Texture.Size), Color);
	}

	inline void CropTexture(TextureStruct& Texture, const std::int_fast32_t x, const std::int_fast32_t y, std::int_fast32_t Width, std::int_fast32_t Height)
	{
		// Exit early if crop start position would be out of texture boundaries
		if ((static_cast<std::uint_fast32_t>(x) > static_cast<std::uint_fast32_t>(Texture.Width) || static_cast<std::uint_fast32_t>(y) > static_cast<std::uint_fast32_t>(Texture.Height)))
		{
			return;
		}

		// Make sure that Width and Height are within texture size limits
		Width = std::clamp(Width, 1, Texture.Width);
		Height = std::clamp(Height, 1, Texture.Height);

		// Exit early if cropped rectangle would be out of texture boundaries
		if (x + Width > Texture.Width || y + Height > Texture.Height)
		{
			return;
		}

		std::vector<std::int_fast32_t>TempBuffer(Width * Height);
		std::int_fast32_t SrcVerticalOffset{ y * Texture.Width };
		std::int_fast32_t DestVerticalOffset{};

		for (std::int_fast32_t i{}; i < Height; ++i)
		{
			for (std::int_fast32_t DestHorizontalOffset{}, SourceHorizontalOffset{ x }, j{}; j < Width; ++j)
			{
				TempBuffer[static_cast<std::size_t>(DestVerticalOffset) + static_cast<std::size_t>(DestHorizontalOffset)] = Texture.Pixels[static_cast<std::size_t>(SrcVerticalOffset) + static_cast<std::size_t>(SourceHorizontalOffset)];

				++DestHorizontalOffset;
				++SourceHorizontalOffset;
			}

			DestVerticalOffset += Width;
			SrcVerticalOffset += Texture.Width;
		}

		Texture.Pixels = std::move(TempBuffer);
		SetTextureMetrics(Texture, Width, Height);
	}

	inline void ResizeTexture(TextureStruct& Texture, const std::int_fast32_t TargetWidth, const std::int_fast32_t TargetHeight, const FilterModes FilterMode)
	{
		// Exit early if texture size would be zero
		if (TargetWidth <= 0 || TargetHeight <= 0)
		{
			return;
		}

		std::vector<std::int_fast32_t> TempBuffer(TargetWidth * TargetHeight);

		switch (FilterMode)
		{
			case FilterModes::NEAREST:
			{
				const IntPointStruct Ratio{ ((Texture.Width << 16) / TargetWidth) + 1, ((Texture.Height << 16) / TargetHeight) + 1 };

				for (std::int_fast32_t Offset{}, i{}; i < TargetHeight; ++i)
				{
					const std::int_fast32_t TempY{ ((i * Ratio.Y) >> 16) * Texture.Width };

					for (std::int_fast32_t j{}; j < TargetWidth; ++j)
					{
						TempBuffer[static_cast<std::size_t>(Offset++)] = Texture.Pixels[static_cast<std::size_t>(TempY) + static_cast<std::size_t>(((j * Ratio.X) >> 16))];
					}
				}
				break;
			}
			case FilterModes::BILINEAR:
			{
				const FloatPointStruct Ratio{ static_cast<float>((Texture.Width - 1)) / TargetWidth, static_cast<float>((Texture.Height - 1)) / TargetHeight };

				for (std::int_fast32_t Offset{}, i{}; i < TargetHeight; ++i)
				{
					const float TempRatioY{ Ratio.Y * i };
					const std::int_fast32_t PosY{ static_cast<std::int_fast32_t>(TempRatioY) };
					const std::int_fast32_t TempY{ PosY * Texture.Width };
					const float Height{ TempRatioY - PosY };

					for (std::int_fast32_t j{}; j < TargetWidth; ++j)
					{
						const float TempRatioX{ Ratio.X * j };
						const std::int_fast32_t PosX{ static_cast<std::int_fast32_t>(TempRatioX) };
						const std::int_fast32_t Index{ TempY + PosX };

						const std::int_fast32_t P1{ Texture.Pixels[static_cast<std::size_t>(Index)] };
						const std::int_fast32_t P2{ Texture.Pixels[static_cast<std::size_t>(Index) + 1] };
						const std::int_fast32_t P3{ Texture.Pixels[static_cast<std::size_t>(Index) + static_cast<std::size_t>(Texture.Width)] };
						const std::int_fast32_t P4{ Texture.Pixels[static_cast<std::size_t>(Index) + static_cast<std::size_t>(Texture.Width) + 1] };

						const float Width{ TempRatioX - PosX };
						const float t1{ (1.0F - Width) * (1.0F - Height) };
						const float t2{ Width * (1.0F - Height) };
						const float t3{ Height * (1.0F - Width) };
						const float t4{ Width * Height };

						TempBuffer[static_cast<std::size_t>(Offset++)] = RGBAtoINT(
							static_cast<std::int_fast32_t>((P1 & 255) * t1 + (P2 & 255) * t2 + (P3 & 255) * t3 + (P4 & 255) * t4),
							static_cast<std::int_fast32_t>(((P1 >> 8) & 255) * t1 + ((P2 >> 8) & 255) * t2 + ((P3 >> 8) & 255) * t3 + ((P4 >> 8) & 255) * t4),
							static_cast<std::int_fast32_t>(((P1 >> 16) & 255) * t1 + ((P2 >> 16) & 255) * t2 + ((P3 >> 16) & 255) * t3 + ((P4 >> 16) & 255) * t4)
							, AMask);
					}
				}
				break;
			}
			default: {}
		}

		Texture.Pixels = std::move(TempBuffer);
		SetTextureMetrics(Texture, TargetWidth, TargetHeight);
	}

	inline void BlitTexture(const TextureStruct& SourceTexture, TextureStruct& TargetTexture, const std::int_fast32_t PosX, const std::int_fast32_t PosY)
	{
		// Exit early if coords are out of target texture boundaries
		if (PosX + SourceTexture.Width < 0 || PosY + SourceTexture.Height < 0 || PosX > TargetTexture.Width || PosY > TargetTexture.Height)
		{
			return;
		}

		// Case 1: Bitmap fits 1:1 into target texture
		if (PosX == 0 && PosY == 0 && TargetTexture.Width == SourceTexture.Width && TargetTexture.Height == SourceTexture.Height)
		{
			TargetTexture.Pixels = SourceTexture.Pixels;
		}
		// All the rest: Clip position and width/height to fit within boundaries
		else
		{
			const std::int_fast32_t StartX{ (PosX < 0 && (std::abs(0 - PosX) <= SourceTexture.Width)) ? std::abs(0 - PosX) : 0 };
			const std::int_fast32_t TargetHeight{ (PosY + SourceTexture.Height >= TargetTexture.Height) ? TargetTexture.Height - PosY : SourceTexture.Height };
			const std::int_fast32_t TargetWidth{ (PosX + SourceTexture.Width >= TargetTexture.Width) ? TargetTexture.Width - PosX : SourceTexture.Width };

			for (std::int_fast32_t y{}, TempPosY{ PosY }; y < TargetHeight; ++y, ++TempPosY)
			{
				if (static_cast<std::uint_fast32_t>(TempPosY) < static_cast<std::uint_fast32_t>(TargetTexture.Height))
				{
					const auto SrcY{ SourceTexture.Pixels.begin() + y * SourceTexture.Width + StartX};
					std::copy(SrcY, SrcY + TargetWidth - StartX, TargetTexture.Pixels.begin() + TempPosY * TargetTexture.Width + PosX + StartX);
				}
			}
		}
	}

	inline void BlitTransTexture(const TextureStruct& SourceTexture, TextureStruct& TargetTexture, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t TransparentColor)
	{
		// Exit early if coords are out of target texture boundaries
		if (PosX + SourceTexture.Width < 0 || PosY + SourceTexture.Height < 0 || PosX > TargetTexture.Width || PosY > TargetTexture.Height)
		{
			return;
		}

		// Case 1: Bitmap fits 1:1 into target texture
		if (PosX == 0 && PosY == 0 && TargetTexture.Width == SourceTexture.Width && TargetTexture.Height == SourceTexture.Height)
		{
			for (std::int_fast32_t i{}; i < SourceTexture.Size; ++i)
			{
				if (SourceTexture.Pixels[i] != TransparentColor)
				{
					TargetTexture.Pixels[i] = SourceTexture.Pixels[i];
				}
			}
		}
		// Case 2: Bitmap fits (= smaller than target texture and within boundaries)
		else if (PosX >= 0 && PosY >= 0 && SourceTexture.Width + PosX <= TargetTexture.Width && SourceTexture.Height + PosY <= TargetTexture.Height)
		{
			for (std::int_fast32_t y{}, TempPosY{ PosY }; y < SourceTexture.Height; ++y, ++TempPosY)
			{
				const std::int_fast32_t DestOffset{ TempPosY * TargetTexture.Width };
				const std::int_fast32_t SrcOffset{ y * SourceTexture.Width };

				for (std::int_fast32_t x{}; x < SourceTexture.Width; ++x)
				{
					if (SourceTexture.Pixels[SrcOffset + x] != TransparentColor)
					{
						TargetTexture.Pixels[DestOffset + PosX + x] = SourceTexture.Pixels[SrcOffset + x];
					}
				}
			}
		}
		// Case 3: Each pixel has to be checked if within boundaries
		else
		{
			for (std::int_fast32_t y{}, TempPosY{ PosY }; y < SourceTexture.Height; ++y, ++TempPosY)
			{
				const std::int_fast32_t DestOffset{ TempPosY * TargetTexture.Width };
				const std::int_fast32_t SrcOffset{ y * SourceTexture.Width };

				for (std::int_fast32_t x{}; x < SourceTexture.Width; ++x)
				{
					if (static_cast<std::uint_fast32_t>(PosX + x) < static_cast<std::uint_fast32_t>(TargetTexture.Width) && static_cast<std::uint_fast32_t>(PosY) < static_cast<std::uint_fast32_t>(TargetTexture.Height))
					{
						if (SourceTexture.Pixels[SrcOffset + x] != TransparentColor)
						{
							TargetTexture.Pixels[DestOffset + PosX + x] = SourceTexture.Pixels[SrcOffset + x];
						}
					}
				}
			}
		}
	}

	inline void RotateTexture(TextureStruct& Texture, const std::int_fast32_t RotCenterX, const std::int_fast32_t RotCenterY, const float Angle)
	{
		// if Angle ist zero degrees, we can exit early - nothing to do!
		if (Angle < FLT_EPSILON)
		{
			return;
		}

		std::vector<std::int_fast32_t> TempBuffer(Texture.Size);

		const float c{ std::cosf(Angle) };
		const float s{ std::sinf(Angle) };

		for (std::int_fast32_t y{}; y < Texture.Height; ++y)
		{
			const float fy{ static_cast<float>(y - RotCenterY) };
			const std::int_fast32_t DestOffset{ y * Texture.Width };

			for (std::int_fast32_t x{}; x < Texture.Width; ++x)
			{
				const float fx{ static_cast<float>(x - RotCenterX) };
				const std::int_fast32_t SrcX{ static_cast<std::int_fast32_t>((+(fx * c) + (fy * s))) + RotCenterX };
				const std::int_fast32_t SrcY{ static_cast<std::int_fast32_t>((-(fx * s) + (fy * c))) + RotCenterY };

				if ((SrcX >= 0) && (SrcX < Texture.Width) && (SrcY >= 0) && (SrcY < Texture.Height))
				{
					TempBuffer[DestOffset + x] = Texture.Pixels[SrcY * Texture.Width + SrcX];
				}
			}
		}

		Texture.Pixels = std::move(TempBuffer);
	}

	inline void ClearTexture(TextureStruct& Texture, const std::int_fast32_t Color)
	{
		std::fill(Texture.Pixels.begin(), Texture.Pixels.end(), Color);
	}


} // namespace lwmf