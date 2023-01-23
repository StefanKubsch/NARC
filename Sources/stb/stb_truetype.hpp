// stb_truetype.hpp - under development - public domain
//
// authored/modified in 2019/2020 by Stefan Kubsch
//
// a stripped down version of stb_truetype.h
//
// WHY AND WHAT?
// -------------
//
// Since I only use the "old" 3D API in my project, I removed all unrelated stuff and tried to "upgrade" the original code to C++.
//
// This lib is perfect if you just need following functions to generate a simple atlas-bitmap for further use with e.g.OpenGL:
//	- stbtt_InitFont()
//	- stbtt_GetCodepointBitmapBox()
//	- stbtt_BakeFontBitmap()
//	- stbtt_GetBakedQuad()
//
// CONVERSION TO C++
// -----------------
//
// WHAT I DID SO FAR:
//	- fixed all warnings reported by PVS-Studio and clang-tidy (use brackets etc.)
//	- replaced "malloc/free" with "new/delete"
//	- replaced all C-style casts with modern-style C++ cast (e.g. static_cast)
//	- initialised all variables and narrowed their scopes if possible
//	- fixed all implicit float-to-double-conversions by consequent use of the float literal (e.g. 0.0F)
//	- made everything "const", what can and should be const
//	- replaced C headers with the corresponding C++ headers
//	- used STL functions like "std::max" or "std::swap" where possible, replacing the old implementations
//	- replaced C-style arrays with std::vector (but, of course, this is still in work)
//	- tagged all functions as "inline"
//	- replaced "NULL" with "nullptr"
//	- removed all implicit casts by being more expressive/use of castings
//	- used "enum class" instead of plain enums
//	- replaced "int" / "unsigned int" with fixed width integer type "std::int_fast32_t"
//	- replaced "unsigned short" with fixed width integer type "std::int_fast32_t"
//
// FURTHER NOTES:
//		- removed Rasterizer version 1
//		- removed new 3D API
//		- removed all assert()
//		- no support for stb_rect_pack.h
//		- no implementation with other RAD tools
//		- removed font name matching API
//		- removed most of the "the macro-preprocessor-magic"
//
// TO DO:
// ------
//
//	- get rid of all these raw pointers
//	- work on replacing arrays with std::vector
//
// LICENSE
//
//   See end of file for license information.
//
// *********************************************************************************************
//
// Original version infos:
// stb_truetype.h - v1.21 - public domain
// authored from 2009-2016 by Sean Barrett / RAD Game Tools
//
//   This library processes TrueType files:
//        parse files
//        extract glyph metrics
//        extract glyph shapes
//        render glyphs to one-channel bitmaps with antialiasing (box filter)
//        render glyphs to one-channel SDF bitmaps (signed-distance field/function)
//
//
// ADDITIONAL CONTRIBUTORS
//
//   Mikko Mononen: compound shape support, more cmap formats
//   Tor Andersson: kerning, subpixel rendering
//   Dougall Johnson: OpenType / Type 2 font handling
//   Daniel Ribeiro Maciel: basic GPOS-based kerning
//
//   Misc other:
//       Ryan Gordon
//       Simon Glass
//       github:IntellectualKitty
//       Imanol Celaya
//       Daniel Ribeiro Maciel
//
//   Bug/warning reports/fixes:
//       "Zer" on mollyrocket       Fabian "ryg" Giesen
//       Cass Everitt               Martins Mozeiko
//       stoiko (Haemimont Games)   Cap Petschulat
//       Brian Hook                 Omar Cornut
//       Walter van Niftrik         github:aloucks
//       David Gow                  Peter LaValle
//       David Given                Sergey Popov
//       Ivan-Assen Ivanov          Giumo X. Clanjor
//       Anthony Pesch              Higor Euripedes
//       Johan Duparc               Thomas Fields
//       Hou Qiming                 Derek Vinyard
//       Rob Loach                  Cort Stratton
//       Kenney Phillis Jr.         github:oyvindjam
//       Brian Costabile            github:vassvik
//
//
// LICENSE
//
//   See end of file for license information.
//

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////
////   INTEGRATION WITH YOUR CODEBASE
////

#pragma once

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <vector>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////   INTERFACE
////
////

// private structure
struct stbtt__buf
{
	std::vector<unsigned char> data{};
	std::int_fast32_t cursor{};
	std::int_fast32_t size{};
};

//////////////////////////////////////////////////////////////////////////////
//
// TEXTURE BAKING API
//
// If you use this API, you only have to call two functions ever.
//

struct stbtt_bakedchar
{
	// coordinates of bbox in bitmap
	std::int_fast32_t x0{};
	std::int_fast32_t y0{};
	std::int_fast32_t x1{};
	std::int_fast32_t y1{};
	float xoff{};
	float yoff{};
	float xadvance{};
};

std::int_fast32_t stbtt_BakeFontBitmap(const std::vector<unsigned char>& data, std::int_fast32_t offset,			// font location (use offset=0 for plain .ttf)
                                float pixel_height,																	// height of font in pixels
                                std::vector<unsigned char>& pixels, std::int_fast32_t pw, std::int_fast32_t ph,		// bitmap to be filled in
                                std::int_fast32_t first_char, std::int_fast32_t num_chars,							// characters to bake
                                std::vector<stbtt_bakedchar>& chardata);											// you allocate this, it's num_chars long
// if return is positive, the first unused row of the bitmap
// if return is negative, returns the negative of the number of characters that fit
// if return is 0, no characters fit and no rows were used
// This uses a very crappy packing.

struct stbtt_aligned_quad
{
	// top-left
	float x0{};
	float y0{};
	float s0{};
	float t0{};
	// bottom-right
	float x1{};
	float y1{};
	float s1{};
	float t1{};
};

void stbtt_GetBakedQuad(const std::vector<stbtt_bakedchar>& chardata, std::int_fast32_t pw, std::int_fast32_t ph,	// same data as above
						std::int_fast32_t char_index,																// character to display
						float& xpos, float ypos,																// pointers to current position in screen pixel space
						stbtt_aligned_quad& q,																		// output: quad to draw
						std::int_fast32_t opengl_fillrule);															// true if opengl fill rule; false if DX9 or earlier
// Call GetBakedQuad with char_index = 'character - first_char', and it
// creates the quad you need to draw and advances the current position.
//
// The coordinate system used assumes y increases downwards.
//
// Characters will extend both above and below the current position;
// see discussion of "BASELINE" above.
//
// It's inefficient; you might want to c&p it and optimize it.

//////////////////////////////////////////////////////////////////////////////
//
// FONT LOADING
//
//

// The following structure is defined publicly so you can declare one on
// the stack or as a global or etc, but you should treat it as opaque.
struct stbtt_fontinfo
{
	std::shared_ptr<void> userdata;
	std::vector<unsigned char> data{};		// pointer to .ttf file
	std::int_fast32_t fontstart{};			// offset of start of font
	std::int_fast32_t numGlyphs{};			// number of glyphs, needed for range checking
	// table locations as offset from start of .ttf
	std::int_fast32_t loca{};
	std::int_fast32_t head{};
	std::int_fast32_t glyf{};
	std::int_fast32_t hhea{};
	std::int_fast32_t hmtx{};
	std::int_fast32_t kern{};
	std::int_fast32_t gpos{};
	std::int_fast32_t index_map{};			// a cmap mapping for our chosen character encoding
	std::int_fast32_t indexToLocFormat{};	// format needed to map from glyph index to glyph
	stbtt__buf cff{};						// cff font data
	stbtt__buf charstrings{};				// the charstring index
	stbtt__buf gsubrs{};					// global charstring subroutines index
	stbtt__buf subrs{};						// private charstring subroutines index
	stbtt__buf fontdicts{};					// array of font dicts
	stbtt__buf fdselect;					// map from glyph to fontdict
};

std::int_fast32_t stbtt_InitFont(stbtt_fontinfo& info, std::vector<unsigned char>& data, std::int_fast32_t offset);
// Given an offset into the file that defines a font, this function builds
// the necessary cached info for the rest of the system. You must allocate
// the stbtt_fontinfo yourself, and stbtt_InitFont will fill it out. You don't
// need to do anything special to free it, because the contents are pure
// value data with no additional data structures. Returns 0 on failure.

//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER TO GLYPH-INDEX CONVERSIOn

std::int_fast32_t stbtt_FindGlyphIndex(const stbtt_fontinfo& info, std::int_fast32_t unicode_codepoint);
// If you're going to perform multiple operations on the same character
// and you want a speed-up, call this function with the character you're
// going to process, then use glyph-based functions instead of the
// codepoint-based functions.
// Returns 0 if the character codepoint is not defined in the font.

//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER PROPERTIES
//

float stbtt_ScaleForPixelHeight(const stbtt_fontinfo& info, float height);
// computes a scale factor to produce a font whose "height" is 'pixels' tall.
// Height is measured as the distance from the highest ascender to the lowest
// descender.

void stbtt_GetGlyphHMetrics(const stbtt_fontinfo& info, std::int_fast32_t glyph_index, std::int_fast32_t& advanceWidth, std::int_fast32_t& leftSideBearing);
std::int_fast32_t stbtt_GetGlyphBox(const stbtt_fontinfo& info, std::int_fast32_t glyph_index, std::int_fast32_t& x0, std::int_fast32_t& y0, std::int_fast32_t& x1, std::int_fast32_t& y1);
// as above, but takes one or more glyph indices for greater efficiency

//////////////////////////////////////////////////////////////////////////////
//
// GLYPH SHAPES (you probably don't need these, but they have to go before
// the bitmaps for C declaration-order reasons)
//

enum class GlyphShapeType
{
	STBTT_vmove		= 1,
	STBTT_vline		= 2,
	STBTT_vcurve	= 3,
	STBTT_vcubic	= 4
};

struct stbtt_vertex
{
	short x{};
	short y{};
	short cx{};
	short cy{};
	short cx1{};
	short cy1{};
	unsigned char type{};
	std::int_fast32_t charpadding{};
};

std::int_fast32_t stbtt_GetGlyphShape(const stbtt_fontinfo& info, std::int_fast32_t glyph_index, std::vector<stbtt_vertex>& vertices);
// returns # of vertices and fills *vertices with the pointer to them
// these are expressed in "unscaled" coordinates
//
// The shape is a series of contours. Each one starts with
// a STBTT_moveto, then consists of a series of mixed
// STBTT_lineto and STBTT_curveto segments. A lineto
// draws a line from previous endpoint to its x,y; a curveto
// draws a quadratic bezier from previous endpoint to
// its x,y, using cx,cy as the bezier control point.


//////////////////////////////////////////////////////////////////////////////
//
// BITMAP RENDERING
//

void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo& font, std::int_fast32_t codepoint, float scale_x, float scale_y, std::int_fast32_t& ix0, std::int_fast32_t& iy0, std::int_fast32_t& ix1, std::int_fast32_t& iy1);
// get the bbox of the bitmap centered around the glyph origin; so the
// bitmap width is ix1-ix0, height is iy1-iy0, and location to place
// the bitmap top left is (leftSideBearing*scale,iy0).
// (Note that the bitmap uses y-increases-down, but the shape uses
// y-increases-up, so CodepointBitmapBox and CodepointBox are inverted.)

void stbtt_GetCodepointBitmapBoxSubpixel(const stbtt_fontinfo& font, std::int_fast32_t codepoint, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t& ix0, std::int_fast32_t& iy0, std::int_fast32_t& ix1, std::int_fast32_t& iy1);
// same as stbtt_GetCodepointBitmapBox, but you can specify a subpixel
// shift for the character

// the following functions are equivalent to the above functions, but operate
// on glyph indices instead of Unicode codepoints (for efficiency)
void stbtt_MakeGlyphBitmap(const stbtt_fontinfo& info, unsigned char* output, std::int_fast32_t out_w, std::int_fast32_t out_h, std::int_fast32_t out_stride, float scale_x, float scale_y, std::int_fast32_t glyph);
void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo& info, unsigned char* output, std::int_fast32_t out_w, std::int_fast32_t out_h, std::int_fast32_t out_stride, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t glyph);
void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo& font, std::int_fast32_t glyph, float scale_x, float scale_y, std::int_fast32_t& ix0, std::int_fast32_t& iy0, std::int_fast32_t& ix1, std::int_fast32_t& iy1);
void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo& font, std::int_fast32_t glyph, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t& ix0, std::int_fast32_t& iy0, std::int_fast32_t& ix1, std::int_fast32_t& iy1);

struct stbtt__bitmap
{
	std::int_fast32_t w{};
	std::int_fast32_t h{};
	std::int_fast32_t stride{};
	unsigned char* pixels{};
};

// rasterize a shape with quadratic beziers into a bitmap
void stbtt_Rasterize(stbtt__bitmap& result,									// 1-channel bitmap to draw into
						float flatness_in_pixels,							// allowable error of curve in pixels
						std::vector<stbtt_vertex>& vertices,				// array of vertices defining shape
						std::int_fast32_t num_verts,						// number of vertices in above array
						float scale_x, float scale_y,						// scale applied to input vertices
						float shift_x, float shift_y,						// translation applied to input vertices
						std::int_fast32_t x_off, std::int_fast32_t y_off,	// another translation applied to input
						std::int_fast32_t invert,							// if non-zero, vertically flip shape
						std::shared_ptr<void> userdata);					// context for to STBTT_MALLOC

enum class platformID
{
	STBTT_PLATFORM_ID_UNICODE   = 0,
	STBTT_PLATFORM_ID_MAC       = 1,
	STBTT_PLATFORM_ID_ISO       = 2,
	STBTT_PLATFORM_ID_MICROSOFT = 3
};

enum class encodingIDMicrosoft // for STBTT_PLATFORM_ID_MICROSOFT
{
	STBTT_MS_EID_SYMBOL        = 0,
	STBTT_MS_EID_UNICODE_BMP   = 1,
	STBTT_MS_EID_SHIFTJIS      = 2,
	STBTT_MS_EID_UNICODE_FULL  = 10
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////   IMPLEMENTATION
////
////

#ifdef STB_TRUETYPE_IMPLEMENTATION

//////////////////////////////////////////////////////////////////////////
//
// stbtt__buf helpers to parse data from file
//

inline static unsigned char stbtt__buf_get8(stbtt__buf& b)
{
	if (b.cursor >= b.size)
	{
		return 0;
	}

	return b.data[b.cursor++];
}

inline static unsigned char stbtt__buf_peek8(const stbtt__buf& b)
{
	if (b.cursor >= b.size)
	{
		return 0;
	}

	return b.data[b.cursor];
}

inline static void stbtt__buf_seek(stbtt__buf& b, const std::int_fast32_t o)
{
	b.cursor = (o > b.size || o < 0) ? b.size : o;
}

inline static void stbtt__buf_skip(stbtt__buf& b, const std::int_fast32_t o)
{
	stbtt__buf_seek(b, b.cursor + o);
}

inline static std::int_fast32_t stbtt__buf_get(stbtt__buf& b, const std::int_fast32_t n)
{
	std::int_fast32_t v{};

	for (std::int_fast32_t i{}; i < n; ++i)
	{
		v = (v << 8) | stbtt__buf_get8(b);
	}

	return v;
}

inline static stbtt__buf stbtt__new_buf(void* p, const std::size_t size)
{
	stbtt__buf r{};
	std::memcpy(r.data.data(), p, size * sizeof(void*)); //-V522
	r.size = static_cast<std::int_fast32_t>(size);
	return r;
}

inline static stbtt__buf stbtt__buf_range(const stbtt__buf& b, const std::int_fast32_t o, const std::int_fast32_t size)
{
	stbtt__buf r{ stbtt__new_buf(nullptr, 0) };

	if (o < 0 || size < 0 || o > b.size || size > b.size - o)
	{
		return r;
	}

	std::memcpy(r.data.data(), reinterpret_cast<void*>(b.data[o]), static_cast<std::size_t>(size) * sizeof(void*));
	r.size = size;
	return r;
}

inline static stbtt__buf stbtt__cff_get_index(stbtt__buf& b)
{
	const std::int_fast32_t start{ b.cursor };

	if (const std::int_fast32_t count{ stbtt__buf_get((b), 2) }; count != 0)
	{
		const std::int_fast32_t offsize{ stbtt__buf_get8(b) };
		stbtt__buf_skip(b, offsize * count);
		stbtt__buf_skip(b, stbtt__buf_get(b, offsize) - 1);
	}

	return stbtt__buf_range(b, start, b.cursor - start);
}

inline static std::int_fast32_t stbtt__cff_int(stbtt__buf& b)
{
	const std::int_fast32_t b0{ stbtt__buf_get8(b) };

	if (b0 >= 32 && b0 <= 246)
	{
		return b0 - 139;
	}

	if (b0 >= 247 && b0 <= 250)
	{
		return (b0 - 247)*256 + stbtt__buf_get8(b) + 108;
	}

	if (b0 >= 251 && b0 <= 254)
	{
		return -(b0 - 251) * 256 - stbtt__buf_get8(b) - 108;
	}

	if (b0 == 28)
	{
		return stbtt__buf_get((b), 2);
	}

	if (b0 == 29)
	{
	   return stbtt__buf_get((b), 4);
	}

	return 0;
}

inline static stbtt__buf stbtt__dict_get(stbtt__buf& b, const std::int_fast32_t key)
{
	stbtt__buf_seek(b, 0);

	while (b.cursor < b.size)
	{
		const std::int_fast32_t start{ b.cursor };

		while (stbtt__buf_peek8(b) >= 28)
		{
			if (const std::int_fast32_t b0{ stbtt__buf_peek8(b) }; b0 == 30)
			{
				stbtt__buf_skip(b, 1);

				while (b.cursor < b.size)
				{
					const std::int_fast32_t v{ stbtt__buf_get8(b) };

					if ((v & 0xF) == 0xF || (v >> 4) == 0xF)
					{
						break;
					}
				}
			}
			else
			{
				stbtt__cff_int(b);
			}
		}

		const std::int_fast32_t end{ b.cursor };
		std::int_fast32_t op{ stbtt__buf_get8(b) };

		if (op == 12)
		{
			op = stbtt__buf_get8(b) | 0x100;
		}

		if (op == key)
		{
			return stbtt__buf_range(b, start, end - start);
		}
	}

	return stbtt__buf_range(b, 0, 0);
}

inline static void stbtt__dict_get_ints(stbtt__buf& b, const std::int_fast32_t key, const std::int_fast32_t outcount, std::vector<std::int_fast32_t>& out)
{
	stbtt__buf operands{ stbtt__dict_get(b, key) };

	for (std::int_fast32_t i{}; i < outcount && operands.cursor < operands.size; ++i)
	{
		out[i] = stbtt__cff_int(operands);
	}
}

inline static stbtt__buf stbtt__cff_index_get(stbtt__buf b, const std::int_fast32_t i)
{
	stbtt__buf_seek(b, 0);

	const std::int_fast32_t count{ stbtt__buf_get((b), 2) };
	const std::int_fast32_t offsize{ stbtt__buf_get8(b) };

	stbtt__buf_skip(b, i * offsize);

	const std::int_fast32_t start{ stbtt__buf_get(b, offsize) };

	return stbtt__buf_range(b, 2 + (count + 1) * offsize + start, stbtt__buf_get(b, offsize) - start);
}

//////////////////////////////////////////////////////////////////////////
//
// accessors to parse data from file
//

// on platforms that don't allow misaligned reads, if we want to allow
// truetype fonts that aren't padded to alignment, define ALLOW_UNALIGNED_TRUETYPE

inline static std::int_fast32_t ttUSHORT(const unsigned char* p)
{
	return (p[0] << 8) + p[1];
}

inline static short ttSHORT(const unsigned char* p)
{
	return (p[0] << 8) + p[1];
}

inline static std::int_fast32_t ttULONG(const unsigned char* p)
{
	return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}

inline static std::int_fast32_t stbtt__find_table(const std::vector<unsigned char>& data, const std::int_fast32_t fontstart, const std::string& tag)
{
	const std::int_fast32_t num_tables{ ttUSHORT(data.data() + fontstart + 4) };
	const std::int_fast32_t tabledir{ fontstart + 12 };

	for (std::int_fast32_t i{}; i < num_tables; ++i)
	{
		const std::int_fast32_t loc{ tabledir + 16 * i };

		if ((data.data() + loc)[0] == tag[0] && (data.data() + loc)[1] == tag[1] && (data.data() + loc)[2] == tag[2] && (data.data() + loc)[3] == tag[3])
		{
			return ttULONG(data.data() + loc + 8);
		}
	}

	return 0;
}

inline static stbtt__buf stbtt__get_subrs(stbtt__buf cff, stbtt__buf fontdict)
{
	std::vector<std::int_fast32_t> private_loc(2);

	stbtt__dict_get_ints(fontdict, 18, 2, private_loc);

	if (private_loc[1] == 0 || private_loc[0] == 0)
	{
		return stbtt__new_buf(nullptr, 0);
	}

	stbtt__buf pdict{ stbtt__buf_range(cff, private_loc[1], private_loc[0]) };

	std::vector<std::int_fast32_t> subrsoff{};

	stbtt__dict_get_ints(pdict, 19, 1, subrsoff);

	if (subrsoff.empty())
	{
		return stbtt__new_buf(nullptr, 0);
	}

	stbtt__buf_seek(cff, private_loc[1] + subrsoff[0]);

	return stbtt__cff_get_index(cff);
}

inline static std::int_fast32_t stbtt_InitFont_internal(stbtt_fontinfo& info, std::vector<unsigned char>& data, const std::int_fast32_t fontstart)
{
	info.data = data;
	info.fontstart = fontstart;
	info.cff = stbtt__new_buf(nullptr, 0);

	info.loca = stbtt__find_table(data, fontstart, "loca"); // required
	info.head = stbtt__find_table(data, fontstart, "head"); // required
	info.glyf = stbtt__find_table(data, fontstart, "glyf"); // required
	info.hhea = stbtt__find_table(data, fontstart, "hhea"); // required
	info.hmtx = stbtt__find_table(data, fontstart, "hmtx"); // required
	info.kern = stbtt__find_table(data, fontstart, "kern"); // not required
	info.gpos = stbtt__find_table(data, fontstart, "GPOS"); // not required

	const std::int_fast32_t cmap{ stbtt__find_table(data, fontstart, "cmap") };       // required

	if (cmap == 0 || info.head == 0 || info.hhea == 0 || info.hmtx == 0)
	{
		return 0;
	}

	if (info.glyf != 0)
	{
		// required for truetype
		if (info.loca == 0)
		{
			return 0;
		}
	}
	else
	{
		// initialization for CFF / Type2 fonts (OTF)
		const std::int_fast32_t cff{ stbtt__find_table(data, fontstart, "CFF ") };

		if (cff == 0)
		{
			return 0;
		}

		info.fontdicts = stbtt__new_buf(nullptr, 0);
		info.fdselect = stbtt__new_buf(nullptr, 0);

		// this should use size from table (not 512MB) - 512 x 1024 x 1024
		info.cff = stbtt__new_buf(data.data() + cff, 536870912);
		stbtt__buf b{ info.cff };

		// read the header
		stbtt__buf_skip(b, 2);
		stbtt__buf_seek(b, stbtt__buf_get8(b)); // hdrsize

		// the name INDEX could list multiple fonts,
		// but we just use the first one.
		stbtt__cff_get_index(b);  // name INDEX
		const stbtt__buf topdictidx{ stbtt__cff_get_index(b) };
		stbtt__buf topdict{ stbtt__cff_index_get(topdictidx, 0) };
		stbtt__cff_get_index(b);  // string INDEX
		info.gsubrs = stbtt__cff_get_index(b);

		std::vector<std::int_fast32_t> cstype(2);
		std::vector<std::int_fast32_t> charstrings(1);
		std::vector<std::int_fast32_t> fdarrayoff(1);
		std::vector<std::int_fast32_t> fdselectoff(1);

		stbtt__dict_get_ints(topdict, 17, 1, charstrings);
		stbtt__dict_get_ints(topdict, 0x100 | 6, 1, cstype);
		stbtt__dict_get_ints(topdict, 0x100 | 36, 1, fdarrayoff);
		stbtt__dict_get_ints(topdict, 0x100 | 37, 1, fdselectoff);
		info.subrs = stbtt__get_subrs(b, topdict);

		// we only support Type 2 charstrings
		if (cstype[0] != 2)
		{
			return 0;
		}

		if (charstrings[0] == 0)
		{
			return 0;
		}

		if (fdarrayoff[0] != 0)
		{
			// looks like a CID font
			if (fdselectoff[0] == 0)
			{
				return 0;
			}

			stbtt__buf_seek(b, fdarrayoff[0]);
			info.fontdicts = stbtt__cff_get_index(b);
			info.fdselect = stbtt__buf_range(b, fdselectoff[0], b.size - fdselectoff[0]);
		}

		stbtt__buf_seek(b, charstrings[0]);
		info.charstrings = stbtt__cff_get_index(b);
	}

	const std::int_fast32_t t{ stbtt__find_table(data, fontstart, "maxp") };
	info.numGlyphs = (t != 0) ? ttUSHORT(data.data() + t + 4) : 0xFFFF;

	// find a cmap encoding table we understand *now* to avoid searching later.
	// the same regardless of glyph.
	const std::int_fast32_t numTables{ ttUSHORT(data.data() + cmap + 2) };
	info.index_map = 0;

	for (std::int_fast32_t i{}; i < numTables; ++i)
	{
		const std::int_fast32_t encoding_record{ cmap + 4 + 8 * i };

		// find an encoding we understand:
		switch(ttUSHORT(data.data() + encoding_record))
		{
			case static_cast<std::int_fast32_t>(platformID::STBTT_PLATFORM_ID_MICROSOFT):
			{
				switch (ttUSHORT(data.data() + encoding_record + 2))
				{
					case static_cast<std::int_fast32_t>(encodingIDMicrosoft::STBTT_MS_EID_UNICODE_BMP):
					{
						break;
					}
					case static_cast<std::int_fast32_t>(encodingIDMicrosoft::STBTT_MS_EID_UNICODE_FULL):
					{
						// MS/Unicode
						info.index_map = cmap + ttULONG(data.data() + encoding_record + 4);
						break;
					}
					default: {}
				}
				break;
			}
			case static_cast<std::int_fast32_t>(platformID::STBTT_PLATFORM_ID_UNICODE):
			{
				// Mac/iOS has these
				// all the encodingIDs are unicode, so we don't bother to check it
				info.index_map = cmap + ttULONG(data.data() + encoding_record + 4);
				break;
			}
			default: {}
		}
	}

	if (info.index_map == 0)
	{
		return 0;
	}

	info.indexToLocFormat = ttUSHORT(data.data() + info.head + 50);

	return 1;
}

inline std::int_fast32_t stbtt_FindGlyphIndex(const stbtt_fontinfo& info, const std::int_fast32_t unicode_codepoint)
{
	std::vector<unsigned char> data{ info.data };
	const std::int_fast32_t index_map{ info.index_map };
	const std::int_fast32_t format{ ttUSHORT(data.data() + index_map + 0) };

	if (format == 0)
	{
		// apple byte encoding
		if (unicode_codepoint < ttUSHORT(data.data() + index_map + 2) - 6)
		{
			return *static_cast<unsigned char*>(data.data() + index_map + 6 + unicode_codepoint);
		}

		return 0;
	}

	if (format == 6)
	{
		if (const std::int_fast32_t first{ ttUSHORT(data.data() + index_map + 6) }; unicode_codepoint >= first && unicode_codepoint < first + ttUSHORT(data.data() + index_map + 8))
		{
			return ttUSHORT(data.data() + index_map + 10 + (unicode_codepoint - first) * 2);
		}

		return 0;
	}

	if (format == 2)
	{
		return 0;
	}

	if (format == 4)
	{
		if (unicode_codepoint > 0xFFFF)
		{
			return 0;
		}

		// standard mapping for windows fonts: binary search collection of ranges
		const std::int_fast32_t segcount{ ttUSHORT(data.data() + index_map + 6) >> 1 };
		std::int_fast32_t searchRange{ttUSHORT(data.data() + index_map + 8) >> 1 };
		std::int_fast32_t entrySelector{ ttUSHORT(data.data() + index_map + 10) };
		const std::int_fast32_t rangeShift{ ttUSHORT(data.data() + index_map + 12) >> 1 };

		// do a binary search of the segments
		const std::int_fast32_t endCount{ index_map + 14 };
		std::int_fast32_t search{ endCount };

		// they lie from endCount .. endCount + segCount
		// but searchRange is the nearest power of two, so...
		if (unicode_codepoint >= ttUSHORT(data.data() + search + (rangeShift << 1)))
		{
			search += rangeShift << 1;
		}

		// now decrement to bias correctly to find smallest
		search -= 2;

		while (entrySelector != 0)
		{
			searchRange >>= 1;

			if (unicode_codepoint > ttUSHORT(data.data() + search + (searchRange << 1)))
			{
				search += searchRange << 1;
			}

			--entrySelector;
		}

		search += 2;

		{
			const std::int_fast32_t item{ (search - endCount) >> 1 };
			const std::int_fast32_t start{ ttUSHORT(data.data() + index_map + 14 + segcount * 2 + 2 + 2 * item) };

			if (unicode_codepoint < start)
			{
				return 0;
			}

			const std::int_fast32_t offset{ ttUSHORT(data.data() + index_map + 14 + segcount * 6 + 2 + 2 * item) };

			if (offset == 0)
			{
				return unicode_codepoint + ttSHORT(data.data() + index_map + 14 + segcount * 4 + 2 + 2 * item);
			}

			return ttUSHORT(data.data() + offset + (unicode_codepoint - start) * 2 + index_map + 14 + segcount * 6 + 2 + 2 * item);
		}
	}

	if (format == 12 || format == 13)
	{
		std::int_fast32_t low{};
		std::int_fast32_t high{ ttULONG(data.data() + index_map + 12) };
		// Binary search the right group.

		while (low < high)
		{
			const std::int_fast32_t mid{ low + ((high - low) >> 1) }; // rounds down, so low <= mid < high
			const std::int_fast32_t start_char{ ttULONG(data.data() + index_map + 16 + mid * 12) };
			const std::int_fast32_t end_char{ ttULONG(data.data() + index_map + 16 + mid * 12 + 4) };

			if (unicode_codepoint < start_char)
			{
				high = mid;
			}
			else if (unicode_codepoint > end_char)
			{
				low = mid + 1;
			}
			else
			{
				const std::int_fast32_t start_glyph{ ttULONG(data.data() + index_map + 16 + mid * 12 + 8) };

				if (format == 12)
				{
					return start_glyph + unicode_codepoint - start_char;
				}

				if (format == 13)
				{
					return start_glyph;
				}
			}
		}

		return 0; // not found
	}

	return 0;
}

inline static void stbtt_setvertex(stbtt_vertex& v, const unsigned char type, const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t cx, const std::int_fast32_t cy)
{
	v.type = type;
	v.x = static_cast<short>(x);
	v.y = static_cast<short>(y);
	v.cx = static_cast<short>(cx);
	v.cy = static_cast<short>(cy);
}

inline static std::int_fast32_t stbtt__GetGlyfOffset(const stbtt_fontinfo& info, const std::int_fast32_t glyph_index)
{
	if (glyph_index >= info.numGlyphs)
	{
		return -1; // glyph index out of range
	}

	if (info.indexToLocFormat >= 2)
	{
		return -1; // unknown index->glyph map format
	}

	std::int_fast32_t g1{};
	std::int_fast32_t g2{};

	if (info.indexToLocFormat == 0)
	{
		g1 = info.glyf + ttUSHORT(info.data.data() + info.loca + glyph_index * 2) * 2;
		g2 = info.glyf + ttUSHORT(info.data.data() + info.loca + glyph_index * 2 + 2) * 2;
	}
	else
	{
		g1 = info.glyf + ttULONG (info.data.data() + info.loca + glyph_index * 4);
		g2 = info.glyf + ttULONG (info.data.data() + info.loca + glyph_index * 4 + 4);
	}

	return g1 == g2 ? -1 : g1; // if length is 0, return -1
}

static std::int_fast32_t stbtt__GetGlyphInfoT2(const stbtt_fontinfo& info, std::int_fast32_t glyph_index, std::int_fast32_t& x0, std::int_fast32_t& y0, std::int_fast32_t& x1, std::int_fast32_t& y1);

inline std::int_fast32_t stbtt_GetGlyphBox(const stbtt_fontinfo& info, const std::int_fast32_t glyph_index, std::int_fast32_t& x0, std::int_fast32_t& y0, std::int_fast32_t& x1, std::int_fast32_t& y1)
{
	if (info.cff.size != 0)
	{
		stbtt__GetGlyphInfoT2(info, glyph_index, x0, y0, x1, y1);
	}
	else
	{
		const std::int_fast32_t g{ stbtt__GetGlyfOffset(info, glyph_index) };

		if (g < 0)
		{
			return 0;
		}

		x0 = ttSHORT(info.data.data() + g + 2);
		y0 = ttSHORT(info.data.data() + g + 4);
		x1 = ttSHORT(info.data.data() + g + 6);
		y1 = ttSHORT(info.data.data() + g + 8);
	}

	return 1;
}

inline static std::int_fast32_t stbtt__close_shape(std::vector<stbtt_vertex>& vertices, std::int_fast32_t num_vertices, const std::int_fast32_t was_off, const std::int_fast32_t start_off, const std::int_fast32_t sx, const std::int_fast32_t sy, const std::int_fast32_t scx, const std::int_fast32_t scy, const std::int_fast32_t cx, const std::int_fast32_t cy)
{
	if (start_off != 0)
	{
		if (was_off != 0)
		{
			stbtt_setvertex(vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve), (cx + scx) >> 1, (cy + scy) >> 1, cx, cy);
		}

		stbtt_setvertex(vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve), sx, sy, scx, scy);
	}
	else
	{
		was_off != 0 ? stbtt_setvertex(vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve), sx, sy, cx, cy) : stbtt_setvertex(vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vline), sx, sy, 0, 0);
	}

	return num_vertices;
}

inline static std::int_fast32_t stbtt__GetGlyphShapeTT(const stbtt_fontinfo& info, const std::int_fast32_t glyph_index, std::vector<stbtt_vertex>& pvertices)
{
	const std::int_fast32_t g{ stbtt__GetGlyfOffset(info, glyph_index) };

	if (g < 0)
	{
		return 0;
	}

	std::vector<unsigned char> data{ info.data };
	std::int_fast32_t num_vertices{};
	const short numberOfContours{ ttSHORT(data.data() + g) };
	std::vector<stbtt_vertex> vertices{};

	if (numberOfContours > 0)
	{
		unsigned char* endPtsOfContours{ (data.data() + g + 10) };
		const std::int_fast32_t n{ 1 + ttUSHORT(endPtsOfContours + (numberOfContours << 1) - 2) };
		const std::int_fast32_t m{ n + (numberOfContours << 1) };  // a loose bound on how many vertices we might need
		vertices.resize(m);

		if (vertices.empty())
		{
			return 0;
		}

		// in first pass, we load uninterpreted data into the allocated array
		// above, shifted to the end of the array so we won't overwrite it when
		// we create our final data starting from the front

		const std::int_fast32_t off{ m - n }; // starting offset for uninterpreted data, regardless of how m ends up being calculated
		unsigned char flagcount{};

		// first load flags
		unsigned char flags{};
		unsigned char* points{ data.data() + g + 10 + (numberOfContours << 1) + 2 + ttUSHORT(data.data() + g + 10 + (numberOfContours << 1)) };

		for (std::int_fast32_t i{}; i < n; ++i)
		{
			if (flagcount == 0)
			{
				flags = *points++;

				if ((flags & 8) != 0)
				{
					flagcount = *points++;
				}
			}
			else
			{
				--flagcount;
			}

			vertices[off + i].type = flags;
		}

		std::int_fast32_t x{};

		// now load x coordinates
		for (std::int_fast32_t i{}; i < n; ++i)
		{
			flags = vertices[off + i].type;

			if ((flags & 2) != 0)
			{
				const short dx{ *points++ };
				x += (flags & 16) != 0 ? dx : -dx; // ???
			}
			else
			{
				if ((flags & 16) == 0)
				{
					x += static_cast<std::int_fast32_t>((points[0] << 8) + points[1]);
					points += 2;
				}
			}

			vertices[off+i].x = static_cast<short>(x);
		}

		std::int_fast32_t y{};

		// now load y coordinates
		for (std::int_fast32_t i{}; i < n; ++i)
		{
			flags = vertices[off + i].type;

			if ((flags & 4) != 0)
			{
				const short dy{ *points++ };
				y += (flags & 32) != 0 ? dy : -dy;
			}
			else
			{
				if ((flags & 32) == 0)
				{
					y = y + static_cast<std::int_fast32_t>((points[0] << 8) + points[1]);
					points += 2;
				}
			}
			vertices[off + i].y = static_cast<short>(y);
		}

		std::int_fast32_t scx{};
		std::int_fast32_t scy{};
		std::int_fast32_t sx{};
		std::int_fast32_t sy{};
		std::int_fast32_t cx{};
		std::int_fast32_t cy{};
		std::int_fast32_t was_off{};
		std::int_fast32_t start_off{};
		std::int_fast32_t j{};
		std::int_fast32_t next_move{};

		// now convert them to our format
		for (std::int_fast32_t i{}; i < n; ++i)
		{
			flags = vertices[off + i].type;
			x = static_cast<std::int_fast32_t>(vertices[off + i].x);
			y = static_cast<std::int_fast32_t>(vertices[off + i].y);

			if (next_move == i)
			{
				if (i != 0)
				{
					num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx, sy, scx, scy, cx, cy);
				}

				// now start the new one
				start_off = static_cast<std::int_fast32_t>(static_cast<std::int_fast32_t>((flags & 1)) == 0);

				if (start_off != 0)
				{
					// if we start off with an off-curve point, then when we need to find a point on the curve
					// where we can start, and we need to save some state for when we wraparound.
					scx = x;
					scy = y;

					if ((vertices[off + i + 1].type & 1) == 0)
					{
						// next point is also a curve point, so interpolate an on-point curve
						sx = (x + static_cast<std::int_fast32_t>(vertices[off + i + 1].x)) >> 1;
						sy = (y + static_cast<std::int_fast32_t>(vertices[off + i + 1].y)) >> 1;
					}
					else
					{
						// otherwise just use the next point as our start point
						sx = static_cast<std::int_fast32_t>(vertices[off + i + 1].x);
						sy = static_cast<std::int_fast32_t>(vertices[off + i + 1].y);
						++i; // we're using point i+1 as the starting point, so skip it
					}
				}
				else
				{
					sx = x;
					sy = y;
				}

				stbtt_setvertex(vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vmove), sx, sy, 0, 0);
				was_off = 0;
				next_move = 1 + ttUSHORT(endPtsOfContours + (j << 1));
				++j;
			}
			else
			{
				if ((flags & 1) == 0)
				{ // if it's a curve
					if (was_off != 0) // two off-curve control points in a row means interpolate an on-curve midpoint
					{
						stbtt_setvertex(vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve), (cx + x) >> 1, (cy + y) >> 1, cx, cy);
					}

					cx = x;
					cy = y;
					was_off = 1;
				}
				else
				{
					was_off != 0 ? stbtt_setvertex(vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve), x, y, cx, cy) : stbtt_setvertex(vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vline), x, y, 0, 0);
					was_off = 0;
				}
			}
		}

		num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx, sy, scx, scy, cx, cy);
	}
	else if (numberOfContours == -1)
	{
		// Compound shapes.
		std::int_fast32_t more{ 1 };
		unsigned char* comp{ data.data() + g + 10 };

		while (more != 0)
		{
			std::array<float, 6> mtx{ 1, 0, 0, 1, 0, 0 };

			const std::int_fast32_t flags{ static_cast<std::int_fast32_t>(ttSHORT(comp)) };
			comp += 2;
			const std::int_fast32_t gidx{ static_cast<std::int_fast32_t>(ttSHORT(comp)) };
			comp += 2;

			if ((flags & 2) != 0)
			{ // XY values
				if ((flags & 1) != 0)
				{ // shorts
					mtx[4] = ttSHORT(comp);
					comp += 2;
					mtx[5] = ttSHORT(comp);
					comp += 2;
				}
				else
				{
					mtx[4] = *reinterpret_cast<char*>(comp);
					comp += 1;
					mtx[5] = *reinterpret_cast<char*>(comp);
					comp += 1;
				}
			}

			if ((flags & (1 << 3)) != 0)
			{ // WE_HAVE_A_SCALE
				mtx[0] = static_cast<float>(ttSHORT(comp)) / 16384.0F;
				mtx[3] = mtx[0];
				comp += 2;
				mtx[1] = 0;
				mtx[2] = 0;
			}
			else if ((flags & (1 <<6 )) != 0)
			{ // WE_HAVE_AN_X_AND_YSCALE
				mtx[0] = static_cast<float>(ttSHORT(comp)) / 16384.0F;
				comp += 2;
				mtx[1] = 0;
				mtx[2] = 0;
				mtx[3] = static_cast<float>(ttSHORT(comp)) / 16384.0F;
				comp+=2;
			}
			else if ((flags & (1 << 7)) != 0)
			{ // WE_HAVE_A_TWO_BY_TWO
				mtx[0] = static_cast<float>(ttSHORT(comp)) / 16384.0F;
				comp += 2;
				mtx[1] = static_cast<float>(ttSHORT(comp)) / 16384.0F;
				comp += 2;
				mtx[2] = static_cast<float>(ttSHORT(comp)) / 16384.0F;
				comp += 2;
				mtx[3] = static_cast<float>(ttSHORT(comp)) / 16384.0F;
				comp += 2;
			}

			// Find transformation scales.
			const float m{ std::sqrtf(mtx[0] * mtx[0] + mtx[1] * mtx[1]) };
			const float n{ std::sqrtf(mtx[2] * mtx[2] + mtx[3] * mtx[3]) };

			// Get indexed glyph.
			std::vector<stbtt_vertex> comp_verts{};

			if (const std::int_fast32_t comp_num_verts{ stbtt_GetGlyphShape(info, gidx, comp_verts) }; comp_num_verts > 0)
			{
				// Transform vertices.
				for (std::int_fast32_t i{}; i < comp_num_verts; ++i)
				{
					short x{ comp_verts[i].x };
					short y{ comp_verts[i].y };

					comp_verts[i].x = static_cast<short>(m* (mtx[0] * static_cast<float>(x) + mtx[2] * static_cast<float>(y) + mtx[4]));
					comp_verts[i].y = static_cast<short>(n* (mtx[1] * static_cast<float>(x) + mtx[3] * static_cast<float>(y) + mtx[5]));

					x = comp_verts[i].cx;
					y = comp_verts[i].cy;

					comp_verts[i].cx = static_cast<short>(m* (mtx[0] * static_cast<float>(x) + mtx[2] * static_cast<float>(y) + mtx[4]));
					comp_verts[i].cy = static_cast<short>(n* (mtx[1] * static_cast<float>(x) + mtx[3] * static_cast<float>(y) + mtx[5]));
				}

				// Append vertices.
				std::vector<stbtt_vertex> tmp(static_cast<std::size_t>(num_vertices) + static_cast<std::size_t>(comp_num_verts));

				if (tmp.empty())
				{
					return 0;
				}

				if (num_vertices > 0 && !vertices.empty())
				{
					std::copy(vertices.begin(), vertices.begin() + num_vertices, tmp.begin());
				}

				std::copy(comp_verts.begin(), comp_verts.begin() + comp_num_verts, tmp.begin() + num_vertices);

				vertices = std::move(tmp);
				num_vertices += comp_num_verts;
			}

			// More components ?
			more = flags & (1 << 5);
		}
	}

	pvertices = vertices;

	return num_vertices;
}

struct stbtt__csctx final
{
	std::int_fast32_t bounds{};
	std::int_fast32_t started{};
	float first_x{};
	float first_y{};
	float x{};
	float y{};
	std::int_fast32_t min_x{};
	std::int_fast32_t max_x{};
	std::int_fast32_t min_y{};
	std::int_fast32_t max_y{};
	std::vector<stbtt_vertex> pvertices{};
	std::int_fast32_t num_vertices{};
};

inline static void stbtt__track_vertex(stbtt__csctx& c, const std::int_fast32_t x, const std::int_fast32_t y)
{
	if (x > c.max_x || c.started == 0)
	{
		c.max_x = x;
	}

	if (y > c.max_y || c.started == 0)
	{
		c.max_y = y;
	}

	if (x < c.min_x || c.started == 0)
	{
		c.min_x = x;
	}

	if (y < c.min_y || c.started == 0)
	{
		c.min_y = y;
	}

	c.started = 1;
}

inline static void stbtt__csctx_v(stbtt__csctx& c, const unsigned char type, const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t cx, const std::int_fast32_t cy, const std::int_fast32_t cx1, const std::int_fast32_t cy1)
{
	if (c.bounds != 0)
	{
		stbtt__track_vertex(c, x, y);

		if (type == static_cast<unsigned char>(GlyphShapeType::STBTT_vcubic))
		{
			stbtt__track_vertex(c, cx, cy);
			stbtt__track_vertex(c, cx1, cy1);
		}
	}
	else
	{
		stbtt_setvertex(c.pvertices[c.num_vertices], type, x, y, cx, cy);
		c.pvertices[c.num_vertices].cx1 = static_cast<short>(cx1);
		c.pvertices[c.num_vertices].cy1 = static_cast<short>(cy1);
	}

	++c.num_vertices;
}

inline static void stbtt__csctx_close_shape(stbtt__csctx& ctx)
{
	if (std::abs(ctx.first_x - ctx.x) > FLT_EPSILON || std::abs(ctx.first_y - ctx.y) > FLT_EPSILON)
	{
		stbtt__csctx_v(ctx, static_cast<unsigned char>(GlyphShapeType::STBTT_vline), static_cast<std::int_fast32_t>(ctx.first_x), static_cast<std::int_fast32_t>(ctx.first_y), 0, 0, 0, 0);
	}
}

inline static void stbtt__csctx_rmove_to(stbtt__csctx& ctx, const float dx, const float dy)
{
	stbtt__csctx_close_shape(ctx);
	ctx.first_x = ctx.x = ctx.x + dx;
	ctx.first_y = ctx.y = ctx.y + dy;
	stbtt__csctx_v(ctx, static_cast<unsigned char>(GlyphShapeType::STBTT_vmove), static_cast<std::int_fast32_t>(ctx.x), static_cast<std::int_fast32_t>(ctx.y), 0, 0, 0, 0);
}

inline static void stbtt__csctx_rline_to(stbtt__csctx& ctx, const float dx, const float dy)
{
	ctx.x += dx;
	ctx.y += dy;
	stbtt__csctx_v(ctx, static_cast<unsigned char>(GlyphShapeType::STBTT_vline), static_cast<std::int_fast32_t>(ctx.x), static_cast<std::int_fast32_t>(ctx.y), 0, 0, 0, 0);
}

inline static void stbtt__csctx_rccurve_to(stbtt__csctx& ctx, const float dx1, const float dy1, const float dx2, const float dy2, const float dx3, const float dy3)
{
	const float cx1{ ctx.x + dx1 };
	const float cy1{ ctx.y + dy1 };
	const float cx2{ cx1 + dx2 };
	const float cy2{ cy1 + dy2 };

	ctx.x = cx2 + dx3;
	ctx.y = cy2 + dy3;

	stbtt__csctx_v(ctx, static_cast<unsigned char>(GlyphShapeType::STBTT_vcubic), static_cast<std::int_fast32_t>(ctx.x), static_cast<std::int_fast32_t>(ctx.y), static_cast<std::int_fast32_t>(cx1), static_cast<std::int_fast32_t>(cy1), static_cast<std::int_fast32_t>(cx2), static_cast<std::int_fast32_t>(cy2));
}

inline static stbtt__buf stbtt__get_subr(stbtt__buf idx, std::int_fast32_t n)
{
	stbtt__buf_seek(idx, 0);
	const std::int_fast32_t count{ stbtt__buf_get((idx), 2) };
	std::int_fast32_t bias{ 107 };

	if (count >= 33900)
	{
		bias = 32768;
	}
	else if (count >= 1240)
	{
		bias = 1131;
	}

	n += bias;

	if (n < 0 || n >= count)
	{
		return stbtt__new_buf(nullptr, 0);
	}

	return stbtt__cff_index_get(idx, n);
}

inline static stbtt__buf stbtt__cid_get_glyph_subrs(const stbtt_fontinfo& info, const std::int_fast32_t glyph_index)
{
	stbtt__buf fdselect{ info.fdselect };
	std::int_fast32_t fdselector{ -1 };

	stbtt__buf_seek(fdselect, 0);

	if (const std::int_fast32_t fmt{ stbtt__buf_get8(fdselect) }; fmt == 0)
	{
		// untested
		stbtt__buf_skip(fdselect, glyph_index);
		fdselector = stbtt__buf_get8(fdselect);
	}
	else if (fmt == 3)
	{
		const std::int_fast32_t nranges{ stbtt__buf_get((fdselect), 2) };
		std::int_fast32_t start{ stbtt__buf_get((fdselect), 2) };

		for (std::int_fast32_t i{}; i < nranges; ++i)
		{
			const std::int_fast32_t v{ stbtt__buf_get8(fdselect) };
			const std::int_fast32_t end{ stbtt__buf_get((fdselect), 2) };

			if (glyph_index >= start && glyph_index < end)
			{
				fdselector = v;
				break;
			}

			start = end;
		}
	}

	if (fdselector == -1)
	{
		stbtt__new_buf(nullptr, 0);
	}

	return stbtt__get_subrs(info.cff, stbtt__cff_index_get(info.fontdicts, fdselector));
}

inline static std::int_fast32_t stbtt__run_charstring(const stbtt_fontinfo& info, const std::int_fast32_t glyph_index, stbtt__csctx& c)
{
	std::int_fast32_t in_header{ 1 };
	std::int_fast32_t maskbits{};
	std::int_fast32_t subr_stack_height{};
	std::int_fast32_t sp{};
	std::int_fast32_t has_subrs{};
	std::array<float, 48> s{};
	std::array<stbtt__buf, 10> subr_stack{};
	stbtt__buf subrs{ info.subrs };

	#define STBTT__CSERR(s) (0)

	// this currently ignores the initial width value, which isn't needed if we have hmtx
	stbtt__buf b{ stbtt__cff_index_get(info.charstrings, glyph_index) };

	while (b.cursor < b.size)
	{
		std::int_fast32_t i{};
		const std::int_fast32_t b0{ stbtt__buf_get8(b) };

		switch (b0)
		{
			case 0x14: // cntrmask
			{
				if (in_header != 0)
				{
					maskbits += (sp / 2); // implicit "vstem"
				}

				in_header = 0;
				stbtt__buf_skip(b, (maskbits + 7) >> 3);
				break;
			}
			case 0x15: // rmoveto
			{
				in_header = 0;

				if (sp < 2)
				{
					return STBTT__CSERR("rmoveto stack");
				}

				stbtt__csctx_rmove_to(c, s[sp - 2], s[sp - 1]);
				break;
			}
			case 0x04: // vmoveto
			{
				in_header = 0;

				if (sp < 1)
				{
					return STBTT__CSERR("vmoveto stack");
				}

				stbtt__csctx_rmove_to(c, 0, s[sp - 1]);
				break;
			}
			case 0x16: // hmoveto
			{
				in_header = 0;

				if (sp < 1)
				{
					return STBTT__CSERR("hmoveto stack");
				}

				stbtt__csctx_rmove_to(c, s[sp - 1], 0);
				break;
			}
			case 0x05: // rlineto
			{
				if (sp < 2)
				{
					return STBTT__CSERR("rlineto stack");
				}

				for (; i + 1 < sp; i += 2)
				{
					stbtt__csctx_rline_to(c, s[i], s[i + 1]);
				}

				break;
			}
			// hlineto/vlineto and vhcurveto/hvcurveto alternate horizontal and vertical
			// starting from a different place.
			case 0x07: // vlineto
			{
				if (sp < 1)
				{
					return STBTT__CSERR("vlineto stack");
				}

				goto vlineto;
			}
			case 0x06: // hlineto
			{
				if (sp < 1)
				{
					return STBTT__CSERR("hlineto stack");
				}

				for (;;)
				{
					if (i >= sp) //-V547
					{
						break;
					}
				}

				stbtt__csctx_rline_to(c, s[i], 0);
				++i;

				vlineto:
				if (i >= sp) //-V547
				{
					break;
				}

				stbtt__csctx_rline_to(c, 0, s[i]);
				++i;

				break;
			}
			case 0x1F: // hvcurveto
			{
				if (sp < 4)
				{
					return STBTT__CSERR("hvcurveto stack");
				}

				goto hvcurveto;
			}
			case 0x1E: // vhcurveto
			{
				if (sp < 4)
				{
					return STBTT__CSERR("vhcurveto stack");
				}

				for (;;)
				{
					if (i + 3 >= sp)
					{
						break;
					}

					stbtt__csctx_rccurve_to(c, 0, s[i], s[i + 1], s[i + 2], s[i + 3], (sp - i == 5) ? s[i + 4] : 0.0F);
					i += 4;

					hvcurveto:
					if (i + 3 >= sp)
					{
						break;
					}

					stbtt__csctx_rccurve_to(c, s[i], 0, s[i + 1], s[i + 2], (sp - i == 5) ? s[i + 4] : 0.0F, s[i + 3]);
					i += 4;
				}
				break;
			}
			case 0x08: // rrcurveto
			{
				if (sp < 6)
				{
					return STBTT__CSERR("rcurveline stack");
				}

				for (; i + 5 < sp; i += 6)
				{
					stbtt__csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]);
				}

				break;
			}
			case 0x18: // rcurveline
			{
				if (sp < 8)
				{
					return STBTT__CSERR("rcurveline stack");
				}

				for (; i + 5 < sp - 2; i += 6)
				{
					stbtt__csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]);
				}

				if (i + 1 >= sp)
				{
					return STBTT__CSERR("rcurveline stack");
				}

				stbtt__csctx_rline_to(c, s[i], s[i + 1]);
				break;
			}
			case 0x19: // rlinecurve
			{
				if (sp < 8)
				{
					return STBTT__CSERR("rlinecurve stack");
				}

				for (; i + 1 < sp - 6; i += 2)
				{
					stbtt__csctx_rline_to(c, s[i], s[i + 1]);
				}

				if (i + 5 >= sp)
				{
					return STBTT__CSERR("rlinecurve stack");
				}

				stbtt__csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]);
				break;
			}
			case 0x1A: // vvcurveto
			{
				break;
			}
			case 0x1B: // hhcurveto
			{
				if (sp < 4)
				{
					return STBTT__CSERR("(vv|hh)curveto stack");
				}

				float f{};

				if ((sp & 1) != 0)
				{
					f = s[i];
					++i;
				}

				for (; i + 3 < sp; i += 4)
				{
					b0 == 0x1B ? stbtt__csctx_rccurve_to(c, s[i], f, s[i + 1], s[i + 2], s[i + 3], 0.0F) : stbtt__csctx_rccurve_to(c, f, s[i], s[i + 1], s[i + 2], 0.0F, s[i + 3]);
					f = 0.0F;
				}
				break;
			}
			case 0x0A: // callsubr
			{
				if (has_subrs == 0)
				{
					if (info.fdselect.size != 0)
					{
						subrs = stbtt__cid_get_glyph_subrs(info, glyph_index);
					}

					has_subrs = 1;
				}
			}
			// fallthrough
			case 0x1D: // callgsubr
			{
				if (sp < 1)
				{
					return STBTT__CSERR("call(g|)subr stack");
				}

				if (subr_stack_height >= 10)
				{
					return STBTT__CSERR("recursion limit");
				}

				const std::int_fast32_t v{ static_cast<std::int_fast32_t>(s[--sp]) };
				subr_stack[subr_stack_height++] = b;
				b = stbtt__get_subr(b0 == 0x0A ? subrs : info.gsubrs, v);

				if (b.size == 0)
				{
					return STBTT__CSERR("subr not found");
				}

				b.cursor = 0;
				break;
			}
			case 0x0B: // return
			{
				if (subr_stack_height <= 0)
				{
					return STBTT__CSERR("return outside subr");
				}

				b = subr_stack[--subr_stack_height];
				break;
			}
			case 0x0E: // endchar
			{
				stbtt__csctx_close_shape(c);
				return 1;
			}
			case 0x0C: // two-byte escape
			{
				switch (stbtt__buf_get8(b))
				{
					case 0x22: // hflex
					{
						if (sp < 7)
						{
							return STBTT__CSERR("hflex stack");
						}

						stbtt__csctx_rccurve_to(c, s[0], 0.0F, s[1], s[2], s[3], 0.0F);
						stbtt__csctx_rccurve_to(c, s[4], 0.0F, s[5], -s[2], s[6], 0.0F);
						break;
					}
					case 0x23: // flex
					{
						if (sp < 13)
						{
							return STBTT__CSERR("flex stack");
						}

						//fd is s[12]
						stbtt__csctx_rccurve_to(c, s[0], s[1], s[2], s[3], s[4], s[5]);
						stbtt__csctx_rccurve_to(c, s[6], s[7], s[8], s[9], s[10], s[11]);
						break;
					}
					case 0x24: // hflex1
					{
						if (sp < 9)
						{
							return STBTT__CSERR("hflex1 stack");
						}

						stbtt__csctx_rccurve_to(c, s[0], s[1], s[2], s[3], s[4], 0.0F);
						stbtt__csctx_rccurve_to(c, s[5], 0.0F, s[6], s[7], s[8], -(s[1] + s[3] + s[7]));
						break;
					}
					case 0x25: // flex1
					{
						if (sp < 11)
						{
							return STBTT__CSERR("flex1 stack");
						}

						const float dx{ s[0] + s[2] + s[4] + s[6] + s[8] };
						const float dy{ s[1] + s[3] + s[5] + s[7] + s[9] };

						std::abs(dx) > std::abs(dy) ? s[10] = -dy : s[10] = -dx;

						stbtt__csctx_rccurve_to(c, s[0], s[1], s[2], s[3], s[4], s[5]);
						stbtt__csctx_rccurve_to(c, s[6], s[7], s[8], s[9], s[10], s[10]);
						break;
					}
					default:
					{
						return STBTT__CSERR("unimplemented");
						break;
					}
				}
				break;
			}
			default:
			{
				if (b0 != 255 && b0 != 28 && (b0 < 32 || b0 > 254)) //-V560
				{
					return STBTT__CSERR("reserved operator");
				}

				// push immediate
				float f{};
				b0 == 255 ? f = static_cast<float>(stbtt__buf_get(b, 4)) / 65536.0F : (stbtt__buf_skip(b, -1), f = static_cast<float>(stbtt__cff_int(b)));

				if (sp >= 48)
				{
					return STBTT__CSERR("push stack overflow");
				}

				s[sp++] = f;
				break;
			}
		}
	}

	return STBTT__CSERR("no endchar");

	#undef STBTT__CSERR
}

inline static std::int_fast32_t stbtt__GetGlyphShapeT2(const stbtt_fontinfo& info, const std::int_fast32_t glyph_index, std::vector<stbtt_vertex>& pvertices)
{
	// runs the charstring twice, once to count and once to output (to avoid realloc)
	stbtt__csctx count_ctx{ 1, 0, 0.0F, 0.0F, 0.0F, 0.0F, 0, 0, 0, 0, {}, 0 };

	if (stbtt__run_charstring(info, glyph_index, count_ctx) != 0)
	{
		pvertices.resize(static_cast<std::size_t>(count_ctx.num_vertices));
		stbtt__csctx output_ctx{};

		if (stbtt__run_charstring(info, glyph_index, output_ctx) != 0)
		{
			return output_ctx.num_vertices;
		}
	}

	return 0;
}

inline static std::int_fast32_t stbtt__GetGlyphInfoT2(const stbtt_fontinfo& info, const std::int_fast32_t glyph_index, std::int_fast32_t& x0, std::int_fast32_t& y0, std::int_fast32_t& x1, std::int_fast32_t& y1)
{
	stbtt__csctx c{ 1, 0, 0.0F, 0.0F, 0.0F, 0.0F, 0, 0, 0, 0, {}, 0 };
	const std::int_fast32_t r{ stbtt__run_charstring(info, glyph_index, c) };

	x0 = r != 0 ? c.min_x : 0;
	y0 = r != 0 ? c.min_y : 0;
	x1 = r != 0 ? c.max_x : 0;
	y1 = r != 0 ? c.max_y : 0;

	return r != 0 ? c.num_vertices : 0;
}

inline std::int_fast32_t stbtt_GetGlyphShape(const stbtt_fontinfo& info, const std::int_fast32_t glyph_index, std::vector<stbtt_vertex>& pvertices)
{
	if (info.cff.size == 0)
	{
		return stbtt__GetGlyphShapeTT(info, glyph_index, pvertices);
	}

	return stbtt__GetGlyphShapeT2(info, glyph_index, pvertices);
}

inline void stbtt_GetGlyphHMetrics(const stbtt_fontinfo& info, const std::int_fast32_t glyph_index, std::int_fast32_t& advanceWidth, std::int_fast32_t& leftSideBearing)
{
	if (const std::int_fast32_t numOfLongHorMetrics{ ttUSHORT(info.data.data() + info.hhea + 34) }; glyph_index < numOfLongHorMetrics)
	{
		advanceWidth = ttSHORT(info.data.data() + info.hmtx + 4 * glyph_index);
		leftSideBearing = ttSHORT(info.data.data() + info.hmtx + 4 * glyph_index + 2);
	}
	else
	{
		advanceWidth = ttSHORT(info.data.data() + info.hmtx + 4 * (numOfLongHorMetrics - 1));
		leftSideBearing = ttSHORT(info.data.data() + info.hmtx + 4 * numOfLongHorMetrics + 2 * (glyph_index - numOfLongHorMetrics));
	}
}

inline float stbtt_ScaleForPixelHeight(const stbtt_fontinfo& info, const float height)
{
	return height / static_cast<float>((ttSHORT(info.data.data() + info.hhea + 4) - ttSHORT(info.data.data() + info.hhea + 6)));
}

//////////////////////////////////////////////////////////////////////////////
//
// antialiasing software rasterizer
//

inline void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo& font, const std::int_fast32_t glyph, const float scale_x, const float scale_y, const float shift_x, const float shift_y, std::int_fast32_t& ix0, std::int_fast32_t& iy0, std::int_fast32_t& ix1, std::int_fast32_t& iy1)
{
	std::int_fast32_t x0{};
	std::int_fast32_t y0{};
	std::int_fast32_t x1{};
	std::int_fast32_t y1{};

	if (stbtt_GetGlyphBox(font, glyph, x0, y0, x1, y1) == 0)
	{
		// e.g. space character
		ix0 = 0;
		iy0 = 0;
		ix1 = 0;
		iy1 = 0;
	}
	else
	{
		// move to integral bboxes (treating pixels as little squares, what pixels get touched)?
		ix0 = static_cast<std::int_fast32_t>(std::floorf(x0 * scale_x + shift_x));
		iy0 = static_cast<std::int_fast32_t>(std::floorf(-y1 * scale_y + shift_y));
		ix1 = static_cast<std::int_fast32_t>(std::ceilf(x1 * scale_x + shift_x));
		iy1 = static_cast<std::int_fast32_t>(std::ceilf(-y0 * scale_y + shift_y));
	}
}

inline void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo& font, const std::int_fast32_t glyph, const float scale_x, const float scale_y, std::int_fast32_t& ix0, std::int_fast32_t& iy0, std::int_fast32_t& ix1, std::int_fast32_t& iy1)
{
   stbtt_GetGlyphBitmapBoxSubpixel(font, glyph, scale_x, scale_y, 0.0F, 0.0F, ix0, iy0, ix1, iy1);
}

inline void stbtt_GetCodepointBitmapBoxSubpixel(const stbtt_fontinfo& font, const std::int_fast32_t codepoint, const float scale_x, const float scale_y, const float shift_x, const float shift_y, std::int_fast32_t& ix0, std::int_fast32_t& iy0, std::int_fast32_t& ix1, std::int_fast32_t& iy1)
{
   stbtt_GetGlyphBitmapBoxSubpixel(font, stbtt_FindGlyphIndex(font,codepoint), scale_x, scale_y, shift_x, shift_y, ix0, iy0, ix1, iy1);
}

inline void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo& font, const std::int_fast32_t codepoint, const float scale_x, const float scale_y, std::int_fast32_t& ix0, std::int_fast32_t& iy0, std::int_fast32_t& ix1, std::int_fast32_t& iy1)
{
   stbtt_GetCodepointBitmapBoxSubpixel(font, codepoint, scale_x, scale_y, 0.0F, 0.0F, ix0, iy0, ix1, iy1);
}

//////////////////////////////////////////////////////////////////////////////
//
//  Rasterizer

struct stbtt__hheap_chunk
{
	struct stbtt__hheap_chunk* next{};
};

struct stbtt__hheap
{
	struct stbtt__hheap_chunk* head{};
	void* first_free{};
	std::int_fast32_t num_remaining_in_head_chunk{};
};

inline static void *stbtt__hheap_alloc(stbtt__hheap& hh, const size_t size)
{
	if (hh.first_free != nullptr)
	{
		void* p{ hh.first_free };
		hh.first_free = *reinterpret_cast<void**>(p);
		return p;
	}

	if (hh.num_remaining_in_head_chunk == 0)
	{
		const std::int_fast32_t count{ (size < 32 ? 2000 : size < 128 ? 800 : 100) };
		stbtt__hheap_chunk* c{ new stbtt__hheap_chunk[sizeof(stbtt__hheap_chunk) + size * count] };

		if (c == nullptr) //-V668
		{
			return nullptr;
		}

		c->next = hh.head;
		hh.head = c;
		hh.num_remaining_in_head_chunk = count;
	}

	--hh.num_remaining_in_head_chunk;
	return reinterpret_cast<char*>(hh.head) + sizeof(stbtt__hheap_chunk) + size * hh.num_remaining_in_head_chunk;
}

inline static void stbtt__hheap_free(stbtt__hheap& hh, void* p)
{
	*reinterpret_cast<void**>(p) = hh.first_free;
	hh.first_free = p;
}

inline static void stbtt__hheap_cleanup(stbtt__hheap& hh, void* userdata)
{
	stbtt__hheap_chunk* c{ hh.head };

	while (c != nullptr)
	{
		stbtt__hheap_chunk* n{ c->next };
		static_cast<void>(userdata), free(c);
		c = n;
	}
}

struct stbtt__edge
{
	float x0{};
	float y0{};
	float x1{};
	float y1{};
	std::int_fast32_t invert{};
};

struct stbtt__active_edge
{
	struct stbtt__active_edge* next{};
	float fx{};
	float fdx{};
	float fdy{};
	float direction{};
	float sy{};
	float ey{};
};

inline static stbtt__active_edge* stbtt__new_active(stbtt__hheap& hh, stbtt__edge* e, const std::int_fast32_t off_x, const float start_point)
{
	stbtt__active_edge* z{ static_cast<stbtt__active_edge*>(stbtt__hheap_alloc(hh, sizeof(*z))) };

	if (z == nullptr)
	{
		return z;
	}

	const float dxdy{ (e->x1 - e->x0) / (e->y1 - e->y0) };

	z->fdx = dxdy;
	z->fdy = std::abs(dxdy) > FLT_EPSILON ? (1.0F / dxdy) : 0.0F;
	z->fx = e->x0 + dxdy * (start_point - e->y0);
	z->fx -= off_x;
	z->direction = (e->invert != 0) ? 1.0F : -1.0F;
	z->sy = e->y0;
	z->ey = e->y1;
	z->next = nullptr;

	return z;
}

// the edge passed in here does not cross the vertical line at x or the vertical line at x+1
// (i.e. it has already been clipped to those)
inline static void stbtt__handle_clipped_edge(float* scanline, const std::int_fast32_t x, stbtt__active_edge* e, float x0, float y0, float x1, float y1)
{
	if (std::abs(y0 - y1) < FLT_EPSILON)
	{
		return;
	}

	if (y0 > e->ey)
	{
		return;
	}

	if (y1 < e->sy)
	{
		return;
	}

	if (y0 < e->sy)
	{
		x0 += (x1 - x0) * (e->sy - y0) / (y1 - y0);
		y0 = e->sy;
	}

	if (y1 > e->ey)
	{
		x1 += (x1 - x0) * (e->ey - y1) / (y1 - y0);
		y1 = e->ey;
	}

	if (x0 <= x && x1 <= x)
	{
		scanline[x] += e->direction * (y1 - y0);
	}
	else if (x0 >= static_cast<float>(x) + 1.0F && x1 >= static_cast<float>(x) + 1.0F)
	{
		// dummy
	}
	else
	{
		scanline[x] += e->direction * (y1 - y0) * (1.0F - ((x0 - x) + (x1 - x)) / 2.0F); // coverage = 1 - average x position
	}
}

inline static void stbtt__fill_active_edges_new(std::vector<float>& scanline, float* scanline_fill, const std::int_fast32_t len, stbtt__active_edge* e, const float y_top)
{
	const float y_bottom{ y_top + 1.0F };

	while (e != nullptr)
	{
		// brute force every pixel

		// compute intersection points with top & bottom

		if (std::fabs(e->fdx) < FLT_EPSILON)
		{
			if (const float x0{ e->fx }; x0 < len)
			{
				if (x0 >= 0.0F)
				{
					stbtt__handle_clipped_edge(scanline.data(), static_cast<std::int_fast32_t>(x0), e, x0, y_top, x0, y_bottom);
					stbtt__handle_clipped_edge(scanline_fill - 1, static_cast<std::int_fast32_t>(x0) + 1, e, x0, y_top, x0, y_bottom);
				}
				else
				{
					stbtt__handle_clipped_edge(scanline_fill - 1, 0, e, x0, y_top, x0, y_bottom);
				}
			}
		}
		else
		{
			float x0{ e->fx };
			const float dx{ e->fdx };
			float xb{ x0 + dx };
			float x_top{};
			float x_bottom{};
			float sy0{};
			float sy1{};

			// compute endpoints of line segment clipped to this scanline (if the
			// line segment starts on this scanline. x0 is the intersection of the
			// line with y_top, but that may be off the line segment.
			e->sy > y_top ? (x_top = x0 + dx * (e->sy - y_top), sy0 = e->sy) : (x_top = x0, sy0 = y_top);
			e->ey < y_bottom ? (x_bottom = x0 + dx * (e->ey - y_top), sy1 = e->ey) : (x_bottom = xb, sy1 = y_bottom);

			if (x_top >= 0.0F && x_bottom >= 0.0F && x_top < len && x_bottom < len)
			{
				// from here on, we don't have to range check x values
				if (float dy{ e->fdy }; static_cast<std::int_fast32_t>(x_top) == static_cast<std::int_fast32_t>(x_bottom))
				{
					// simple case, only spans one pixel
					const std::int_fast32_t x{ static_cast<std::int_fast32_t>(x_top) };
					const float height{ sy1 - sy0 };

					scanline[x] += e->direction * (1 - ((x_top - x) + (x_bottom - x)) / 2) * height;
					scanline_fill[x] += e->direction * height; // everything right of this pixel is filled
				}
				else
				{
					// covers 2+ pixels
					if (x_top > x_bottom)
					{
						// flip scanline vertically; signed area is the same
						sy0 = y_bottom - (sy0 - y_top);
						sy1 = y_bottom - (sy1 - y_top);

						std::swap(sy0, sy1);
						std::swap(x_bottom, x_top);
						std::swap(x0, xb);

						dy = -dy;
					}

					const std::int_fast32_t x1{ static_cast<std::int_fast32_t>(x_top) };
					const std::int_fast32_t x2{ static_cast<std::int_fast32_t>(x_bottom) };
					// compute intersection with y axis at x1+1
					float y_crossing{ (static_cast<float>(x1) + 1.0F - x0) * dy + y_top };
					const float sign{ e->direction };
					// area of the rectangle covered from y0..y_crossing
					float area{ sign * (y_crossing - sy0) };
					// area of the triangle (x_top,y0), (x+1,y0), (x+1,y_crossing)
					scanline[x1] += area * (1.0F - ((x_top - static_cast<float>(x1)) + (static_cast<float>(x1) + 1.0F - static_cast<float>(x1))) / 2.0F);
					const float step{ sign * dy };

					for (std::int_fast32_t x{ x1 + 1 }; x < x2; ++x)
					{
						scanline[x] += area + step / 2.0F;
						area += step;
					}

					y_crossing += dy * (static_cast<float>(x2) - (static_cast<float>(x1) + 1.0F));
					scanline[x2] += area + sign * (1.0F - ((static_cast<float>(x2) - static_cast<float>(x2)) + (x_bottom - static_cast<float>(x2))) / 2.0F) * (sy1 - y_crossing);
					scanline_fill[x2] += sign * (sy1 - sy0);
				}
			}
			else
			{
				// if edge goes outside of box we're drawing, we require
				// clipping logic. since this does not match the intended use
				// of this library, we use a different, very slow brute
				// force implementation
				for (std::int_fast32_t x{}; x < len; ++x)
				{
					// cases:
					//
					// there can be up to two intersections with the pixel. any intersection
					// with left or right edges can be handled by splitting into two (or three)
					// regions. intersections with top & bottom do not necessitate case-wise logic.
					//
					// the old way of doing this found the intersections with the left & right edges,
					// then used some simple logic to produce up to three segments in sorted order
					// from top-to-bottom. however, this had a problem: if an x edge was epsilon
					// across the x border, then the corresponding y position might not be distinct
					// from the other y segment, and it might ignored as an empty segment. to avoid
					// that, we need to explicitly produce segments based on x positions.

					const float x1{ static_cast<float>(x) };
					const float x2{ static_cast<float>(x) + 1.0F };
					const float y1{ (static_cast<float>(x) - x0) / dx + y_top };
					const float y2{ (static_cast<float>(x) + 1.0F - x0) / dx + y_top };

					if (x0 < x1 && xb > x2)
					{	// three segments descending down-right
						stbtt__handle_clipped_edge(scanline.data(), x, e, x0, y_top, x1, y1);
						stbtt__handle_clipped_edge(scanline.data(), x, e, x1, y1, x2, y2);
						stbtt__handle_clipped_edge(scanline.data(), x, e, x2, y2, xb, y_bottom);
					}
					else if (xb < x1 && x0 > x2)
					{	// three segments descending down-left
						stbtt__handle_clipped_edge(scanline.data(), x, e, x0, y_top, x2, y2);
						stbtt__handle_clipped_edge(scanline.data(), x, e, x2, y2, x1, y1);
						stbtt__handle_clipped_edge(scanline.data(), x, e, x1, y1, xb, y_bottom);
					}
					else if ((x0 < x1 && xb > x1) || (xb < x1 && x0 > x1))
					{	// two segments across x, down-right
						stbtt__handle_clipped_edge(scanline.data(), x, e, x0, y_top, x1, y1);
						stbtt__handle_clipped_edge(scanline.data(), x, e, x1, y1, xb, y_bottom);
					}
					else if ((x0 < x2 && xb > x2) || (xb < x2 && x0 > x2))
					{	// two segments across x+1, down-right
						stbtt__handle_clipped_edge(scanline.data(), x, e, x0, y_top, x2, y2);
						stbtt__handle_clipped_edge(scanline.data(), x, e, x2, y2, xb, y_bottom);
					}
					else
					{	// one segment
						stbtt__handle_clipped_edge(scanline.data(), x, e, x0, y_top, xb, y_bottom);
					}
				}
			}
		}

		e = e->next;
	}
}

// directly AA rasterize edges w/o supersampling
inline static void stbtt__rasterize_sorted_edges(stbtt__bitmap& result, stbtt__edge* e, const std::int_fast32_t n, const std::int_fast32_t off_x, const std::int_fast32_t off_y, void* userdata)
{
	stbtt__hheap hh{};
	stbtt__active_edge* active{};
	std::vector<float> scanline(129);

	if (result.w > 64)
	{
		scanline.resize(static_cast<std::size_t>(result.w) * 2 + 1);
	}

	std::vector<float> scanline2(scanline.size() + static_cast<std::size_t>(result.w));
	std::int_fast32_t y{ off_y };

	e[n].y0 = static_cast<float>(off_y + result.h) + 1.0F;

	std::int_fast32_t j{};

	while (j < result.h)
	{
		// find center of pixel for this scanline
		const float scan_y_top{ static_cast<float>(y) };
		const float scan_y_bottom{ static_cast<float>(y) + 1.0F };

		stbtt__active_edge** step{ &active };

		std::fill(scanline.begin(), scanline.end(), 0.0F);
		std::fill(scanline2.begin(), scanline2.end(), 0.0F);

		// update all active edges;
		// remove all active edges that terminate before the top of this scanline
		while (*step != nullptr) //-V712
		{
			if (stbtt__active_edge* z{ *step }; z->ey <= scan_y_top)
			{
				*step = z->next; // delete from list
				z->direction = 0;
				stbtt__hheap_free(hh, z);
			}
			else
			{
				step = &((*step)->next); // advance through list
			}
		}

		// insert all edges that start before the bottom of this scanline
		while (e->y0 <= scan_y_bottom)
		{
			if (std::fabs(e->y0 - e->y1) >= FLT_EPSILON)
			{
				if (stbtt__active_edge* z{ stbtt__new_active(hh, e, off_x, scan_y_top) }; z != nullptr)
				{
					if (j == 0 && off_y != 0)
					{
						if (z->ey < scan_y_top)
						{
							// this can happen due to subpixel positioning and some kind of fp rounding error i think
							z->ey = scan_y_top;
						}
					}

					// insert at front
					z->next = active;
					active = z;
				}
			}

			++e;
		}

		// now process all active edges
		if (active != nullptr)
		{
			stbtt__fill_active_edges_new(scanline, scanline2.data() + 1, result.w, active, scan_y_top);
		}

		{
			float sum{};

			for (std::int_fast32_t i{}; i < result.w; ++i)
			{
				sum += scanline2[i];
				const std::int_fast32_t m{ std::clamp(static_cast<std::int_fast32_t>(std::round(std::abs(scanline[i] + sum) * 255.0F)), 0, 255) };
				result.pixels[j * result.stride + i] = static_cast<unsigned char>(m);
			}
		}

		// advance all the edges
		step = &active;

		while (*step != nullptr)
		{
			stbtt__active_edge* z{ *step };
			z->fx += z->fdx; // advance to position for current scanline
			step = &((*step)->next); // advance through list
		}

		++y;
		++j;
	}

	stbtt__hheap_cleanup(hh, userdata);
}

inline static void stbtt__sort_edges_ins_sort(std::vector<stbtt__edge>& p, const std::int_fast32_t n)
{
	for (std::int_fast32_t i{ 1 }; i < n; ++i)
	{
		const stbtt__edge t{ p[i] };
		const stbtt__edge* a{ &t };
		std::int_fast32_t j{ i };

		while (j > 0) //-V712 //-V654
		{
			if (stbtt__edge* b{ &p[j - 1] }; !(a->y0 < b->y0))
			{
				break;
			}

			p[j] = p[j - 1];
			--j;
		}

		if (i != j)
		{
			p[j] = t;
		}
	}
}

inline static void stbtt__sort_edges_quicksort(stbtt__edge* p, std::int_fast32_t n)
{
	// threshold for transitioning to insertion sort
	while (n > 12)
	{
		// compute median of three
		const std::int_fast32_t m{ n >> 1 };
		const bool c01{ (&p[0])->y0 < (&p[m])->y0 };
		const bool c12{ (&p[m])->y0 < (&p[n - 1])->y0 };

		// if 0 >= mid >= end, or 0 < mid < end, then use mid
		if (c01 != c12)
		{
			// otherwise, we'll need to swap something else to middle
			const bool c{ (&p[0])->y0 < (&p[n - 1])->y0 };

			// 0>mid && mid<n:  0>n => n; 0<n => 0
			// 0<mid && mid>n:  0>n => 0; 0<n => n
			std::swap(p[(c == c12) ? 0 : n - 1], p[m]);
		}

		// now p[m] is the median-of-three
		// swap it to the beginning so it won't move around
		std::swap(p[0], p[m]);

		/* partition loop */
		std::int_fast32_t i{ 1 };
		std::int_fast32_t j{ n - 1 };

		for(;;)
		{
			// handling of equality is crucial here
			// for sentinels & efficiency with duplicates
			for (;;++i)
			{
				if (!((&p[i])->y0 < (&p[0])->y0))
				{
					break;
				}
			}

			for (;;--j)
			{
				if (!((&p[0])->y0 < (&p[j])->y0))
				{
					break;
				}
			}

			// make sure we haven't crossed
			if (i >= j)
			{
				break;
			}

			std::swap(p[i], p[j]);

			++i;
			--j;
		}

		// recurse on smaller side, iterate on larger
		if (j < (n - i))
		{
			stbtt__sort_edges_quicksort(p, j);
			p = p + i;
			n = n - i;
		}
		else
		{
			stbtt__sort_edges_quicksort(p + i, n - i);
			n = j;
		}
	}
}

inline static void stbtt__sort_edges(std::vector<stbtt__edge>& p, const std::int_fast32_t n)
{
	stbtt__sort_edges_quicksort(p.data(), n);
	stbtt__sort_edges_ins_sort(p, n);
}

struct stbtt__point
{
	float x{};
	float y{};
};

inline static void stbtt__rasterize(stbtt__bitmap& result, std::vector<stbtt__point>& pts, const std::vector<std::int_fast32_t>& wcount, const std::int_fast32_t windings, const float scale_x, const float scale_y, const float shift_x, const float shift_y, const std::int_fast32_t off_x, const std::int_fast32_t off_y, const std::int_fast32_t invert, void* userdata)
{
	// now we have to blow out the windings into explicit edge lists
	std::int_fast32_t n{};

	for (std::int_fast32_t i{}; i < windings; ++i)
	{
		n += wcount[i];
	}

	std::vector<stbtt__edge> e(n + 1); // add an extra one as a sentinel

	n = 0;

	const float y_scale_inv{ (invert != 0) ? -scale_y : scale_y };

	for (std::int_fast32_t m{}, i{}; i < windings; ++i)
	{
		const std::vector<stbtt__point> p(pts.begin() + m, pts.end());
		m += wcount[i];

		for (std::int_fast32_t j{ wcount[i] - 1 }, k{}; k < wcount[i]; j = k++)
		{
			std::int_fast32_t a{ k };
			std::int_fast32_t b{ j };

			// skip the edge if horizontal
			if (fabs(p[j].y - p[k].y) < FLT_EPSILON)
			{
				continue;
			}

			// add edge from j to k to the list
			e[n].invert = 0;

			if (invert != 0 ? p[j].y > p[k].y : p[j].y < p[k].y)
			{
				e[n].invert = 1;
				a = j;
				b = k;
			}

			e[n].x0 = p[a].x * scale_x + shift_x;
			e[n].y0 = (p[a].y * y_scale_inv + shift_y);
			e[n].x1 = p[b].x * scale_x + shift_x;
			e[n].y1 = (p[b].y * y_scale_inv + shift_y);

			++n;
		}
	}

	// now sort the edges by their highest point (should snap to integer, and then by x)
	stbtt__sort_edges(e, n);

	// now, traverse the scanlines and find the intersections on each scanline, use xor winding rule
	stbtt__rasterize_sorted_edges(result, e.data(), n, off_x, off_y, userdata);
}

inline static void stbtt__add_point(std::vector<stbtt__point>& points, const std::int_fast32_t n, const float x, const float y)
{
	if (points.empty())
	{
		return; // during first pass, it's unallocated
	}

	points[n].x = x;
	points[n].y = y;
}

// tessellate until threshold p is happy...
inline static std::int_fast32_t stbtt__tesselate_curve(std::vector<stbtt__point>& points, std::int_fast32_t& num_points, const float x0, const float y0, const float x1, const float y1, const float x2, const float y2, const float objspace_flatness_squared, const std::int_fast32_t n)
{
	if (n > 16) // 65536 segments on one curve better be enough!
	{
		return 1;
	}

	// midpoint
	const float mx{ (x0 + 2.0F * x1 + x2) / 4.0F };
	const float my{ (y0 + 2.0F * y1 + y2) / 4.0F };
	// versus directly drawn line
	const float dx{ (x0 + x2) / 2.0F - mx };
	const float dy{ (y0 + y2) / 2.0F - my };

	if (dx * dx + dy * dy > objspace_flatness_squared) // half-pixel error allowed... need to be smaller if AA
	{
		stbtt__tesselate_curve(points, num_points, x0, y0, (x0 + x1) / 2.0F, (y0 + y1) / 2.0F, mx, my, objspace_flatness_squared, n + 1);
		stbtt__tesselate_curve(points, num_points, mx, my, (x1 + x2) / 2.0F, (y1 + y2) / 2.0F, x2, y2, objspace_flatness_squared, n + 1);
	}
	else
	{
		stbtt__add_point(points, num_points, x2, y2);
		num_points = num_points + 1;
	}

	return 1;
}

inline static void stbtt__tesselate_cubic(std::vector<stbtt__point>& points, std::int_fast32_t& num_points, const float x0, const float y0, const float x1, const float y1, const float x2, const float y2, const float x3, const float y3, const float objspace_flatness_squared, const std::int_fast32_t n)
{
	if (n > 16) // 65536 segments on one curve better be enough!
	{
		return;
	}

	const float dx0{ x1 - x0 };
	const float dy0{ y1 - y0 };
	const float dx1{ x2 - x1 };
	const float dy1{ y2 - y1 };
	const float dx2{ x3 - x2 };
	const float dy2{ y3 - y2 };
	const float dx{ x3 - x0 };
	const float dy{ y3 - y0 };
	const float longlen{ std::sqrtf(dx0 * dx0 + dy0 * dy0) + std::sqrtf(dx1 * dx1 + dy1 * dy1) + std::sqrtf(dx2 * dx2 + dy2 * dy2) };
	const float shortlen{ std::sqrtf(dx * dx + dy * dy) };
	const float flatness_squared{ longlen * longlen - shortlen * shortlen };

	if (flatness_squared > objspace_flatness_squared)
	{
		const float x01{ (x0 + x1) / 2.0F };
		const float y01{ (y0 + y1) / 2.0F };
		const float x12{ (x1 + x2) / 2.0F };
		const float y12{ (y1 + y2) / 2.0F };
		const float x23{ (x2 + x3) / 2.0F };
		const float y23{ (y2 + y3) / 2.0F };

		const float xa{ (x01 + x12) / 2.0F };
		const float ya{ (y01 + y12) / 2.0F };
		const float xb{ (x12 + x23) / 2.0F };
		const float yb{ (y12 + y23) / 2.0F };

		const float mx{ (xa + xb) / 2.0F };
		const float my{ (ya + yb) / 2.0F };

		stbtt__tesselate_cubic(points, num_points, x0, y0, x01, y01, xa, ya, mx, my, objspace_flatness_squared, n + 1);
		stbtt__tesselate_cubic(points, num_points, mx, my, xb, yb, x23, y23, x3, y3, objspace_flatness_squared, n + 1);
	}
	else
	{
		stbtt__add_point(points, num_points, x3, y3);
		++num_points;
	}
}

// returns number of contours
inline static std::vector<stbtt__point> stbtt_FlattenCurves(stbtt_vertex* vertices, const std::int_fast32_t num_verts, const float objspace_flatness, std::vector<std::int_fast32_t>& contour_lengths, std::int_fast32_t& num_contours)
{
	std::int_fast32_t n{};

	// count how many "moves" there are to get the contour count
	for (std::int_fast32_t i{}; i < num_verts; ++i)
	{
		if (vertices[i].type == static_cast<unsigned char>(GlyphShapeType::STBTT_vmove))
		{
			++n;
		}
	}

	if (n == 0)
	{
		return {};
	}

	contour_lengths.resize(static_cast<std::size_t>(n));

	if (contour_lengths.empty())
	{
		num_contours = 0;
		return {};
	}

	num_contours = n;
	const float objspace_flatness_squared{ objspace_flatness * objspace_flatness };
	std::vector<stbtt__point> points{};
	std::int_fast32_t num_points{};
	std::int_fast32_t start{};

	// make two passes through the points so we don't need to realloc
	for (std::int_fast32_t pass{}; pass < 2; ++pass)
	{
		float x{};
		float y{};

		if (pass == 1)
		{
			points.resize(static_cast<std::size_t>(num_points));

			if (points.empty())
			{
				num_contours = 0;
				return {};
			}
		}

		num_points = 0;
		n= -1;

		for (std::int_fast32_t i{}; i < num_verts; ++i)
		{
			switch (vertices[i].type)
			{
				case static_cast<unsigned char>(GlyphShapeType::STBTT_vmove):
				{
					// start the next contour
					if (n >= 0)
					{
						contour_lengths[n] = num_points - start;
					}

					++n;
					start = num_points;

					x = vertices[i].x;
					y = vertices[i].y;
					stbtt__add_point(points, num_points++, x, y);
					break;
				}
				case static_cast<unsigned char>(GlyphShapeType::STBTT_vline):
				{
					x = vertices[i].x;
					y = vertices[i].y;
					stbtt__add_point(points, num_points++, x, y);
					break;
				}
				case static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve):
				{
					stbtt__tesselate_curve(points, num_points, x, y, vertices[i].cx, vertices[i].cy, vertices[i].x, vertices[i].y,	objspace_flatness_squared, 0);
					x = vertices[i].x;
					y = vertices[i].y;
					break;
				}
				case static_cast<unsigned char>(GlyphShapeType::STBTT_vcubic):
				{
					stbtt__tesselate_cubic(points, num_points, x, y, vertices[i].cx, vertices[i].cy, vertices[i].cx1, vertices[i].cy1, vertices[i].x, vertices[i].y, objspace_flatness_squared, 0);
					x = vertices[i].x;
					y = vertices[i].y;
					break;
				}
			}
		}

		contour_lengths[n] = num_points - start;
	}

	return points;
}

inline void stbtt_Rasterize(stbtt__bitmap& result, const float flatness_in_pixels, std::vector<stbtt_vertex>& vertices, const std::int_fast32_t num_verts, const float scale_x, const float scale_y, const float shift_x, const float shift_y, const std::int_fast32_t x_off, const std::int_fast32_t y_off, const std::int_fast32_t invert, std::shared_ptr<void> userdata)
{
	const float scale{ scale_x > scale_y ? scale_y : scale_x };
	std::int_fast32_t winding_count{};
	std::vector<std::int_fast32_t> winding_lengths{};

	std::vector<stbtt__point> windings{ stbtt_FlattenCurves(vertices.data(), num_verts, flatness_in_pixels / scale, winding_lengths, winding_count) };

	if (!windings.empty())
	{
		stbtt__rasterize(result, windings, winding_lengths, winding_count, scale_x, scale_y, shift_x, shift_y, x_off, y_off, invert, &userdata);
	}
}

inline void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo& info, unsigned char* output, std::int_fast32_t out_w, std::int_fast32_t out_h, std::int_fast32_t out_stride, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t glyph)
{
	std::vector<stbtt_vertex> vertices{};
	const std::int_fast32_t num_verts{ stbtt_GetGlyphShape(info, glyph, vertices) };
	std::int_fast32_t ix0{};
	std::int_fast32_t iy0{};
	std::int_fast32_t ix1{};
	std::int_fast32_t iy1{};

	stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, ix0, iy0, ix1, iy1);

	if (stbtt__bitmap gbm{ out_w, out_h, out_stride, output }; gbm.w != 0 && gbm.h != 0)
	{
		stbtt_Rasterize(gbm, 0.35F, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1, info.userdata);
	}
}

inline void stbtt_MakeGlyphBitmap(const stbtt_fontinfo& info, unsigned char* output, const std::int_fast32_t out_w, const std::int_fast32_t out_h, const std::int_fast32_t out_stride, const float scale_x, const float scale_y, const std::int_fast32_t glyph)
{
	stbtt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0F,0.0F, glyph);
}

//////////////////////////////////////////////////////////////////////////////
//
// bitmap baking
//
// This is SUPER-CRAPPY packing to keep source code small

inline static std::int_fast32_t stbtt_BakeFontBitmap_internal(std::vector<unsigned char>& data, const std::int_fast32_t offset, const float pixel_height, std::vector<unsigned char>& pixels, const std::int_fast32_t pw, const std::int_fast32_t ph, const std::int_fast32_t first_char, const std::int_fast32_t num_chars, std::vector<stbtt_bakedchar>& chardata)
{
	stbtt_fontinfo f{};

	if (stbtt_InitFont(f, data, offset) == 0)
	{
		return -1;
	}

	std::fill(pixels.begin(), pixels.end(), static_cast<unsigned char>(0));

	const float scale{ stbtt_ScaleForPixelHeight(f, pixel_height) };
	std::int_fast32_t x{ 1 };
	std::int_fast32_t y{ 1 };
	std::int_fast32_t bottom_y{ 1 };

	for (std::int_fast32_t i{}; i < num_chars; ++i)
	{
		std::int_fast32_t advance{};
		std::int_fast32_t lsb{};
		std::int_fast32_t x0{};
		std::int_fast32_t y0{};
		std::int_fast32_t x1{};
		std::int_fast32_t y1{};
		const std::int_fast32_t g{ stbtt_FindGlyphIndex(f, first_char + i) };

		stbtt_GetGlyphHMetrics(f, g, advance, lsb);
		stbtt_GetGlyphBitmapBox(f, g, scale, scale, x0, y0, x1, y1);

		const std::int_fast32_t gw{ x1 - x0 };
		const std::int_fast32_t gh{ y1 - y0 };

		if (x + gw + 1 >= pw)
		{
			y = bottom_y;
			x = 1; // advance to next row
		}

		if (y + gh + 1 >= ph) // check if it fits vertically AFTER potentially moving to next row
		{
			return -i;
		}

		stbtt_MakeGlyphBitmap(f, pixels.data() + x + y * pw, gw, gh, pw, scale, scale, g);

		chardata[i].x0 = x;
		chardata[i].y0 = y;
		chardata[i].x1 = x + gw;
		chardata[i].y1 = y + gh;
		chardata[i].xadvance = scale * advance;
		chardata[i].xoff = static_cast<float>(x0);
		chardata[i].yoff = static_cast<float>(y0);

		x += gw + 1;

		if (y + gh + 1 > bottom_y)
		{
			bottom_y = y + gh + 1;
		}
	}

	return bottom_y;
}

inline void stbtt_GetBakedQuad(const std::vector<stbtt_bakedchar>& chardata, const std::int_fast32_t pw, const std::int_fast32_t ph, const std::int_fast32_t char_index, float& xpos, float ypos, stbtt_aligned_quad& q, const std::int_fast32_t opengl_fillrule)
{
	const float d3d_bias{ (opengl_fillrule != 0) ? 0.0F : -0.5F };
	const float ipw{ 1.0F / pw };
	const float iph{ 1.0F / ph };
	const stbtt_bakedchar b{ chardata[char_index] };
	const std::int_fast32_t round_x{ static_cast<std::int_fast32_t>(std::roundf(xpos + b.xoff)) };
	const std::int_fast32_t round_y{ static_cast<std::int_fast32_t>(std::roundf(ypos + b.yoff)) };

	q.x0 = round_x + d3d_bias;
	q.y0 = round_y + d3d_bias;
	q.x1 = static_cast<float>(round_x + b.x1 - b.x0) + d3d_bias;
	q.y1 = static_cast<float>(round_y + b.y1 - b.y0) + d3d_bias;

	q.s0 = b.x0 * ipw;
	q.t0 = b.y0 * iph;
	q.s1 = b.x1 * ipw;
	q.t1 = b.y1 * iph;

	xpos += b.xadvance;
}

inline std::int_fast32_t stbtt_BakeFontBitmap(std::vector<unsigned char>& data, const std::int_fast32_t offset, const float pixel_height, std::vector<unsigned char>& pixels, const std::int_fast32_t pw, const std::int_fast32_t ph, const std::int_fast32_t first_char, const std::int_fast32_t num_chars, std::vector<stbtt_bakedchar>& chardata)
{
	return stbtt_BakeFontBitmap_internal(data, offset, pixel_height, pixels, pw, ph, first_char, num_chars, chardata);
}

inline std::int_fast32_t stbtt_InitFont(stbtt_fontinfo& info, std::vector<unsigned char>& data, const std::int_fast32_t offset)
{
	return stbtt_InitFont_internal(info, data, offset);
}

#endif // STB_TRUETYPE_IMPLEMENTATION

/*
------------------------------------------------------------------------------
Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/