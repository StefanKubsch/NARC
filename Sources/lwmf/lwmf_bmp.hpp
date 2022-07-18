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

#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <fstream>

#include "lwmf_logging.hpp"
#include "lwmf_color.hpp"
#include "lwmf_texture.hpp"

namespace lwmf
{


	void LoadBMP(TextureStruct& Texture, const std::string& Filename);

	//
	// Functions
	//

	inline void LoadBMP(TextureStruct& Texture, const std::string& Filename)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, "Load BMP file " + Filename + "...");

		std::ifstream File(Filename, std::ios::in | std::ios::binary);

		if (File.fail())
		{
			std::array<char, 100> ErrorMessage{};
			strerror_s(ErrorMessage.data(), 100, errno);

			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, __LINE__, "Error loading " + Filename + ": " + std::string(ErrorMessage.data()));
		}
		else
		{
			std::array<unsigned char, sizeof(BITMAPFILEHEADER)> FileHeaderBuffer{};
			File.read(reinterpret_cast<char*>(FileHeaderBuffer.data()), sizeof(BITMAPFILEHEADER));

			std::array<unsigned char, sizeof(BITMAPINFOHEADER)> InfoHeaderBuffer{};
			File.read(reinterpret_cast<char*>(InfoHeaderBuffer.data()), sizeof(BITMAPINFOHEADER));

			const BITMAPFILEHEADER* BMPHeader{ reinterpret_cast<BITMAPFILEHEADER*>(FileHeaderBuffer.data()) };
			const BITMAPINFOHEADER* BMPInfo{ reinterpret_cast<BITMAPINFOHEADER*>(InfoHeaderBuffer.data()) };

			std::vector<char> InputBuffer(BMPInfo->biSizeImage);
			File.seekg(BMPHeader->bfOffBits);
			File.read(InputBuffer.data(), BMPInfo->biSizeImage);

			CreateTexture(Texture, BMPInfo->biWidth, BMPInfo->biHeight, 0x00000000);

			// Since we read the data from bottom upwards, we need to flip everything upside down
			for (std::int_fast32_t Offset{}, y{ Texture.Height - 1 }; y >= 0; --y)
			{
				const std::int_fast32_t TempY{ y * Texture.Width };

				for (std::int_fast32_t x{}; x < Texture.Width; ++x)
				{
					// Combine RGB values to given colour
					const std::int_fast32_t TempPos{ 3 * (TempY + x) };
					Texture.Pixels[static_cast<std::size_t>(Offset++)] = RGBAtoINT(InputBuffer[static_cast<std::size_t>(TempPos) + 2] & 255, InputBuffer[static_cast<std::size_t>(TempPos) + 1] & 255, InputBuffer[static_cast<std::size_t>(TempPos)] & 255, 255);
				}
			}
		}
	}


} // namespace lwmf