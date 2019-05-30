/*
******************************************
*                                        *
* GFX_TextClass.hpp                      *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include "Game_GlobalDefinitions.hpp"
#include "Tools_INIFile.hpp"
#include "GFX_OpenGLShaderClass.hpp"

class GFX_TextClass final
{
public:
	void InitFont(const std::string& INIFileName, const std::string& Section);
	void RenderText(const std::string& Text, std::int_fast32_t x, std::int_fast32_t y);
	void RenderTextCentered(const std::string& Text, std::int_fast32_t y);

	lwmf::IntPointStruct Offset{};
	std::int_fast32_t FontHeight{};

private:
	struct GlyphStruct final
	{
		std::int_fast32_t PosX{};
		std::int_fast32_t PosY{};
		std::int_fast32_t Height{};
		std::int_fast32_t Width{};
		std::int_fast32_t Advance{};
		std::int_fast32_t Baseline{};
		GLuint Texture{};
	};

	GFX_OpenGLShaderClass GlyphShader{};
	std::vector<GlyphStruct> Glyphs;
};

inline void GFX_TextClass::InitFont(const std::string& INIFileName, const std::string& Section)
{
	if (Tools_ErrorHandling::CheckFileExistence(INIFileName, ShowMessage, StopOnError))
	{
		if (const std::string FontName{ Tools_INIFile::ReadValue<std::string>(INIFileName, Section, "FontName") }; Tools_ErrorHandling::CheckFileExistence(FontName, ShowMessage, StopOnError))
		{
			GlyphShader.LoadShader("Default");

			const float FontSize{ Tools_INIFile::ReadValue<float>(INIFileName, Section, "FontSize") };
			const unsigned char FontColorRed{ static_cast<unsigned char>(Tools_INIFile::ReadValue<std::int_fast32_t>(INIFileName, Section, "ColorRED")) };
			const unsigned char FontColorGreen{ static_cast<unsigned char>(Tools_INIFile::ReadValue<std::int_fast32_t>(INIFileName, Section, "ColorGREEN")) };
			const unsigned char FontColorBlue{ static_cast<unsigned char>(Tools_INIFile::ReadValue<std::int_fast32_t>(INIFileName, Section, "ColorBLUE")) };
			Offset = { Tools_INIFile::ReadValue<std::int_fast32_t>(INIFileName, Section, "OffsetX"), Tools_INIFile::ReadValue<std::int_fast32_t>(INIFileName, Section, "OffsetY") };

			// Get raw (binary) font data
			std::vector<char> FontBuffer;
			std::ifstream FontFile(FontName.c_str(), std::ifstream::binary);
			FontFile.seekg(0, std::ios_base::end);
			FontBuffer.resize(FontFile.tellg());
			FontFile.seekg(0, std::ios_base::beg);
			FontFile.read(FontBuffer.data(), FontBuffer.size());

			// Render the glyphs for ASCII chars from 32 ("space") to 127 (last official ASCII char)
			// This makes 96 printable chars

			stbtt_fontinfo FontInfo{};
			stbtt_InitFont(&FontInfo, reinterpret_cast<unsigned char*>(FontBuffer.data()), 0);

			std::int_fast32_t Width{};
			FontHeight = static_cast<std::int_fast32_t>(FontSize + 1.0F);
			const std::int_fast32_t Height{ FontHeight + 5 };
			const std::int_fast32_t FirstASCIIChar{ 32 };
			const std::int_fast32_t LastASCIIChar{ 127 };
			const std::int_fast32_t NumberOfASCIIChars{ LastASCIIChar - FirstASCIIChar };

			for (char Char{ FirstASCIIChar }; Char < LastASCIIChar; ++Char)
			{
				lwmf::IntPointStruct i0{};
				lwmf::IntPointStruct i1{};

				stbtt_GetCodepointBitmapBox(&FontInfo, Char, 1.0F, 1.0F, &i0.X, &i0.Y, &i1.X, &i1.Y);
				Width += 1 + static_cast<std::int_fast32_t>((i1.X * FontSize / 1000.0F) + 1.0F) - static_cast<std::int_fast32_t>(i0.X * FontSize / 1000.0F);
			}

			std::vector<unsigned char> BakedFontGreyscale(Width * Height);
			std::vector<std::int_fast32_t> FontColor(Width * Height);
			stbtt_bakedchar CharData[NumberOfASCIIChars];
			stbtt_BakeFontBitmap(reinterpret_cast<unsigned char*>(FontBuffer.data()), 0, FontSize, BakedFontGreyscale.data(), Width, Height, FirstASCIIChar, NumberOfASCIIChars, CharData);

			// Since the glyphs were rendered in greyscale, they need to be colored...
			for (std::int_fast32_t i{}; i < BakedFontGreyscale.size(); ++i)
			{
				FontColor[i] = lwmf::RGBAtoINT(FontColorRed, FontColorGreen, FontColorBlue, BakedFontGreyscale[i]);
			}

			Glyphs.resize(127);

			for (std::int_fast32_t Char{ FirstASCIIChar }; Char < LastASCIIChar; ++Char)
			{
				lwmf::FloatPointStruct QuadPos{};
				stbtt_aligned_quad Quad;
				stbtt_GetBakedQuad(CharData, Width, Height, Char - FirstASCIIChar, &QuadPos.X, &QuadPos.Y, &Quad, 1);

				Glyphs[Char].PosX = static_cast<std::int_fast32_t>(Quad.s0 * Width);
				Glyphs[Char].PosY = static_cast<std::int_fast32_t>(Quad.t0 * Height);
				Glyphs[Char].Width = static_cast<std::int_fast32_t>(static_cast<std::int_fast32_t>(((Quad.s1 - Quad.s0) * Width) + 1.0F));
				Glyphs[Char].Height = static_cast<std::int_fast32_t>(static_cast<std::int_fast32_t>(((Quad.t1 - Quad.t0) * Height) + 1.0F));
				Glyphs[Char].Advance = static_cast<std::int_fast32_t>(QuadPos.X + 0.5F);
				Glyphs[Char].Baseline = static_cast<std::int_fast32_t>(-Quad.y0);

				// Blit single glyphs to individual textures
				lwmf::TextureStruct TempGlyphTexture;
				TempGlyphTexture.Pixels.resize(Glyphs[Char].Width * Glyphs[Char].Height);
				TempGlyphTexture.Width = Glyphs[Char].Width;
				TempGlyphTexture.Height = Glyphs[Char].Height;

				for (std::int_fast32_t TargetY{}, y{ Glyphs[Char].PosY }; y < Glyphs[Char].PosY + Glyphs[Char].Height; ++y, ++TargetY)
				{
					for (std::int_fast32_t TargetX{}, x{ Glyphs[Char].PosX }; x < Glyphs[Char].PosX + Glyphs[Char].Width; ++x, ++ TargetX)
					{
						TempGlyphTexture.Pixels[TargetY * TempGlyphTexture.Width + TargetX] = FontColor[y * Width + x];
					}
				}

				GlyphShader.LoadTextureInGPU(TempGlyphTexture, &Glyphs[Char].Texture);
			}
		}
	}
}

inline void GFX_TextClass::RenderText(const std::string& Text, std::int_fast32_t x, const std::int_fast32_t y)
{
	for (const char& Char : Text)
	{
		GlyphShader.RenderTexture(&Glyphs[Char].Texture, x, y - Glyphs[Char].Baseline + FontHeight, Glyphs[Char].Width, Glyphs[Char].Height);
		x += Glyphs[Char].Advance;
	}
}

inline void GFX_TextClass::RenderTextCentered(const std::string& Text, const std::int_fast32_t y)
{
	std::int_fast32_t TextLengthInPixels{};

	for (const char& Char : Text)
	{
		TextLengthInPixels += Glyphs[Char].Advance;
	}

	RenderText(Text, ScreenTexture.WidthMid - (TextLengthInPixels >> 1), y);
}