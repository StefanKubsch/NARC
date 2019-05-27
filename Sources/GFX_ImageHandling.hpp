/*
******************************************
*                                        *
* GFX_ImageHandling.hpp                  *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"

namespace GFX_ImageHandling
{


	lwmf::TextureStruct ImportImage(const std::string& ImageFileName);
	lwmf::TextureStruct ImportTexture(const std::string& ImageFileName, std::int_fast32_t Size);

	//
	// Functions
	//

	inline lwmf::TextureStruct ImportImage(const std::string& ImageFileName)
	{
		lwmf::TextureStruct TempTexture;

		if (Tools_ErrorHandling::CheckFileExistence(ImageFileName, ShowMessage, StopOnError))
		{
			std::vector<unsigned char> Buffer;
			std::vector<unsigned char> ImageData;

			lwmf::LoadPNG(Buffer, ImageFileName);
			lwmf::DecodePNG(ImageData, TempTexture.Width, TempTexture.Height, Buffer.data(), static_cast<std::int_fast32_t>(Buffer.size()));
			TempTexture.Pixels.resize(TempTexture.Width * TempTexture.Height);

			for (std::int_fast32_t Offset{}; Offset < (TempTexture.Width * TempTexture.Height); ++Offset)
			{
				TempTexture.Pixels[Offset] = lwmf::RGBAtoINT(ImageData[Offset << 2], ImageData[(Offset << 2) + 1], ImageData[(Offset << 2) + 2], ImageData[(Offset << 2) + 3]);
			}
		}

		return TempTexture;
	}

	inline lwmf::TextureStruct ImportTexture(const std::string& ImageFileName, const std::int_fast32_t Size)
	{
		lwmf::TextureStruct TempTexture;

		if (Tools_ErrorHandling::CheckFileExistence(ImageFileName, ShowMessage, StopOnError))
		{
			std::vector<unsigned char> Buffer;
			std::vector<unsigned char> ImageData;

			lwmf::LoadPNG(Buffer, ImageFileName);
			lwmf::DecodePNG(ImageData, TempTexture.Width, TempTexture.Height, Buffer.data(), static_cast<std::int_fast32_t>(Buffer.size()));

			if (Tools_ErrorHandling::CheckTextureSize(TempTexture.Width, TempTexture.Height, Size, ShowMessage, StopOnError))
			{
				TempTexture.Pixels.resize(TempTexture.Width * TempTexture.Height);

				for (std::int_fast32_t Offset{}; Offset < (TempTexture.Width * TempTexture.Height); ++Offset)
				{
					TempTexture.Pixels[Offset] = lwmf::RGBAtoINT(ImageData[Offset << 2], ImageData[(Offset << 2) + 1], ImageData[(Offset << 2) + 2], ImageData[(Offset << 2) + 3]);
				}
			}
		}

		return TempTexture;
	}


} // namespace GFX_ImageHandling
