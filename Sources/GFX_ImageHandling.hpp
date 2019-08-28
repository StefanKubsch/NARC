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
		lwmf::LoadPNG(TempTexture, ImageFileName);

		return TempTexture;
	}

	inline lwmf::TextureStruct ImportTexture(const std::string& ImageFileName, const std::int_fast32_t Size)
	{
		lwmf::TextureStruct TempTexture;
		lwmf::LoadPNG(TempTexture, ImageFileName);

		if (Tools_ErrorHandling::CheckTextureSize(TempTexture.Width, TempTexture.Height, Size, StopOnError))
		{
			// Dummy, just check Size
		}

		return TempTexture;
	}


} // namespace GFX_ImageHandling
