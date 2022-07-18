/*
******************************************
*                                        *
* GFX_TextClass.hpp                      *
*                                        *
* (c) 2017 - 2020 Stefan Kubsch          *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <istream>

#define STB_TRUETYPE_IMPLEMENTATION
#include "./stb/stb_truetype.hpp"

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"

class GFX_TextClass final
{
public:
	void InitFont(const std::string& INIFileName, const std::string& Section);
	void RenderText(std::string_view Text, std::int_fast32_t x, std::int_fast32_t y);
	void RenderTextCentered(std::string_view Text, std::int_fast32_t y);
	lwmf::IntPointStruct GetOffset();
	std::int_fast32_t GetFontHeight();

private:
	struct GlyphStruct final
	{
		std::int_fast32_t Height{};
		std::int_fast32_t Width{};
		std::int_fast32_t Advance{};
		std::int_fast32_t Baseline{};
		GLuint Texture{};
	};

	lwmf::ShaderClass GlyphShader{};
	std::vector<GlyphStruct> Glyphs{};

	lwmf::IntPointStruct Offset{};
	std::int_fast32_t FontHeight{};
};

inline void GFX_TextClass::InitFont(const std::string& INIFileName, const std::string& Section)
{
	NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init font...");

	if (Tools_ErrorHandling::CheckFileExistence(INIFileName, StopOnError))
	{
		const std::string FontName{ lwmf::ReadINIValue<std::string>(INIFileName, Section, "FontName") };

		GlyphShader.LoadShader("Default", ScreenTexture);

		const float FontSize{ lwmf::ReadINIValue<float>(INIFileName, Section, "FontSize") };
		const unsigned char FontColorRed{ static_cast<unsigned char>(lwmf::ReadINIValue<std::int_fast32_t>(INIFileName, Section, "ColorRED")) };
		const unsigned char FontColorGreen{ static_cast<unsigned char>(lwmf::ReadINIValue<std::int_fast32_t>(INIFileName, Section, "ColorGREEN")) };
		const unsigned char FontColorBlue{ static_cast<unsigned char>(lwmf::ReadINIValue<std::int_fast32_t>(INIFileName, Section, "ColorBLUE")) };
		Offset = { lwmf::ReadINIValue<std::int_fast32_t>(INIFileName, Section, "OffsetX"), lwmf::ReadINIValue<std::int_fast32_t>(INIFileName, Section, "OffsetY") };

		// Get raw (binary) font data
		if (Tools_ErrorHandling::CheckFileExistence(FontName, StopOnError))
		{
			std::ifstream FontFile(FontName.c_str(), std::ifstream::binary);

			FontFile.seekg(0, std::ios::end);
			std::vector<unsigned char> FontBuffer(static_cast<std::size_t>(FontFile.tellg()));
			FontFile.seekg(0, std::ios::beg);
			FontFile.read(reinterpret_cast<char*>(FontBuffer.data()), FontBuffer.size());

			// Render the glyphs for ASCII chars
			// This makes 128 chars, the last 96 are printable (from 32 = "space" on)

			stbtt_fontinfo FontInfo{};
			stbtt_InitFont(FontInfo, FontBuffer, 0);

			FontHeight = static_cast<std::int_fast32_t>(FontSize + 1.0F);
			const std::int_fast32_t Height{ FontHeight + 5 };
			constexpr std::int_fast32_t LastASCIIChar{ 127 }; // All ASCII chars except DEL (127)
			std::int_fast32_t Width{};

			for (char Char{}; Char < LastASCIIChar; ++Char)
			{
				lwmf::IntPointStruct i0{};
				lwmf::IntPointStruct i1{};

				stbtt_GetCodepointBitmapBox(FontInfo, Char, 1.0F, 1.0F, i0.X, i0.Y, i1.X, i1.Y);
				Width += 1 + (i1.X * static_cast<std::int_fast32_t>(FontSize) / 1000) + 1 - (i0.X * static_cast<std::int_fast32_t>(FontSize) / 1000);
			}

			const std::size_t Size{ static_cast<std::size_t>(Width) * static_cast<std::size_t>(Height) };
			std::vector<unsigned char> BakedFontGreyscale(Size);
			std::vector<stbtt_bakedchar> CharData(LastASCIIChar);
			stbtt_BakeFontBitmap(FontBuffer, 0, static_cast<float>(FontSize), BakedFontGreyscale, Width, Height, 0, LastASCIIChar, CharData);

			// Since the glyphs were rendered in greyscale, they need to be colored...
			std::vector<std::int_fast32_t> FontColor(Size);

			for (std::size_t i{}; i < Size; ++i)
			{
				FontColor[i] = lwmf::RGBAtoINT(FontColorRed, FontColorGreen, FontColorBlue, BakedFontGreyscale[i]);
			}

			Glyphs.resize(static_cast<std::size_t>(LastASCIIChar));

			for (std::int_fast32_t Char{}; Char < LastASCIIChar; ++Char)
			{
				lwmf::FloatPointStruct QuadPos{};
				stbtt_aligned_quad Quad{};
				stbtt_GetBakedQuad(CharData, Width, Height, Char, QuadPos.X, QuadPos.Y, Quad, 1);

				const lwmf::IntPointStruct Pos{ static_cast<std::int_fast32_t>(Quad.s0 * Width), static_cast<std::int_fast32_t>(Quad.t0 * Height) };
				Glyphs[Char].Width = static_cast<std::int_fast32_t>(static_cast<std::int_fast32_t>(((Quad.s1 - Quad.s0) * Width) + 1.0F));
				Glyphs[Char].Height = static_cast<std::int_fast32_t>(static_cast<std::int_fast32_t>(((Quad.t1 - Quad.t0) * Height) + 1.0F));
				Glyphs[Char].Advance = static_cast<std::int_fast32_t>(std::round(QuadPos.X));
				Glyphs[Char].Baseline = static_cast<std::int_fast32_t>(-Quad.y0);

				// Blit single glyphs to individual textures
				lwmf::TextureStruct TempGlyphTexture{};
				lwmf::CreateTexture(TempGlyphTexture, Glyphs[Char].Width, Glyphs[Char].Height, 0x00000000);

				for (std::int_fast32_t DestY{}, SrcY{ Pos.Y }; SrcY < Pos.Y + Glyphs[Char].Height; ++SrcY, ++DestY)
				{
					const std::int_fast32_t TempDestY{ DestY * TempGlyphTexture.Width };
					const std::int_fast32_t TempSrcY{ SrcY * Width };

					for (std::int_fast32_t DestX{}, SrcX{ Pos.X }; SrcX < Pos.X + Glyphs[Char].Width; ++SrcX, ++DestX)
					{
						TempGlyphTexture.Pixels[TempDestY + DestX] = FontColor[TempSrcY + SrcX];
					}
				}

				GlyphShader.LoadTextureInGPU(TempGlyphTexture, &Glyphs[Char].Texture);
			}
		}
	}
}

inline void GFX_TextClass::RenderText(const std::string_view Text, std::int_fast32_t x, const std::int_fast32_t y)
{
	for (const char& Char : Text)
	{
		GlyphShader.RenderTexture(&Glyphs[Char].Texture, x, y - Glyphs[Char].Baseline + FontHeight, Glyphs[Char].Width, Glyphs[Char].Height, true, 1.0F);
		x += Glyphs[Char].Advance;
	}
}

inline void GFX_TextClass::RenderTextCentered(const std::string_view Text, const std::int_fast32_t y)
{
	std::int_fast32_t TextLengthInPixels{};

	for (const char& Char : Text)
	{
		TextLengthInPixels += Glyphs[Char].Advance;
	}

	RenderText(Text, ScreenTexture.WidthMid - (TextLengthInPixels >> 1), y);
}

inline lwmf::IntPointStruct GFX_TextClass::GetOffset()
{
	return Offset;
}

inline std::int_fast32_t GFX_TextClass::GetFontHeight()
{
	return FontHeight;
}
