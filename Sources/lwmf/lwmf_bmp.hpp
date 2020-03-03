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
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <cstring>

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
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Load file " + Filename + "...");

		std::ifstream File(Filename, std::ios::in | std::ios::binary);

		if (File.fail())
		{
			std::array<char, 100> ErrorMessage{};
			strerror_s(ErrorMessage.data(), 100, errno);

			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error loading " + Filename + ": " + std::string(ErrorMessage.data()));
		}
		else
		{
			std::array<unsigned char, sizeof(BITMAPFILEHEADER)> FileHeaderBuffer{};
			std::array<unsigned char, sizeof(BITMAPINFOHEADER)> InfoHeaderBuffer{};

			File.read(reinterpret_cast<char*>(FileHeaderBuffer.data()), sizeof(BITMAPFILEHEADER));
			File.read(reinterpret_cast<char*>(InfoHeaderBuffer.data()), sizeof(BITMAPINFOHEADER));

			const BITMAPFILEHEADER* BMPHeader{ reinterpret_cast<BITMAPFILEHEADER*>(FileHeaderBuffer.data()) };
			const BITMAPINFOHEADER* BMPInfo{ reinterpret_cast<BITMAPINFOHEADER*>(InfoHeaderBuffer.data()) };

			std::vector<char> InputBuffer(BMPInfo->biSizeImage);
			File.seekg(BMPHeader->bfOffBits);
			File.read(InputBuffer.data(), BMPInfo->biSizeImage);

			CreateTexture(Texture, BMPInfo->biWidth, BMPInfo->biHeight, 0x00000000);

			for (std::int_fast32_t Offset{}, y{ Texture.Height - 1 }; y >= 0; --y)
			{
				const std::int_fast32_t TempY{ y * Texture.Width };

				for (std::int_fast32_t x{}; x < Texture.Width; ++x)
				{
					const std::int_fast32_t TempPos{ 3 * (TempY + x) };

					Texture.Pixels[static_cast<size_t>(Offset++)] = RGBAtoINT(InputBuffer[static_cast<size_t>(TempPos) + 2] & 255, InputBuffer[static_cast<size_t>(TempPos) + 1] & 255, InputBuffer[static_cast<size_t>(TempPos)] & 255, 255);
				}
			}
		}
	}


} // namespace lwmf