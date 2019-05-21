/*
***************************************************
*                                                 *
* lwmf_bmp - lightweight media framework          *
*                                                 *
* (C) 2019 - present by Stefan Kubsch             *
*                                                 *
***************************************************
*/

#pragma once

#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <fstream>

#include "lwmf_general.hpp"
#include "lwmf_color.hpp"
#include "lwmf_pixelbuffer.hpp"

namespace lwmf
{


	struct BitmapStruct final
	{
		std::vector<std::int_fast32_t> BitmapData;
		std::int_fast32_t Width{};
		std::int_fast32_t Height{};
		size_t Size{};
		size_t Stride{};
	};

	void SetBMPMetrics(BitmapStruct& Bitmap, std::int_fast32_t Width, std::int_fast32_t Height);
	void LoadBMP(BitmapStruct& Bitmap, const std::string& Filename);
	void CropBMP(BitmapStruct& Bitmap, std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Width, std::int_fast32_t Height);
	void ResizeBMP(BitmapStruct& Bitmap, std::int_fast32_t TargetWidth, std::int_fast32_t TargetHeight, std::int_fast32_t FilterMode);
	void BlitBMP(const BitmapStruct& Bitmap, std::int_fast32_t PosX, std::int_fast32_t PosY);
	void BlitTransBMP(const BitmapStruct& Bitmap, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t TransparentColor);

	//
	// Variables and constants
	//

	// Define Filtermodes for ResizeBMP
	constexpr std::int_fast32_t NEAREST{ 1 };
	constexpr std::int_fast32_t BILINEAR{ 2 };

	//
	// Functions
	//

	inline void SetBMPMetrics(BitmapStruct& Bitmap, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		Bitmap.Height = Height;
		Bitmap.Width = Width;
		Bitmap.Size = static_cast<size_t>(Width) * static_cast<size_t>(Height);
		Bitmap.Stride = static_cast<size_t>(Width) << 2;
	}

	inline void LoadBMP(BitmapStruct& Bitmap, const std::string& Filename)
	{
		std::ifstream File(Filename, std::ios::binary);

		if (!File)
		{
			exit(-1);
		}

		std::vector<unsigned char> FileHeaderBuffer(sizeof(BITMAPFILEHEADER));
		std::vector<unsigned char> InfoHeaderBuffer(sizeof(BITMAPINFOHEADER));

		File.read(reinterpret_cast<char*>(FileHeaderBuffer.data()), sizeof(BITMAPFILEHEADER));
		File.read(reinterpret_cast<char*>(InfoHeaderBuffer.data()), sizeof(BITMAPINFOHEADER));

		const BITMAPFILEHEADER* BMPHeader{ reinterpret_cast<BITMAPFILEHEADER*>(FileHeaderBuffer.data()) };
		const BITMAPINFOHEADER* BMPInfo{ reinterpret_cast<BITMAPINFOHEADER*>(InfoHeaderBuffer.data()) };

		std::vector<char> InputBuffer(BMPInfo->biSizeImage);
		File.seekg(BMPHeader->bfOffBits);
		File.read(InputBuffer.data(), BMPInfo->biSizeImage);

		SetBMPMetrics(Bitmap, BMPInfo->biWidth, BMPInfo->biHeight);
		Bitmap.BitmapData.resize(Bitmap.Size);

		for (std::int_fast32_t Offset{}, y{ Bitmap.Height - 1 }; y >= 0; --y)
		{
			const std::int_fast32_t TempY{ y * Bitmap.Width };

			for (std::int_fast32_t x{}; x < Bitmap.Width; ++x)
			{
				const std::int_fast32_t TempPos{ 3 * (TempY + x) };

				Bitmap.BitmapData[Offset++] = RGBAtoINT(InputBuffer[TempPos + 2] & 255, InputBuffer[TempPos + 1] & 255, InputBuffer[TempPos] & 255, 255);
			}
		}
	}

	inline void CropBMP(BitmapStruct& Bitmap, const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		std::vector<std::int_fast32_t>TempBuffer(Width * Height);
		std::int_fast32_t SourceVerticalOffset{ y * Bitmap.Width };
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
				TempBuffer[DestTotalOffset] = Bitmap.BitmapData[SourceTotalOffset];
				++DestHorizontalOffset;
				++SourceHorizontalOffset;
			}

			DestVerticalOffset += Width;
			SourceVerticalOffset += Bitmap.Width;
		}

		Bitmap.BitmapData = std::move(TempBuffer);
		SetBMPMetrics(Bitmap, Width, Height);
	}

	inline void ResizeBMP(BitmapStruct& Bitmap, const std::int_fast32_t TargetWidth, const std::int_fast32_t TargetHeight, const std::int_fast32_t FilterMode)
	{
		std::vector<std::int_fast32_t> TempBuffer(TargetWidth * TargetHeight);

		if (FilterMode == NEAREST)
		{
			const IntPointStruct Ratio{ ((Bitmap.Width << 16) / TargetWidth) + 1, ((Bitmap.Height << 16) / TargetHeight) + 1 };

			for (std::int_fast32_t Offset{}, i{}; i < TargetHeight; ++i)
			{
				const std::int_fast32_t TempY{ ((i * Ratio.y) >> 16) * Bitmap.Width };

				for (std::int_fast32_t j{}; j < TargetWidth; ++j)
				{
					TempBuffer[Offset++] = Bitmap.BitmapData[TempY + ((j * Ratio.x) >> 16)];
				}
			}
		}
		else if (FilterMode == BILINEAR)
		{
			const FloatPointStruct Ratio{ static_cast<float>((Bitmap.Width - 1)) / TargetWidth, static_cast<float>((Bitmap.Height - 1)) / TargetHeight };

			for (std::int_fast32_t Offset{}, i{}; i < TargetHeight; ++i)
			{
				const std::int_fast32_t PosY{ static_cast<std::int_fast32_t>(Ratio.y * i) };
				const std::int_fast32_t TempY{ PosY * Bitmap.Width };
				const float Height{ Ratio.y * i - PosY };

				for (std::int_fast32_t j{}; j < TargetWidth; ++j)
				{
					const std::int_fast32_t PosX{ static_cast<std::int_fast32_t>(Ratio.x * j) };
					const std::int_fast32_t Index{ TempY + PosX };
					const std::int_fast32_t P1{ Bitmap.BitmapData[Index] };
					const std::int_fast32_t P2{ Bitmap.BitmapData[Index + 1] };
					const std::int_fast32_t P3{ Bitmap.BitmapData[Index + Bitmap.Width] };
					const std::int_fast32_t P4{ Bitmap.BitmapData[Index + Bitmap.Width + 1] };

					const float Width{ Ratio.x * j - PosX };
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

		Bitmap.BitmapData = std::move(TempBuffer);
		SetBMPMetrics(Bitmap, TargetWidth, TargetHeight);
	}

	inline void BlitBMP(const BitmapStruct& Bitmap, const std::int_fast32_t PosX, std::int_fast32_t PosY)
	{
		if (PosX == 0 && PosY == 0 && ViewportWidth == Bitmap.Width && ViewportHeight == Bitmap.Height)
		{
			std::memcpy(PixelBuffer.data(), Bitmap.BitmapData.data(), Bitmap.Stride * static_cast<size_t>(Bitmap.Height));
		}
		else
		{
			for (std::int_fast32_t y{}; y < Bitmap.Height; ++y, ++PosY)
			{
				std::memcpy(PixelBuffer.data() + PosY * ViewportWidth + PosX, Bitmap.BitmapData.data() + y * Bitmap.Width, Bitmap.Stride);
			}
		}
	}

	inline void BlitTransBMP(const BitmapStruct& Bitmap, const std::int_fast32_t PosX, std::int_fast32_t PosY, const std::int_fast32_t TransparentColor)
	{
		if (PosX == 0 && PosY == 0 && ViewportWidth == Bitmap.Width && ViewportHeight == Bitmap.Height)
		{
			for (std::int_fast32_t i{}; i < Bitmap.Size; ++i)
			{
				if (const std::int_fast32_t Color{ Bitmap.BitmapData[i] }; Color != TransparentColor)
				{
					PixelBuffer[i] = Color;
				}
			}
		}
		else
		{
			for (std::int_fast32_t y{}; y < Bitmap.Height; ++y, ++PosY)
			{
				const std::int_fast32_t BufferPos{ PosY * ViewportWidth };

				for (std::int_fast32_t x{}; x < Bitmap.Width; ++x)
				{
					if (const std::int_fast32_t Color{ Bitmap.BitmapData[y * Bitmap.Width + x] }; Color != TransparentColor)
					{
						PixelBuffer[BufferPos + PosX + x] = Color;
					}
				}
			}
		}
	}


} // namespace lwmf