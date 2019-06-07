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
#include <fstream>

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
		lwmf_SystemLog.AddEntry("BMP: Load file " + Filename + "...");

		std::ifstream File(Filename, std::ios::binary);

		if (File.fail())
		{
			lwmf_SystemLog.LogErrorAndThrowException("Error loading " + Filename + "!");
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

		SetTextureMetrics(Texture, BMPInfo->biWidth, BMPInfo->biHeight);
		Texture.Pixels.resize(static_cast<size_t>(Texture.Size));

		for (std::int_fast32_t Offset{}, y{ Texture.Height - 1 }; y >= 0; --y)
		{
			const std::int_fast32_t TempY{ y * Texture.Width };

			for (std::int_fast32_t x{}; x < Texture.Width; ++x)
			{
				const std::int_fast32_t TempPos{ 3 * (TempY + x) };
				Texture.Pixels[Offset++] = RGBAtoINT(InputBuffer[TempPos + 2] & 255, InputBuffer[TempPos + 1] & 255, InputBuffer[TempPos] & 255, 255);
			}
		}
	}


} // namespace lwmf