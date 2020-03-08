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
#include <string>
#include <vector>
#include <algorithm>

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
	void ClearTexture(TextureStruct& Texture, std::int_fast32_t Color);

	//
	// Functions
	//

	inline void SetTextureMetrics(TextureStruct& Texture, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		if (Width <= 0 || Height <= 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, __LINE__, "Value for texture width or height is zero or negative! Check your parameters in lwmf::SetTextureMetrics()!");
		}

		Texture.Width = Width;
		Texture.Height = Height;
		Texture.WidthMid = Width >> 1;
		Texture.HeightMid = Height >> 1;
		Texture.Size = Width * Height;
	}

	inline void CreateTexture(TextureStruct& Texture, const std::int_fast32_t Width, const std::int_fast32_t Height, const std::int_fast32_t Color)
	{
		SetTextureMetrics(Texture, Width, Height);
		Texture.Pixels.resize(static_cast<std::size_t>(Texture.Size));
		ClearTexture(Texture, Color);
	}

	inline void CropTexture(TextureStruct& Texture, const std::int_fast32_t x, const std::int_fast32_t y, std::int_fast32_t Width, std::int_fast32_t Height)
	{
		if (x < 0 || y < 0 || x > Texture.Width || y > Texture.Height)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, __LINE__, "Value for texture x or y is out of range! Check your parameters used with lwmf::CropTexture()!");
		}

		// Make sure that Width and Height are within texture size limits!
		Width = std::clamp(Width, 1, Texture.Width);
		Height = std::clamp(Height, 1, Texture.Height);

		if (x + Width > Texture.Width || y + Height > Texture.Height)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, __LINE__, "Value for width or height is out of range! Check your parameters used with lwmf::CropTexture()!");
		}

		std::vector<std::int_fast32_t>TempBuffer(Width * Height);
		std::int_fast32_t SourceVerticalOffset{ y * Texture.Width };
		std::int_fast32_t DestVerticalOffset{};

		for (std::int_fast32_t i{}; i < Height; ++i)
		{
			for (std::int_fast32_t DestHorizontalOffset{}, SourceHorizontalOffset{ x }, j{}; j < Width; ++j)
			{
				TempBuffer[static_cast<std::size_t>(DestVerticalOffset) + static_cast<std::size_t>(DestHorizontalOffset)] = Texture.Pixels[static_cast<std::size_t>(SourceVerticalOffset) + static_cast<std::size_t>(SourceHorizontalOffset)];

				++DestHorizontalOffset;
				++SourceHorizontalOffset;
			}

			DestVerticalOffset += Width;
			SourceVerticalOffset += Texture.Width;
		}

		Texture.Pixels = std::move(TempBuffer);
		SetTextureMetrics(Texture, Width, Height);
	}

	inline void ResizeTexture(TextureStruct& Texture, const std::int_fast32_t TargetWidth, const std::int_fast32_t TargetHeight, const FilterModes FilterMode)
	{
		if (TargetWidth <= 0 || TargetHeight <= 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, __LINE__, "Value for texture width or height is zero or negative! Check your parameters in lwmf::ResizeTexture()!");
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
		}

		SetTextureMetrics(Texture, TargetWidth, TargetHeight);
		Texture.Pixels = std::move(TempBuffer);
	}

	inline void BlitTexture(const TextureStruct& SourceTexture, TextureStruct& TargetTexture, const std::int_fast32_t PosX, std::int_fast32_t PosY)
	{
		// Exit early if coords are out of visual boundaries
		if (PosX + SourceTexture.Width < 0 || PosY + SourceTexture.Height < 0 || PosX > TargetTexture.Width || PosY > TargetTexture.Height)
		{
			return;
		}

		// Case 1: Bitmap fits 1:1 into target texture
		if (PosX == 0 && PosY == 0 && TargetTexture.Width == SourceTexture.Width && TargetTexture.Height == SourceTexture.Height)
		{
			TargetTexture.Pixels = SourceTexture.Pixels;
			return;
		}

		// Case 2: Bitmap fits (= smaller than target texture and within boundaries)
		if (PosX >= 0 && PosY >= 0 && SourceTexture.Width + PosX <= TargetTexture.Width && SourceTexture.Height + PosY <= TargetTexture.Height)
		{
			for (std::int_fast32_t y{}; y < SourceTexture.Height; ++y, ++PosY)
			{
				const auto SourceY{ SourceTexture.Pixels.begin() + y * SourceTexture.Width };
				std::copy(SourceY, SourceY + SourceTexture.Width, TargetTexture.Pixels.begin() + PosY * TargetTexture.Width + PosX);
			}
		}
		// Case 3: Each pixel has to be checked if within boundaries
		else
		{
			for (std::int_fast32_t y{}; y < SourceTexture.Height; ++y, ++PosY)
			{
				const std::int_fast32_t DestOffset{ PosY * TargetTexture.Width };
				const std::int_fast32_t SrcOffset{ y * SourceTexture.Width };

				for (std::int_fast32_t x{}; x < SourceTexture.Width; ++x)
				{
					if (static_cast<std::uint_fast32_t>(PosX + x) < static_cast<std::uint_fast32_t>(TargetTexture.Width) && static_cast<std::uint_fast32_t>(PosY) < static_cast<std::uint_fast32_t>(TargetTexture.Height))
					{
						TargetTexture.Pixels[DestOffset + PosX + x] = SourceTexture.Pixels[SrcOffset + x];
					}
				}
			}
		}
	}

	inline void BlitTransTexture(const TextureStruct& SourceTexture, TextureStruct& TargetTexture, const std::int_fast32_t PosX, std::int_fast32_t PosY, const std::int_fast32_t TransparentColor)
	{
		// Exit early if coords are out of visual boundaries
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

			return;
		}

		// Case 2: Bitmap fits (= smaller than target texture and within boundaries)
		if (PosX >= 0 && PosY >= 0 && SourceTexture.Width + PosX <= TargetTexture.Width && SourceTexture.Height + PosY <= TargetTexture.Height)
		{
			for (std::int_fast32_t y{}; y < SourceTexture.Height; ++y, ++PosY)
			{
				const std::int_fast32_t DestOffset{ PosY * TargetTexture.Width };
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
			for (std::int_fast32_t y{}; y < SourceTexture.Height; ++y, ++PosY)
			{
				const std::int_fast32_t DestOffset{ PosY * TargetTexture.Width };
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

	inline void ClearTexture(TextureStruct& Texture, const std::int_fast32_t Color)
	{
		std::fill(Texture.Pixels.begin(), Texture.Pixels.end(), Color);
	}


} // namespace lwmf