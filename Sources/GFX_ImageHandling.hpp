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

// Prepare stb_image
// Use only PNG decoder
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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
			std::int_fast32_t OriginalFormat{};

			unsigned char* ImageData{ stbi_load(ImageFileName.c_str(), &TempTexture.Width, &TempTexture.Height, &OriginalFormat, STBI_rgb_alpha) };

			TempTexture.Pixels.resize(TempTexture.Width * TempTexture.Height);

			for (std::int_fast32_t Offset{}; Offset < (TempTexture.Width * TempTexture.Height); ++Offset)
			{
				TempTexture.Pixels[Offset] = *reinterpret_cast<std::int_fast32_t*>(ImageData + Offset * 4);
			}

			stbi_image_free(ImageData);
		}

		return TempTexture;
	}

	inline lwmf::TextureStruct ImportTexture(const std::string& ImageFileName, const std::int_fast32_t Size)
	{
		lwmf::TextureStruct TempTexture;

		if (Tools_ErrorHandling::CheckFileExistence(ImageFileName, ShowMessage, StopOnError))
		{
			std::int_fast32_t OriginalFormat{};

			unsigned char* ImageData{ stbi_load(ImageFileName.c_str(), &TempTexture.Width, &TempTexture.Height, &OriginalFormat, STBI_rgb_alpha) };

			if (Tools_ErrorHandling::CheckTextureSize(TempTexture.Width, TempTexture.Height, Size, ShowMessage, StopOnError))
			{
				TempTexture.Pixels.resize(Size * Size);

				for (std::int_fast32_t Offset{}; Offset < (Size * Size); ++Offset)
				{
					TempTexture.Pixels[Offset] = *reinterpret_cast<std::int_fast32_t*>(ImageData + Offset * 4);
				}
			}

			stbi_image_free(ImageData);
		}

		return TempTexture;
	}


} // namespace GFX_ImageHandling
