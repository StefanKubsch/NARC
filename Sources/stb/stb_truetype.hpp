// stb_truetype.hpp - under development - public domain
//
// authored in 2019 by Stefan Kubsch
//
// a stripped down and modernised version of stb_truetype.h
//
// Conversion to C++
//
// WHAT I DID SO FAR:
//
//	- fixed all warnings reported by PVS-Studio and clang-tidy (use brackets etc.)
//	- replaced all "typedef" with "using"
//	- replaced "malloc/free" with "new/delete"
//	- replaced all C-style casts with modern-style C++ cast (e.g. static_cast)
//	- initialised all variables and narrowed their scopes if possible
//	- fixed all implicit conversions to "double" by consequent use of the float literal (e.g. 0.0F)
//	- made everything "const", what can be const
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
// TO DO:
//
//	- get rid of all these raw pointers
//	- work on replacing arrays with std::vector
//
// FURTHER NOTES:
//		- removed Rasterizer version 1
//		- removed all assert()
//		- no support for stb_rect_pack.h
//		- no implementation with other RAD tools
//		- removed font name matching API
//		- removed most of the "the macro-preprocessor-magic"
//
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
// USAGE
//
//   Include this file in whatever places need to refer to it. In ONE C/C++
//   file, write:
//      #define STB_TRUETYPE_IMPLEMENTATION
//   before the #include of this file. This expands out the actual
//   implementation into that C/C++ file.
//
//
//   Simple 3D API (don't ship this, but it's fine for tools and quick start)
//           stbtt_BakeFontBitmap()               -- bake a font to a bitmap for use as texture
//           stbtt_GetBakedQuad()                 -- compute quad to draw for a given char
//
//   Improved 3D API (more shippable):
//           stbtt_PackBegin()
//           stbtt_PackSetOversampling()          -- for improved quality on small fonts
//           stbtt_PackFontRanges()               -- pack and renders
//           stbtt_PackEnd()
//           stbtt_GetPackedQuad()
//
//   "Load" a font file from a memory buffer (you have to keep the buffer loaded)
//           stbtt_InitFont()
//           stbtt_GetFontOffsetForIndex()        -- indexing for TTC font collections
//           stbtt_GetNumberOfFonts()             -- number of fonts for TTC font collections
//
//   Render a unicode codepoint to a bitmap
//           stbtt_GetCodepointBitmap()           -- allocates and returns a bitmap
//           stbtt_MakeCodepointBitmap()          -- renders into bitmap you provide
//           stbtt_GetCodepointBitmapBox()        -- how big the bitmap must be
//
//   Character advance/positioning
//           stbtt_GetCodepointHMetrics()
//           stbtt_GetFontVMetrics()
//           stbtt_GetFontVMetricsOS2()
//           stbtt_GetCodepointKernAdvance()
//
//   Starting with version 1.06, the rasterizer was replaced with a new,
//   faster and generally-more-precise rasterizer. The new rasterizer more
//   accurately measures pixel coverage for anti-aliasing, except in the case
//   where multiple shapes overlap, in which case it overestimates the AA pixel
//   coverage. Thus, anti-aliasing of intersecting shapes may look wrong.
//
// ADDITIONAL DOCUMENTATION
//
//   Some important concepts to understand to use this library:
//
//      Codepoint
//         Characters are defined by unicode codepoints, e.g. 65 is
//         uppercase A, 231 is lowercase c with a cedilla, 0x7e30 is
//         the hiragana for "ma".
//
//      Glyph
//         A visual character shape (every codepoint is rendered as
//         some glyph)
//
//      Glyph index
//         A font-specific integer ID representing a glyph
//
//      Baseline
//         Glyph shapes are defined relative to a baseline, which is the
//         bottom of uppercase characters. Characters extend both above
//         and below the baseline.
//
//      Current Point
//         As you draw text to the screen, you keep track of a "current point"
//         which is the origin of each character. The current point's vertical
//         position is the baseline. Even "baked fonts" use this model.
//
//      Vertical Font Metrics
//         The vertical qualities of the font, used to vertically position
//         and space the characters. See docs for stbtt_GetFontVMetrics.
//
//      Font Size in Pixels or Points
//         The preferred interface for specifying font sizes in stb_truetype
//         is to specify how tall the font's vertical extent should be in pixels.
//         If that sounds good enough, skip the next paragraph.
//
//         Most font APIs instead use "points", which are a common typographic
//         measurement for describing font size, defined as 72 points per inch.
//         stb_truetype provides a point API for compatibility. However, true
//         "per inch" conventions don't make much sense on computer displays
//         since different monitors have different number of pixels per
//         inch. For example, Windows traditionally uses a convention that
//         there are 96 pixels per inch, thus making 'inch' measurements have
//         nothing to do with inches, and thus effectively defining a point to
//         be 1.333 pixels. Additionally, the TrueType font data provides
//         an explicit scale factor to scale a given font's glyphs to points,
//         but the author has observed that this scale factor is often wrong
//         for non-commercial fonts, thus making fonts scaled in points
//         according to the TrueType spec incoherently sized in practice.
//
// DETAILED USAGE:
//
//  Scale:
//    Select how high you want the font to be, in points or pixels.
//    Call ScaleForPixelHeight or ScaleForMappingEmToPixels to compute
//    a scale factor SF that will be used by all other functions.
//
//  Baseline:
//    You need to select a y-coordinate that is the baseline of where
//    your text will appear. Call GetFontBoundingBox to get the baseline-relative
//    bounding box for all characters. SF*-y0 will be the distance in pixels
//    that the worst-case character could extend above the baseline, so if
//    you want the top edge of characters to appear at the top of the
//    screen where y=0, then you would set the baseline to SF*-y0.
//
//  Current point:
//    Set the current point where the first character will appear. The
//    first character could extend left of the current point; this is font
//    dependent. You can either choose a current point that is the leftmost
//    point and hope, or add some padding, or check the bounding box or
//    left-side-bearing of the first character to be displayed and set
//    the current point based on that.
//
//  Displaying a character:
//    Compute the bounding box of the character. It will contain signed values
//    relative to <current_point, baseline>. I.e. if it returns x0,y0,x1,y1,
//    then the character should be displayed in the rectangle from
//    <current_point+SF*x0, baseline+SF*y0> to <current_point+SF*x1,baseline+SF*y1).
//
//  Advancing for the next character:
//    Call GlyphHMetrics, and compute 'current_point += SF * advance'.
//
//
// ADVANCED USAGE
//
//   Quality:
//
//    - Use the functions with Subpixel at the end to allow your characters
//      to have subpixel positioning. Since the font is anti-aliased, not
//      hinted, this is very import for quality. (This is not possible with
//      baked fonts.)
//
//    - Kerning is now supported, and if you're supporting subpixel rendering
//      then kerning is worth using to give your text a polished look.
//
//   Performance:
//
//    - Convert Unicode codepoints to glyph indexes and operate on the glyphs;
//      if you don't do this, stb_truetype is forced to do the conversion on
//      every call.
//
//    - There are a lot of memory allocations. We should modify it to take
//      a temp buffer and allocate from the temp buffer (without freeing),
//      should help performance a lot.
//
// NOTES
//
//   The system uses the raw data found in the .ttf file without changing it
//   and without building auxiliary data structures. This is a bit inefficient
//   on little-endian systems (the data is big-endian), but assuming you're
//   caching the bitmaps or glyph shapes this shouldn't be a big deal.
//
//   It appears to be very hard to programmatically determine what font a
//   given file is in a general way.
//
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
#include <vector>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////   INTERFACE
////
////

// private structure
using stbtt__buf = struct
{
	unsigned char* data{};
	std::int_fast32_t cursor{};
	std::int_fast32_t size{};
};

//////////////////////////////////////////////////////////////////////////////
//
// TEXTURE BAKING API
//
// If you use this API, you only have to call two functions ever.
//

using stbtt_bakedchar = struct
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

std::int_fast32_t stbtt_BakeFontBitmap(const unsigned char* data, std::int_fast32_t offset,			// font location (use offset=0 for plain .ttf)
                                float pixel_height,													// height of font in pixels
                                unsigned char* pixels, std::int_fast32_t pw, std::int_fast32_t ph,  // bitmap to be filled in
                                std::int_fast32_t first_char, std::int_fast32_t num_chars,          // characters to bake
                                stbtt_bakedchar* chardata);											// you allocate this, it's num_chars long
// if return is positive, the first unused row of the bitmap
// if return is negative, returns the negative of the number of characters that fit
// if return is 0, no characters fit and no rows were used
// This uses a very crappy packing.

using stbtt_aligned_quad = struct
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

void stbtt_GetBakedQuad(const stbtt_bakedchar* chardata, std::int_fast32_t pw, std::int_fast32_t ph,	// same data as above
                               std::int_fast32_t char_index,											// character to display
                               float* xpos, const float* ypos,											// pointers to current position in screen pixel space
                               stbtt_aligned_quad* q,													// output: quad to draw
                               std::int_fast32_t opengl_fillrule);										// true if opengl fill rule; false if DX9 or earlier
// Call GetBakedQuad with char_index = 'character - first_char', and it
// creates the quad you need to draw and advances the current position.
//
// The coordinate system used assumes y increases downwards.
//
// Characters will extend both above and below the current position;
// see discussion of "BASELINE" above.
//
// It's inefficient; you might want to c&p it and optimize it.

void stbtt_GetScaledFontVMetrics(const unsigned char* fontdata, std::int_fast32_t index, float size, float* ascent, float* descent, float* lineGap);
// Query the font vertical metrics without having to create a font first.

//////////////////////////////////////////////////////////////////////////////
//
// NEW TEXTURE BAKING API
//
// This provides options for packing multiple fonts into one atlas, not
// perfectly but better than nothing.

using stbtt_packedchar = struct
{
	// coordinates of bbox in bitmap
	std::int_fast32_t x0{};
	std::int_fast32_t y0{};
	std::int_fast32_t x1{};
	std::int_fast32_t y1{};
	float xoff{};
	float yoff{};
	float xadvance{};
	float xoff2{};
	float yoff2{};
};

using stbtt_pack_context = struct stbtt_pack_context;
using stbtt_fontinfo = struct stbtt_fontinfo;

#ifndef STB_RECT_PACK_VERSION
	using stbrp_rect = struct stbrp_rect;
#endif

std::int_fast32_t stbtt_PackBegin(stbtt_pack_context* spc, unsigned char* pixels, std::int_fast32_t pw, std::int_fast32_t ph, std::int_fast32_t stride_in_bytes, std::int_fast32_t padding, void* alloc_context);
// Initializes a packing context stored in the passed-in stbtt_pack_context.
// Future calls using this context will pack characters into the bitmap passed
// in here: a 1-channel bitmap that is width * height. stride_in_bytes is
// the distance from one row to the next (or 0 to mean they are packed tightly
// together). "padding" is the amount of padding to leave between each
// character (normally you want '1' for bitmaps you'll use as textures with
// bilinear filtering).
//
// Returns 0 on failure, 1 on success.

void stbtt_PackEnd(stbtt_pack_context* spc);
// Cleans up the packing context and frees all memory.

std::int_fast32_t stbtt_PackFontRange(stbtt_pack_context* spc, const unsigned char* fontdata, std::int_fast32_t font_index, float font_size, std::int_fast32_t first_unicode_codepoint_in_range, std::int_fast32_t num_chars_in_range, stbtt_packedchar* chardata_for_range);
// Creates character bitmaps from the font_index'th font found in fontdata (use
// font_index=0 if you don't know what that is). It creates num_chars_in_range
// bitmaps for characters with unicode values starting at first_unicode_char_in_range
// and increasing. Data for how to render them is stored in chardata_for_range;
// pass these to stbtt_GetPackedQuad to get back renderable quads.
//
// font_size is the full height of the character from ascender to descender,
// as computed by stbtt_ScaleForPixelHeight. To use a point size as computed
// by stbtt_ScaleForMappingEmToPixels, wrap the point size in STBTT_POINT_SIZE()
// and pass that result as 'font_size':
//       ...,                  20 , ... // font max minus min y is 20 pixels tall
//       ..., STBTT_POINT_SIZE(20), ... // 'M' is 20 pixels tall

using stbtt_pack_range = struct
{
	float font_size{};
	std::int_fast32_t first_unicode_codepoint_in_range{};		// if non-zero, then the chars are continuous, and this is the first codepoint
	std::int_fast32_t* array_of_unicode_codepoints{};			// if non-zero, then this is an array of unicode codepoints
	std::int_fast32_t num_chars{};
	stbtt_packedchar* chardata_for_range{};						// output
	// don't set these, they're used internally
	unsigned char h_oversample{};
	unsigned char v_oversample{};
};

std::int_fast32_t stbtt_PackFontRanges(stbtt_pack_context* spc, const unsigned char* fontdata, std::int_fast32_t font_index, stbtt_pack_range* ranges, std::int_fast32_t num_ranges);
// Creates character bitmaps from multiple ranges of characters stored in
// ranges. This will usually create a better-packed bitmap than multiple
// calls to stbtt_PackFontRange. Note that you can call this multiple
// times within a single PackBegin/PackEnd.

void stbtt_PackSetOversampling(stbtt_pack_context* spc, std::int_fast32_t h_oversample, std::int_fast32_t v_oversample);
// Oversampling a font increases the quality by allowing higher-quality subpixel
// positioning, and is especially valuable at smaller text sizes.
//
// This function sets the amount of oversampling for all following calls to
// stbtt_PackFontRange(s) or stbtt_PackFontRangesGatherRects for a given
// pack context. The default (no oversampling) is achieved by h_oversample=1
// and v_oversample=1. The total number of pixels required is
// h_oversample*v_oversample larger than the default; for example, 2x2
// oversampling requires 4x the storage of 1x1. For best results, render
// oversampled textures with bilinear filtering. Look at the readme in
// stb/tests/oversample for information about oversampled fonts
//
// To use with PackFontRangesGather etc., you must set it before calls
// call to PackFontRangesGatherRects.

void stbtt_PackSetSkipMissingCodepoints(stbtt_pack_context* spc, std::int_fast32_t skip);
// If skip != 0, this tells stb_truetype to skip any codepoints for which
// there is no corresponding glyph. If skip=0, which is the default, then
// codepoints without a glyph recived the font's "missing character" glyph,
// typically an empty box by convention.

void stbtt_GetPackedQuad(const stbtt_packedchar* chardata, std::int_fast32_t pw, std::int_fast32_t ph,	// same data as above
                               std::int_fast32_t char_index,											// character to display
                               float* xpos, const float* ypos,											// pointers to current position in screen pixel space
                               stbtt_aligned_quad* q,													// output: quad to draw
                               std::int_fast32_t align_to_integer);

std::int_fast32_t stbtt_PackFontRangesGatherRects(stbtt_pack_context* spc, const stbtt_fontinfo* info, stbtt_pack_range* ranges, std::int_fast32_t num_ranges, stbrp_rect* rects);
void stbtt_PackFontRangesPackRects(stbtt_pack_context* spc, stbrp_rect* rects, std::int_fast32_t num_rects);
std::int_fast32_t stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context* spc, const stbtt_fontinfo* info, stbtt_pack_range* ranges, std::int_fast32_t num_ranges, stbrp_rect* rects);
// Calling these functions in sequence is roughly equivalent to calling
// stbtt_PackFontRanges(). If you more control over the packing of multiple
// fonts, or if you want to pack custom data into a font texture, take a look
// at the source to of stbtt_PackFontRanges() and create a custom version
// using these functions, e.g. call GatherRects multiple times,
// building up a single array of rects, then call PackRects once,
// then call RenderIntoRects repeatedly. This may result in a
// better packing than calling PackFontRanges multiple times
// (or it may not).

// this is an opaque structure that you shouldn't mess with which holds
// all the context needed from PackBegin to PackEnd.
struct stbtt_pack_context
{
	void* user_allocator_context{};
	void* pack_info{};
	std::int_fast32_t width{};
	std::int_fast32_t height{};
	std::int_fast32_t stride_in_bytes{};
	std::int_fast32_t padding{};
	std::int_fast32_t skip_missing{};
	std::int_fast32_t h_oversample{};
	std::int_fast32_t v_oversample{};
	unsigned char* pixels{};
	void* nodes{};
};

//////////////////////////////////////////////////////////////////////////////
//
// FONT LOADING
//
//

std::int_fast32_t stbtt_GetNumberOfFonts(const unsigned char* data);
// This function will determine the number of fonts in a font file.  TrueType
// collection (.ttc) files may contain multiple fonts, while TrueType font
// (.ttf) files only contain one font. The number of fonts can be used for
// indexing with the previous function where the index is between zero and one
// less than the total fonts. If an error occurs, -1 is returned.

std::int_fast32_t stbtt_GetFontOffsetForIndex(const unsigned char* data, std::int_fast32_t index);
// Each .ttf/.ttc file may have more than one font. Each font has a sequential
// index number starting from 0. Call this function to get the font offset for
// a given index; it returns -1 if the index is out of range. A regular .ttf
// file will only define one font and it always be at offset 0, so it will
// return '0' for index 0, and -1 for all other indices.

// The following structure is defined publicly so you can declare one on
// the stack or as a global or etc, but you should treat it as opaque.
struct stbtt_fontinfo
{
	void* userdata{};
	unsigned char* data{};							// pointer to .ttf file
	std::int_fast32_t fontstart{};					// offset of start of font
	std::int_fast32_t numGlyphs{};					// number of glyphs, needed for range checking
	// table locations as offset from start of .ttf
	std::int_fast32_t loca{};
	std::int_fast32_t head{};
	std::int_fast32_t glyf{};
	std::int_fast32_t hhea{};
	std::int_fast32_t hmtx{};
	std::int_fast32_t kern{};
	std::int_fast32_t gpos{};
	std::int_fast32_t index_map{};                  // a cmap mapping for our chosen character encoding
	std::int_fast32_t indexToLocFormat{};           // format needed to map from glyph index to glyph

	stbtt__buf cff{};								// cff font data
	stbtt__buf charstrings{};						// the charstring index
	stbtt__buf gsubrs{};							// global charstring subroutines index
	stbtt__buf subrs{};								// private charstring subroutines index
	stbtt__buf fontdicts{};							// array of font dicts
	stbtt__buf fdselect;							// map from glyph to fontdict
};

std::int_fast32_t stbtt_InitFont(stbtt_fontinfo* info, const unsigned char* data, std::int_fast32_t offset);
// Given an offset into the file that defines a font, this function builds
// the necessary cached info for the rest of the system. You must allocate
// the stbtt_fontinfo yourself, and stbtt_InitFont will fill it out. You don't
// need to do anything special to free it, because the contents are pure
// value data with no additional data structures. Returns 0 on failure.


//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER TO GLYPH-INDEX CONVERSIOn

std::int_fast32_t stbtt_FindGlyphIndex(const stbtt_fontinfo* info, std::int_fast32_t unicode_codepoint);
// If you're going to perform multiple operations on the same character
// and you want a speed-up, call this function with the character you're
// going to process, then use glyph-based functions instead of the
// codepoint-based functions.
// Returns 0 if the character codepoint is not defined in the font.


//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER PROPERTIES
//

float stbtt_ScaleForPixelHeight(const stbtt_fontinfo* info, float height);
// computes a scale factor to produce a font whose "height" is 'pixels' tall.
// Height is measured as the distance from the highest ascender to the lowest
// descender; in other words, it's equivalent to calling stbtt_GetFontVMetrics
// and computing:
//       scale = pixels / (ascent - descent)
// so if you prefer to measure height by the ascent only, use a similar calculation.

float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo* info, float pixels);
// computes a scale factor to produce a font whose EM size is mapped to
// 'pixels' tall. This is probably what traditional APIs compute, but
// I'm not positive.

void stbtt_GetFontVMetrics(const stbtt_fontinfo* info, std::int_fast32_t* ascent, std::int_fast32_t* descent, std::int_fast32_t* lineGap);
// ascent is the coordinate above the baseline the font extends; descent
// is the coordinate below the baseline the font extends (i.e. it is typically negative)
// lineGap is the spacing between one row's descent and the next row's ascent...
// so you should advance the vertical position by "*ascent - *descent + *lineGap"
// these are expressed in unscaled coordinates, so you must multiply by
// the scale factor for a given size

std::int_fast32_t  stbtt_GetFontVMetricsOS2(const stbtt_fontinfo* info, std::int_fast32_t* typoAscent, std::int_fast32_t* typoDescent, std::int_fast32_t* typoLineGap);
// analogous to GetFontVMetrics, but returns the "typographic" values from the OS/2
// table (specific to MS/Windows TTF files).
//
// Returns 1 on success (table present), 0 on failure.

void stbtt_GetFontBoundingBox(const stbtt_fontinfo* info, std::int_fast32_t* x0, std::int_fast32_t* y0, std::int_fast32_t* x1, std::int_fast32_t* y1);
// the bounding box around all possible characters

void stbtt_GetCodepointHMetrics(const stbtt_fontinfo* info, std::int_fast32_t codepoint, std::int_fast32_t* advanceWidth, std::int_fast32_t* leftSideBearing);
// leftSideBearing is the offset from the current horizontal position to the left edge of the character
// advanceWidth is the offset from the current horizontal position to the next horizontal position
// these are expressed in unscaled coordinates

std::int_fast32_t stbtt_GetCodepointKernAdvance(const stbtt_fontinfo* info, std::int_fast32_t ch1, std::int_fast32_t ch2);
// an additional amount to add to the 'advance' value between ch1 and ch2

std::int_fast32_t stbtt_GetCodepointBox(const stbtt_fontinfo* info, std::int_fast32_t codepoint, std::int_fast32_t* x0, std::int_fast32_t* y0, std::int_fast32_t* x1, std::int_fast32_t* y1);
// Gets the bounding box of the visible part of the glyph, in unscaled coordinates

void stbtt_GetGlyphHMetrics(const stbtt_fontinfo* info, std::int_fast32_t glyph_index, std::int_fast32_t* advanceWidth, std::int_fast32_t* leftSideBearing);
std::int_fast32_t stbtt_GetGlyphKernAdvance(const stbtt_fontinfo* info, std::int_fast32_t glyph1, std::int_fast32_t glyph2);
std::int_fast32_t stbtt_GetGlyphBox(const stbtt_fontinfo* info, std::int_fast32_t glyph_index, std::int_fast32_t* x0, std::int_fast32_t* y0, std::int_fast32_t* x1, std::int_fast32_t* y1);
// as above, but takes one or more glyph indices for greater efficiency

//////////////////////////////////////////////////////////////////////////////
//
// GLYPH SHAPES (you probably don't need these, but they have to go before
// the bitmaps for C declaration-order reasons)
//

enum class GlyphShapeType : unsigned char
{
	STBTT_vmove		= 1,
	STBTT_vline		= 2,
	STBTT_vcurve	= 3,
	STBTT_vcubic	= 4
};

using stbtt_vertex = struct
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

std::int_fast32_t stbtt_IsGlyphEmpty(const stbtt_fontinfo* info, std::int_fast32_t glyph_index);
// returns non-zero if nothing is drawn for this glyph

std::int_fast32_t stbtt_GetCodepointShape(const stbtt_fontinfo* info, std::int_fast32_t unicode_codepoint, stbtt_vertex** vertices);
std::int_fast32_t stbtt_GetGlyphShape(const stbtt_fontinfo* info, std::int_fast32_t glyph_index, stbtt_vertex** vertices);
// returns # of vertices and fills *vertices with the pointer to them
// these are expressed in "unscaled" coordinates
//
// The shape is a series of contours. Each one starts with
// a STBTT_moveto, then consists of a series of mixed
// STBTT_lineto and STBTT_curveto segments. A lineto
// draws a line from previous endpoint to its x,y; a curveto
// draws a quadratic bezier from previous endpoint to
// its x,y, using cx,cy as the bezier control point.

void stbtt_FreeShape(const stbtt_fontinfo* info, stbtt_vertex* vertices);
// frees the data allocated above

//////////////////////////////////////////////////////////////////////////////
//
// BITMAP RENDERING
//

unsigned char* stbtt_GetCodepointBitmap(const stbtt_fontinfo* info, float scale_x, float scale_y, std::int_fast32_t codepoint, std::int_fast32_t* width, std::int_fast32_t* height, std::int_fast32_t* xoff, std::int_fast32_t* yoff);
// allocates a large-enough single-channel 8bpp bitmap and renders the
// specified character/glyph at the specified scale into it, with
// antialiasing. 0 is no coverage (transparent), 255 is fully covered (opaque).
// *width & *height are filled out with the width & height of the bitmap,
// which is stored left-to-right, top-to-bottom.
//
// xoff/yoff are the offset it pixel space from the glyph origin to the top-left of the bitmap

unsigned char* stbtt_GetCodepointBitmapSubpixel(const stbtt_fontinfo* info, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t codepoint, std::int_fast32_t* width, std::int_fast32_t* height, std::int_fast32_t* xoff, std::int_fast32_t* yoff);
// the same as stbtt_GetCodepoitnBitmap, but you can specify a subpixel
// shift for the character

void stbtt_MakeCodepointBitmap(const stbtt_fontinfo* info, unsigned char* output, std::int_fast32_t out_w, std::int_fast32_t out_h, std::int_fast32_t out_stride, float scale_x, float scale_y, std::int_fast32_t codepoint);
// the same as stbtt_GetCodepointBitmap, but you pass in storage for the bitmap
// in the form of 'output', with row spacing of 'out_stride' bytes. the bitmap
// is clipped to out_w/out_h bytes. Call stbtt_GetCodepointBitmapBox to get the
// width and height and positioning info for it first.

void stbtt_MakeCodepointBitmapSubpixel(const stbtt_fontinfo* info, unsigned char* output, std::int_fast32_t out_w, std::int_fast32_t out_h, std::int_fast32_t out_stride, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t codepoint);
// same as stbtt_MakeCodepointBitmap, but you can specify a subpixel
// shift for the character

void stbtt_MakeCodepointBitmapSubpixelPrefilter(const stbtt_fontinfo* info, unsigned char* output, std::int_fast32_t out_w, std::int_fast32_t out_h, std::int_fast32_t out_stride, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t prefilter_x, std::int_fast32_t prefilter_y, float *sub_x, float *sub_y, std::int_fast32_t codepoint);
// same as stbtt_MakeCodepointBitmapSubpixel, but prefiltering
// is performed (see stbtt_PackSetOversampling)

void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo* font, std::int_fast32_t codepoint, float scale_x, float scale_y, std::int_fast32_t* ix0, std::int_fast32_t* iy0, std::int_fast32_t* ix1, std::int_fast32_t* iy1);
// get the bbox of the bitmap centered around the glyph origin; so the
// bitmap width is ix1-ix0, height is iy1-iy0, and location to place
// the bitmap top left is (leftSideBearing*scale,iy0).
// (Note that the bitmap uses y-increases-down, but the shape uses
// y-increases-up, so CodepointBitmapBox and CodepointBox are inverted.)

void stbtt_GetCodepointBitmapBoxSubpixel(const stbtt_fontinfo* font, std::int_fast32_t codepoint, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t* ix0, std::int_fast32_t* iy0, std::int_fast32_t* ix1, std::int_fast32_t* iy1);
// same as stbtt_GetCodepointBitmapBox, but you can specify a subpixel
// shift for the character

// the following functions are equivalent to the above functions, but operate
// on glyph indices instead of Unicode codepoints (for efficiency)
unsigned char* stbtt_GetGlyphBitmap(const stbtt_fontinfo* info, float scale_x, float scale_y, std::int_fast32_t glyph, std::int_fast32_t* width, std::int_fast32_t* height, std::int_fast32_t* xoff, std::int_fast32_t* yoff);
unsigned char* stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo* info, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t glyph, std::int_fast32_t* width, std::int_fast32_t* height, std::int_fast32_t* xoff, std::int_fast32_t* yoff);
void stbtt_MakeGlyphBitmap(const stbtt_fontinfo* info, unsigned char* output, std::int_fast32_t out_w, std::int_fast32_t out_h, std::int_fast32_t out_stride, float scale_x, float scale_y, std::int_fast32_t glyph);
void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo* info, unsigned char* output, std::int_fast32_t out_w, std::int_fast32_t out_h, std::int_fast32_t out_stride, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t glyph);
void stbtt_MakeGlyphBitmapSubpixelPrefilter(const stbtt_fontinfo* info, unsigned char* output, std::int_fast32_t out_w, std::int_fast32_t out_h, std::int_fast32_t out_stride, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t prefilter_x, std::int_fast32_t prefilter_y, float* sub_x, float* sub_y, std::int_fast32_t glyph);
void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo* font, std::int_fast32_t glyph, float scale_x, float scale_y, std::int_fast32_t* ix0, std::int_fast32_t* iy0, std::int_fast32_t* ix1, std::int_fast32_t* iy1);
void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo *font, std::int_fast32_t glyph, float scale_x, float scale_y,float shift_x, float shift_y, std::int_fast32_t* ix0, std::int_fast32_t* iy0, std::int_fast32_t* ix1, std::int_fast32_t* iy1);

using stbtt__bitmap = struct
{
	std::int_fast32_t w{};
	std::int_fast32_t h{};
	std::int_fast32_t stride{};
	unsigned char* pixels{};
};

// rasterize a shape with quadratic beziers into a bitmap
void stbtt_Rasterize(stbtt__bitmap* result,												// 1-channel bitmap to draw into
                               float flatness_in_pixels,								// allowable error of curve in pixels
                               stbtt_vertex* vertices,									// array of vertices defining shape
                               std::int_fast32_t num_verts,								// number of vertices in above array
                               float scale_x, float scale_y,							// scale applied to input vertices
                               float shift_x, float shift_y,							// translation applied to input vertices
                               std::int_fast32_t x_off, std::int_fast32_t y_off,		// another translation applied to input
                               std::int_fast32_t invert,								// if non-zero, vertically flip shape
                               void* userdata);											// context for to STBTT_MALLOC

//////////////////////////////////////////////////////////////////////////////
//
// Signed Distance Function (or Field) rendering

unsigned char* stbtt_GetGlyphSDF(const stbtt_fontinfo* info, float scale, std::int_fast32_t glyph, std::int_fast32_t padding, unsigned char onedge_value, float pixel_dist_scale, std::int_fast32_t* width, std::int_fast32_t* height, std::int_fast32_t* xoff, std::int_fast32_t* yoff);
unsigned char* stbtt_GetCodepointSDF(const stbtt_fontinfo* info, float scale, std::int_fast32_t codepoint, std::int_fast32_t padding, unsigned char onedge_value, float pixel_dist_scale, std::int_fast32_t* width, std::int_fast32_t* height, std::int_fast32_t* xoff, std::int_fast32_t* yoff);
// These functions compute a discretized SDF field for a single character, suitable for storing
// in a single-channel texture, sampling with bilinear filtering, and testing against
// larger than some threshold to produce scalable fonts.
//        info              --  the font
//        scale             --  controls the size of the resulting SDF bitmap, same as it would be creating a regular bitmap
//        glyph/codepoint   --  the character to generate the SDF for
//        padding           --  extra "pixels" around the character which are filled with the distance to the character (not 0),
//                                 which allows effects like bit outlines
//        onedge_value      --  value 0-255 to test the SDF against to reconstruct the character (i.e. the isocontour of the character)
//        pixel_dist_scale  --  what value the SDF should increase by when moving one SDF "pixel" away from the edge (on the 0..255 scale)
//                                 if positive, > onedge_value is inside; if negative, < onedge_value is inside
//        width,height      --  output height & width of the SDF bitmap (including padding)
//        xoff,yoff         --  output origin of the character
//        return value      --  a 2D array of bytes 0..255, width*height in size
//
// pixel_dist_scale & onedge_value are a scale & bias that allows you to make
// optimal use of the limited 0..255 for your application, trading off precision
// and special effects. SDF values outside the range 0..255 are clamped to 0..255.
//
// Example:
//      scale = stbtt_ScaleForPixelHeight(22)
//      padding = 5
//      onedge_value = 180
//      pixel_dist_scale = 180/5.0 = 36.0
//
//      This will create an SDF bitmap in which the character is about 22 pixels
//      high but the whole bitmap is about 22+5+5=32 pixels high. To produce a filled
//      shape, sample the SDF at each pixel and fill the pixel if the SDF value
//      is greater than or equal to 180/255. (You'll actually want to antialias,
//      which is beyond the scope of this example.) Additionally, you can compute
//      offset outlines (e.g. to stroke the character border inside & outside,
//      or only outside). For example, to fill outside the character up to 3 SDF
//      pixels, you would compare against (180-36.0*3)/255 = 72/255. The above
//      choice of variables maps a range from 5 pixels outside the shape to
//      2 pixels inside the shape to 0..255; this is intended primarily for apply
//      outside effects only (the interior range is needed to allow proper
//      antialiasing of the font at *smaller* sizes)
//
// The function computes the SDF analytically at each SDF pixel, not by e.g.
// building a higher-res bitmap and approximating it. In theory the quality
// should be as high as possible for an SDF of this size & representation, but
// unclear if this is true in practice (perhaps building a higher-res bitmap
// and computing from that can allow drop-out prevention).
//
// The algorithm has not been optimized at all, so expect it to be slow
// if computing lots of characters or very large sizes.

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
constexpr std::int_fast32_t STBTT_MAX_OVERSAMPLE{ 8 };

using stbtt__test_oversample_pow2 = std::int_fast32_t[(STBTT_MAX_OVERSAMPLE & (STBTT_MAX_OVERSAMPLE - 1)) == 0 ? 1 : -1];

#ifdef _MSC_VER
	#define STBTT__NOTUSED(v)  static_cast<void>(v)
#else
	#define STBTT__NOTUSED(v)  static_cast<void>(sizeof(v))
#endif

//////////////////////////////////////////////////////////////////////////
//
// stbtt__buf helpers to parse data from file
//

inline static unsigned char stbtt__buf_get8(stbtt__buf* b)
{
	if (b->cursor >= b->size)
	{
		return 0;
	}

	return b->data[b->cursor++];
}

inline static unsigned char stbtt__buf_peek8(stbtt__buf* b)
{
	if (b->cursor >= b->size)
	{
		return 0;
	}

	return b->data[b->cursor];
}

inline static void stbtt__buf_seek(stbtt__buf* b, const std::int_fast32_t o)
{
	b->cursor = (o > b->size || o < 0) ? b->size : o;
}

inline static void stbtt__buf_skip(stbtt__buf* b, const std::int_fast32_t o)
{
	stbtt__buf_seek(b, b->cursor + o);
}

inline static std::int_fast32_t stbtt__buf_get(stbtt__buf* b, const std::int_fast32_t n)
{
	std::int_fast32_t v{};

	for (std::int_fast32_t i{}; i < n; i++)
	{
		v = (v << 8) | stbtt__buf_get8(b);
	}

	return v;
}

inline static stbtt__buf stbtt__new_buf(void* p, const size_t size)
{
	stbtt__buf r{};
	r.data = static_cast<unsigned char*>(p);
	r.size = static_cast<std::int_fast32_t>(size);
	r.cursor = 0;
	return r;
}

inline static stbtt__buf stbtt__buf_range(const stbtt__buf* b, const std::int_fast32_t o, const std::int_fast32_t s)
{
	stbtt__buf r{ stbtt__new_buf(nullptr, 0) };

	if (o < 0 || s < 0 || o > b->size || s > b->size - o)
	{
		return r;
	}

	r.data = b->data + o;
	r.size = s;

	return r;
}

inline static stbtt__buf stbtt__cff_get_index(stbtt__buf* b)
{
	const std::int_fast32_t start{ b->cursor };
	const std::int_fast32_t count{ stbtt__buf_get((b), 2) };

	if (count != 0)
	{
		const std::int_fast32_t offsize{ stbtt__buf_get8(b) };
		stbtt__buf_skip(b, offsize * count);
		stbtt__buf_skip(b, stbtt__buf_get(b, offsize) - 1);
	}

	return stbtt__buf_range(b, start, b->cursor - start);
}

inline static std::int_fast32_t stbtt__cff_int(stbtt__buf* b)
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

inline static void stbtt__cff_skip_operand(stbtt__buf* b)
{
	const std::int_fast32_t b0{ stbtt__buf_peek8(b) };

	if (b0 == 30)
	{
		stbtt__buf_skip(b, 1);

		while (b->cursor < b->size)
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

inline static stbtt__buf stbtt__dict_get(stbtt__buf* b, const std::int_fast32_t key)
{
	stbtt__buf_seek(b, 0);

	while (b->cursor < b->size)
	{
		const std::int_fast32_t start{ b->cursor };

		while (stbtt__buf_peek8(b) >= 28)
		{
			stbtt__cff_skip_operand(b);
		}

		const std::int_fast32_t end{ b->cursor };
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

inline static void stbtt__dict_get_ints(stbtt__buf* b, const std::int_fast32_t key, const std::int_fast32_t outcount, std::int_fast32_t* out)
{
	stbtt__buf operands{ stbtt__dict_get(b, key) };

	for (std::int_fast32_t i{}; i < outcount && operands.cursor < operands.size; ++i)
	{
		out[i] = stbtt__cff_int(&operands);
	}
}

inline static std::int_fast32_t stbtt__cff_index_count(stbtt__buf* b)
{
	stbtt__buf_seek(b, 0);

	return stbtt__buf_get((b), 2);
}

inline static stbtt__buf stbtt__cff_index_get(stbtt__buf b, const std::int_fast32_t i)
{
	stbtt__buf_seek(&b, 0);

	const std::int_fast32_t count{ stbtt__buf_get((&b), 2) };
	const std::int_fast32_t offsize{ stbtt__buf_get8(&b) };

	stbtt__buf_skip(&b, i * offsize);

	const std::int_fast32_t start{ stbtt__buf_get(&b, offsize) };

	return stbtt__buf_range(&b, 2 + (count + 1) * offsize + start, stbtt__buf_get(&b, offsize) - start);
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

inline static std::int_fast32_t ttLONG(const unsigned char* p)
{
	return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}

#define stbtt_tag4(p, c0, c1, c2, c3)		((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define stbtt_tag(p, str)					stbtt_tag4(p, str[0], str[1], str[2], str[3])

inline static std::int_fast32_t stbtt__isfont(const unsigned char* font)
{
	// check the version number
	if (stbtt_tag4(font, '1', 0, 0, 0))
	{
		return 1; // TrueType 1
	}

	if (stbtt_tag(font, "typ1"))
	{
		return 1; // TrueType with type 1 font -- we don't support this!
	}

	if (stbtt_tag(font, "OTTO"))
	{
		return 1; // OpenType with CFF
	}

	if (stbtt_tag4(font, 0, 1, 0, 0))
	{
		return 1; // OpenType 1.0
	}

	if (stbtt_tag(font, "true"))
	{
		return 1; // Apple specification for TrueType fonts
	}

	return 0;
}

// @OPTIMIZE: binary search
inline static std::int_fast32_t stbtt__find_table(const unsigned char* data, const std::int_fast32_t fontstart, const char* tag)
{
	const std::int_fast32_t num_tables{ ttUSHORT(data + fontstart + 4) };
	const std::int_fast32_t tabledir{ fontstart + 12 };

	for (std::int_fast32_t i{}; i < num_tables; ++i)
	{
		const std::int_fast32_t loc{ tabledir + 16 * i };

		if (stbtt_tag(data + loc + 0, tag) != 0)
		{
			return ttULONG(data + loc + 8);
		}
	}

	return 0;
}

inline static std::int_fast32_t stbtt_GetFontOffsetForIndex_internal(const unsigned char* font_collection, const std::int_fast32_t index)
{
	// if it's just a font, there's only one valid index
	if (stbtt__isfont(font_collection) != 0)
	{
		return index == 0 ? 0 : -1;
	}

	// check if it's a TTC
	if (stbtt_tag(font_collection, "ttcf") != 0)
	{
		// version 1?
		if (ttULONG(font_collection + 4) == 0x00010000 || ttULONG(font_collection + 4) == 0x00020000)
		{
			if (index >= ttLONG(font_collection + 8))
			{
				return -1;
			}

			return ttULONG(font_collection + 12 + index * 4);
		}
	}

	return -1;
}

inline static std::int_fast32_t stbtt_GetNumberOfFonts_internal(const unsigned char* font_collection)
{
	// if it's just a font, there's only one valid font
	if (stbtt__isfont(font_collection) != 0)
	{
		return 1;
	}

	// check if it's a TTC
	if (stbtt_tag(font_collection, "ttcf"))
	{
		// version 1?
		if (ttULONG(font_collection + 4) == 0x00010000 || ttULONG(font_collection + 4) == 0x00020000)
		{
			return ttLONG(font_collection+8);
		}
	}

	return 0;
}

inline static stbtt__buf stbtt__get_subrs(stbtt__buf cff, stbtt__buf fontdict)
{
	std::vector<std::int_fast32_t> private_loc(2);

	stbtt__dict_get_ints(&fontdict, 18, 2, private_loc.data());

	if (private_loc[1] == 0 || private_loc[0] == 0)
	{
		return stbtt__new_buf(nullptr, 0);
	}

	stbtt__buf pdict{ stbtt__buf_range(&cff, private_loc[1], private_loc[0]) };

	std::int_fast32_t subrsoff{};

	stbtt__dict_get_ints(&pdict, 19, 1, &subrsoff);

	if (subrsoff == 0)
	{
		return stbtt__new_buf(nullptr, 0);
	}

	stbtt__buf_seek(&cff, private_loc[1] + subrsoff);

	return stbtt__cff_get_index(&cff);
}

inline static std::int_fast32_t stbtt_InitFont_internal(stbtt_fontinfo* info, unsigned char* data, const std::int_fast32_t fontstart)
{
	info->data = data;
	info->fontstart = fontstart;
	info->cff = stbtt__new_buf(nullptr, 0);

	info->loca = stbtt__find_table(data, fontstart, "loca"); // required
	info->head = stbtt__find_table(data, fontstart, "head"); // required
	info->glyf = stbtt__find_table(data, fontstart, "glyf"); // required
	info->hhea = stbtt__find_table(data, fontstart, "hhea"); // required
	info->hmtx = stbtt__find_table(data, fontstart, "hmtx"); // required
	info->kern = stbtt__find_table(data, fontstart, "kern"); // not required
	info->gpos = stbtt__find_table(data, fontstart, "GPOS"); // not required

	const std::int_fast32_t cmap{ stbtt__find_table(data, fontstart, "cmap") };       // required

	if (cmap == 0 || info->head == 0 || info->hhea == 0 || info->hmtx == 0)
	{
		return 0;
	}

	if (info->glyf != 0)
	{
		// required for truetype
		if (info->loca == 0)
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

		info->fontdicts = stbtt__new_buf(nullptr, 0);
		info->fdselect = stbtt__new_buf(nullptr, 0);

		// @TODO this should use size from table (not 512MB) - 512 x 1024 x 1024
		info->cff = stbtt__new_buf(data + cff, 536870912);
		stbtt__buf b{ info->cff };

		// read the header
		stbtt__buf_skip(&b, 2);
		stbtt__buf_seek(&b, stbtt__buf_get8(&b)); // hdrsize

		// @TODO the name INDEX could list multiple fonts,
		// but we just use the first one.
		stbtt__cff_get_index(&b);  // name INDEX
		const stbtt__buf topdictidx{ stbtt__cff_get_index(&b) };
		stbtt__buf topdict{ stbtt__cff_index_get(topdictidx, 0) };
		stbtt__cff_get_index(&b);  // string INDEX
		info->gsubrs = stbtt__cff_get_index(&b);

		std::int_fast32_t cstype{ 2 };
		std::int_fast32_t charstrings{};
		std::int_fast32_t fdarrayoff{};
		std::int_fast32_t fdselectoff{};

		stbtt__dict_get_ints(&topdict, 17, 1, &charstrings);
		stbtt__dict_get_ints(&topdict, 0x100 | 6, 1, &cstype);
		stbtt__dict_get_ints(&topdict, 0x100 | 36, 1, &fdarrayoff);
		stbtt__dict_get_ints(&topdict, 0x100 | 37, 1, &fdselectoff);
		info->subrs = stbtt__get_subrs(b, topdict);

		// we only support Type 2 charstrings
		if (cstype != 2)
		{
			return 0;
		}

		if (charstrings == 0)
		{
			return 0;
		}

		if (fdarrayoff != 0)
		{
			// looks like a CID font
			if (fdselectoff == 0)
			{
				return 0;
			}

			stbtt__buf_seek(&b, fdarrayoff);
			info->fontdicts = stbtt__cff_get_index(&b);
			info->fdselect = stbtt__buf_range(&b, fdselectoff, b.size-fdselectoff);
		}

		stbtt__buf_seek(&b, charstrings);
		info->charstrings = stbtt__cff_get_index(&b);
	}

	const std::int_fast32_t t{ stbtt__find_table(data, fontstart, "maxp") };
	info->numGlyphs = (t != 0) ? ttUSHORT(data + t + 4) : 0xFFFF;

	// find a cmap encoding table we understand *now* to avoid searching
	// later. (todo: could make this installable)
	// the same regardless of glyph.
	const std::int_fast32_t numTables{ ttUSHORT(data + cmap + 2) };
	info->index_map = 0;

	for (std::int_fast32_t i{}; i < numTables; ++i)
	{
		const std::int_fast32_t encoding_record{ cmap + 4 + 8 * i };

		// find an encoding we understand:
		switch(ttUSHORT(data + encoding_record))
		{
			case static_cast<std::int_fast32_t>(platformID::STBTT_PLATFORM_ID_MICROSOFT):
			{
				switch (ttUSHORT(data + encoding_record + 2))
				{
					case static_cast<std::int_fast32_t>(encodingIDMicrosoft::STBTT_MS_EID_UNICODE_BMP):
					{
						break;
					}
					case static_cast<std::int_fast32_t>(encodingIDMicrosoft::STBTT_MS_EID_UNICODE_FULL):
					{
						// MS/Unicode
						info->index_map = cmap + ttULONG(data + encoding_record + 4);
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
				info->index_map = cmap + ttULONG(data + encoding_record + 4);
				break;
			}
			default: {}
		}
	}

	if (info->index_map == 0)
	{
		return 0;
	}

	info->indexToLocFormat = ttUSHORT(data + info->head + 50);

	return 1;
}

inline std::int_fast32_t stbtt_FindGlyphIndex(const stbtt_fontinfo* info, const std::int_fast32_t unicode_codepoint)
{
	unsigned char* data{ info->data };
	const std::int_fast32_t index_map{ info->index_map };
	const std::int_fast32_t format{ ttUSHORT(data + index_map + 0) };

	if (format == 0)
	{ // apple byte encoding
		const std::int_fast32_t bytes{ ttUSHORT(data + index_map + 2) };

		if (unicode_codepoint < bytes - 6)
		{
			return *static_cast<unsigned char*>(data + index_map + 6 + unicode_codepoint);
		}

		return 0;
	}

	if (format == 6)
	{
		const std::int_fast32_t first{ ttUSHORT(data + index_map + 6) };
		const std::int_fast32_t count{ ttUSHORT(data + index_map + 8) };

		if (unicode_codepoint >= first && unicode_codepoint < first + count)
		{
			return ttUSHORT(data + index_map + 10 + (unicode_codepoint - first) * 2);
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
		const std::int_fast32_t segcount{ ttUSHORT(data + index_map + 6) >> 1 };
		std::int_fast32_t searchRange{ttUSHORT(data + index_map + 8) >> 1 };
		std::int_fast32_t entrySelector{ ttUSHORT(data + index_map + 10) };
		const std::int_fast32_t rangeShift{ ttUSHORT(data + index_map + 12) >> 1 };

		// do a binary search of the segments
		const std::int_fast32_t endCount{ index_map + 14 };
		std::int_fast32_t search{ endCount };

		// they lie from endCount .. endCount + segCount
		// but searchRange is the nearest power of two, so...
		if (unicode_codepoint >= ttUSHORT(data + search + (rangeShift << 1)))
		{
			search += rangeShift << 1;
		}

		// now decrement to bias correctly to find smallest
		search -= 2;

		while (entrySelector != 0)
		{
			searchRange >>= 1;
			const std::int_fast32_t end{ ttUSHORT(data + search + (searchRange << 1)) };

			if (unicode_codepoint > end)
			{
				search += searchRange << 1;
			}

			--entrySelector;
		}

		search += 2;

		{
			const std::int_fast32_t item{ (search - endCount) >> 1 };
			const std::int_fast32_t start{ ttUSHORT(data + index_map + 14 + segcount * 2 + 2 + 2 * item) };

			if (unicode_codepoint < start)
			{
				return 0;
			}

			std::int_fast32_t offset{ ttUSHORT(data + index_map + 14 + segcount * 6 + 2 + 2 * item) };

			if (offset == 0)
			{
				return unicode_codepoint + ttSHORT(data + index_map + 14 + segcount * 4 + 2 + 2 * item);
			}

			return ttUSHORT(data + offset + (unicode_codepoint - start) * 2 + index_map + 14 + segcount * 6 + 2 + 2 * item);
		}
	}

	if (format == 12 || format == 13)
	{
		const std::int_fast32_t ngroups{ ttULONG(data + index_map + 12) };
		std::int_fast32_t low{};
		std::int_fast32_t high{ ngroups };
		// Binary search the right group.

		while (low < high)
		{
			const std::int_fast32_t mid{ low + ((high - low) >> 1) }; // rounds down, so low <= mid < high
			std::int_fast32_t start_char{ ttULONG(data + index_map + 16 + mid * 12) };
			std::int_fast32_t end_char{ ttULONG(data + index_map + 16 + mid * 12 + 4) };

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
				const std::int_fast32_t start_glyph{ ttULONG(data + index_map + 16 + mid * 12 + 8) };

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

inline std::int_fast32_t stbtt_GetCodepointShape(const stbtt_fontinfo* info, const std::int_fast32_t unicode_codepoint, stbtt_vertex** vertices)
{
	return stbtt_GetGlyphShape(info, stbtt_FindGlyphIndex(info, unicode_codepoint), vertices);
}

inline static void stbtt_setvertex(stbtt_vertex* v, const unsigned char type, const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t cx, const std::int_fast32_t cy)
{
	v->type = type;
	v->x = static_cast<short>(x);
	v->y = static_cast<short>(y);
	v->cx = static_cast<short>(cx);
	v->cy = static_cast<short>(cy);
}

inline static std::int_fast32_t stbtt__GetGlyfOffset(const stbtt_fontinfo* info, const std::int_fast32_t glyph_index)
{
	std::int_fast32_t g1{};
	std::int_fast32_t g2{};

	if (glyph_index >= info->numGlyphs)
	{
		return -1; // glyph index out of range
	}

	if (info->indexToLocFormat >= 2)
	{
		return -1; // unknown index->glyph map format
	}

	if (info->indexToLocFormat == 0)
	{
		g1 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2) * 2;
		g2 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2 + 2) * 2;
	}
	else
	{
		g1 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4);
		g2 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4 + 4);
	}

	return g1 == g2 ? -1 : g1; // if length is 0, return -1
}

static std::int_fast32_t stbtt__GetGlyphInfoT2(const stbtt_fontinfo* info, std::int_fast32_t glyph_index, std::int_fast32_t* x0, std::int_fast32_t* y0, std::int_fast32_t* x1, std::int_fast32_t* y1);

inline std::int_fast32_t stbtt_GetGlyphBox(const stbtt_fontinfo* info, const std::int_fast32_t glyph_index, std::int_fast32_t* x0, std::int_fast32_t* y0, std::int_fast32_t* x1, std::int_fast32_t* y1)
{
	if (info->cff.size != 0)
	{
		stbtt__GetGlyphInfoT2(info, glyph_index, x0, y0, x1, y1);
	}
	else
	{
		std::int_fast32_t g{ stbtt__GetGlyfOffset(info, glyph_index) };

		if (g < 0)
		{
			return 0;
		}

		if (x0 != nullptr)
		{
			*x0 = ttSHORT(info->data + g + 2);
		}

		if (y0 != nullptr)
		{
			*y0 = ttSHORT(info->data + g + 4);
		}

		if (x1 != nullptr)
		{
			*x1 = ttSHORT(info->data + g + 6);
		}

		if (y1 != nullptr)
		{
			*y1 = ttSHORT(info->data + g + 8);
		}
	}

	return 1;
}

inline std::int_fast32_t stbtt_GetCodepointBox(const stbtt_fontinfo* info, const std::int_fast32_t codepoint, std::int_fast32_t* x0, std::int_fast32_t* y0, std::int_fast32_t* x1, std::int_fast32_t* y1)
{
	return stbtt_GetGlyphBox(info, stbtt_FindGlyphIndex(info,codepoint), x0, y0, x1, y1);
}

inline std::int_fast32_t stbtt_IsGlyphEmpty(const stbtt_fontinfo* info, const std::int_fast32_t glyph_index)
{
	if (info->cff.size != 0)
	{
		return static_cast<std::int_fast32_t>(stbtt__GetGlyphInfoT2(info, glyph_index, nullptr, nullptr, nullptr, nullptr) == 0);
	}

	const std::int_fast32_t g{ stbtt__GetGlyfOffset(info, glyph_index) };

	if (g < 0)
	{
		return 1;
	}

	const short numberOfContours{ ttSHORT(info->data + g) };
	return static_cast<std::int_fast32_t>(numberOfContours == 0);
}

inline static std::int_fast32_t stbtt__close_shape(stbtt_vertex* vertices, std::int_fast32_t num_vertices, const std::int_fast32_t was_off, const std::int_fast32_t start_off, const std::int_fast32_t sx, const std::int_fast32_t sy, const std::int_fast32_t scx, const std::int_fast32_t scy, const std::int_fast32_t cx, const std::int_fast32_t cy)
{
	if (start_off != 0)
	{
		if (was_off != 0)
		{
			stbtt_setvertex(&vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve), (cx + scx) >> 1, (cy + scy) >> 1, cx, cy);
		}

		stbtt_setvertex(&vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve), sx, sy, scx, scy);
	}
	else
	{
		was_off != 0 ? stbtt_setvertex(&vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve), sx, sy, cx, cy) : stbtt_setvertex(&vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vline), sx, sy, 0, 0);
	}

	return num_vertices;
}

inline static std::int_fast32_t stbtt__GetGlyphShapeTT(const stbtt_fontinfo* info, const std::int_fast32_t glyph_index, stbtt_vertex** pvertices)
{
	std::int_fast32_t g{ stbtt__GetGlyfOffset(info, glyph_index) };

	if (g < 0)
	{
		return 0;
	}

	unsigned char* data{ info->data };
	std::int_fast32_t num_vertices{};
	short numberOfContours{ ttSHORT(data + g) };
	stbtt_vertex* vertices{};

	if (numberOfContours > 0)
	{
		unsigned char flags{};
		std::int_fast32_t j{};
		std::int_fast32_t was_off{};
		std::int_fast32_t start_off{};
		std::int_fast32_t x{};
		std::int_fast32_t y{};
		std::int_fast32_t cx{};
		std::int_fast32_t cy{};
		std::int_fast32_t sx{};
		std::int_fast32_t sy{};
		std::int_fast32_t scx{};
		std::int_fast32_t scy{};
		unsigned char* endPtsOfContours{ (data + g + 10) };
		const std::int_fast32_t ins{ ttUSHORT(data + g + 10 + (numberOfContours << 1)) };
		unsigned char* points{ data + g + 10 + (numberOfContours << 1) + 2 + ins };
		const std::int_fast32_t n{ 1 + ttUSHORT(endPtsOfContours + (numberOfContours << 1) - 2) };
		const std::int_fast32_t m{ n + (numberOfContours << 1) };  // a loose bound on how many vertices we might need
		vertices = new stbtt_vertex[m * sizeof(vertices[0])];

		if (vertices == nullptr)
		{
			return 0;
		}

		std::int_fast32_t next_move{};
		unsigned char flagcount{};

		// in first pass, we load uninterpreted data into the allocated array
		// above, shifted to the end of the array so we won't overwrite it when
		// we create our final data starting from the front

		const std::int_fast32_t off{ m - n }; // starting offset for uninterpreted data, regardless of how m ends up being calculated

		// first load flags

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
					x = x + static_cast<std::int_fast32_t>((points[0] << 8) + points[1]);
					points += 2;
				}
			}

			vertices[off+i].x = static_cast<short>(x);
		}

		// now load y coordinates
		for (std::int_fast32_t i{}; i < n; ++i)
		{
			flags = vertices[off + i].type;

			if ((flags & 4) != 0)
			{
				short dy{ *points++ };
				y += (flags & 32) != 0 ? dy : -dy; // ???
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

				stbtt_setvertex(&vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vmove), sx, sy, 0, 0);
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
						stbtt_setvertex(&vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve), (cx + x) >> 1, (cy + y) >> 1, cx, cy);
					}

					cx = x;
					cy = y;
					was_off = 1;
				}
				else
				{
					was_off != 0 ? stbtt_setvertex(&vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve), x, y, cx, cy) : stbtt_setvertex(&vertices[num_vertices++], static_cast<unsigned char>(GlyphShapeType::STBTT_vline), x, y, 0, 0);
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
		unsigned char* comp{ data + g + 10 };

		while (more != 0)
		{
			float mtx[6]{ 1, 0, 0, 1, 0, 0 };

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
				mtx[0] = ttSHORT(comp) / 16384.0F;
				mtx[3] = mtx[0];
				comp += 2;
				mtx[1] = 0;
				mtx[2] = 0;
			}
			else if ((flags & (1 <<6 )) != 0)
			{ // WE_HAVE_AN_X_AND_YSCALE
				mtx[0] = ttSHORT(comp) / 16384.0F;
				comp += 2;
				mtx[1] = 0;
				mtx[2] = 0;
				mtx[3] = ttSHORT(comp) / 16384.0F;
				comp+=2;
			}
			else if ((flags & (1 << 7)) != 0)
			{ // WE_HAVE_A_TWO_BY_TWO
				mtx[0] = ttSHORT(comp) / 16384.0F;
				comp += 2;
				mtx[1] = ttSHORT(comp) / 16384.0F;
				comp += 2;
				mtx[2] = ttSHORT(comp) / 16384.0F;
				comp += 2;
				mtx[3] = ttSHORT(comp) / 16384.0F;
				comp += 2;
			}

			// Find transformation scales.
			const float m{ std::sqrtf(mtx[0] * mtx[0] + mtx[1] * mtx[1]) };
			const float n{ std::sqrtf(mtx[2] * mtx[2] + mtx[3] * mtx[3]) };

			// Get indexed glyph.
			std::unique_ptr<stbtt_vertex> comp_verts;
			const std::int_fast32_t comp_num_verts{ stbtt_GetGlyphShape(info, gidx, reinterpret_cast<stbtt_vertex**>(&comp_verts)) };

			if (comp_num_verts > 0)
			{
				// Transform vertices.
				for (std::int_fast32_t i{}; i < comp_num_verts; ++i)
				{
					stbtt_vertex* v{ &comp_verts.get()[i] };
					short x{ v->x };
					short y{ v->y };

					v->x = static_cast<short>(m * (mtx[0] * x + mtx[2] * y + mtx[4]));
					v->y = static_cast<short>(n * (mtx[1] * x + mtx[3] * y + mtx[5]));

					x = v->cx;
					y = v->cy;

					v->cx = static_cast<short>(m * (mtx[0] * x + mtx[2] * y + mtx[4]));
					v->cy = static_cast<short>(n * (mtx[1] * x + mtx[3] * y + mtx[5]));
				}

				// Append vertices.
				stbtt_vertex* tmp { new stbtt_vertex[(num_vertices + comp_num_verts) * sizeof(stbtt_vertex)] };

				if (tmp == nullptr)
				{
					if (vertices != nullptr)
					{
						delete vertices;
					}

					return 0;
				}

				if (num_vertices > 0)
				{
					std::memcpy(tmp, vertices, num_vertices * sizeof(stbtt_vertex));
				}

				std::memcpy(tmp + num_vertices, comp_verts.get(), comp_num_verts * sizeof(stbtt_vertex));

				if (vertices != nullptr)
				{
					delete vertices;
				}

				vertices = tmp;
				num_vertices += comp_num_verts;
			}

			// More components ?
			more = flags & (1 << 5);
		}
	}
	else if (numberOfContours < 0)
	{
		// @TODO other compound variations?
	}
	else
	{
		// numberOfCounters == 0, do nothing
	}

	*pvertices = vertices;

	return num_vertices;
}

using stbtt__csctx = struct
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
	stbtt_vertex* pvertices{};
	std::int_fast32_t num_vertices{};
};

inline static void stbtt__track_vertex(stbtt__csctx* c, const std::int_fast32_t x, const std::int_fast32_t y)
{
	if (x > c->max_x || c->started == 0)
	{
		c->max_x = x;
	}

	if (y > c->max_y || c->started == 0)
	{
		c->max_y = y;
	}

	if (x < c->min_x || c->started == 0)
	{
		c->min_x = x;
	}

	if (y < c->min_y || c->started == 0)
	{
		c->min_y = y;
	}

	c->started = 1;
}

inline static void stbtt__csctx_v(stbtt__csctx* c, const unsigned char type, const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t cx, const std::int_fast32_t cy, const std::int_fast32_t cx1, const std::int_fast32_t cy1)
{
	if (c->bounds != 0)
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
		stbtt_setvertex(&c->pvertices[c->num_vertices], type, x, y, cx, cy);
		c->pvertices[c->num_vertices].cx1 = static_cast<short>(cx1);
		c->pvertices[c->num_vertices].cy1 = static_cast<short>(cy1);
	}

	c->num_vertices++;
}

inline static void stbtt__csctx_close_shape(stbtt__csctx* ctx)
{
	if (ctx->first_x != ctx->x || ctx->first_y != ctx->y)
	{
		stbtt__csctx_v(ctx, static_cast<unsigned char>(GlyphShapeType::STBTT_vline), static_cast<std::int_fast32_t>(ctx->first_x), static_cast<std::int_fast32_t>(ctx->first_y), 0, 0, 0, 0);
	}
}

inline static void stbtt__csctx_rmove_to(stbtt__csctx* ctx, const float dx, const float dy)
{
	stbtt__csctx_close_shape(ctx);
	ctx->first_x = ctx->x = ctx->x + dx;
	ctx->first_y = ctx->y = ctx->y + dy;
	stbtt__csctx_v(ctx, static_cast<unsigned char>(GlyphShapeType::STBTT_vmove), static_cast<std::int_fast32_t>(ctx->x), static_cast<std::int_fast32_t>(ctx->y), 0, 0, 0, 0);
}

inline static void stbtt__csctx_rline_to(stbtt__csctx* ctx, const float dx, const float dy)
{
	ctx->x += dx;
	ctx->y += dy;
	stbtt__csctx_v(ctx, static_cast<unsigned char>(GlyphShapeType::STBTT_vline), static_cast<std::int_fast32_t>(ctx->x), static_cast<std::int_fast32_t>(ctx->y), 0, 0, 0, 0);
}

inline static void stbtt__csctx_rccurve_to(stbtt__csctx* ctx, const float dx1, const float dy1, const float dx2, const float dy2, const float dx3, const float dy3)
{
	const float cx1{ ctx->x + dx1 };
	const float cy1{ ctx->y + dy1 };
	const float cx2{ cx1 + dx2 };
	const float cy2{ cy1 + dy2 };

	ctx->x = cx2 + dx3;
	ctx->y = cy2 + dy3;

	stbtt__csctx_v(ctx, static_cast<unsigned char>(GlyphShapeType::STBTT_vcubic), static_cast<std::int_fast32_t>(ctx->x), static_cast<std::int_fast32_t>(ctx->y), static_cast<std::int_fast32_t>(cx1), static_cast<std::int_fast32_t>(cy1), static_cast<std::int_fast32_t>(cx2), static_cast<std::int_fast32_t>(cy2));
}

inline static stbtt__buf stbtt__get_subr(stbtt__buf idx, std::int_fast32_t n)
{
	const std::int_fast32_t count{ stbtt__cff_index_count(&idx) };
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

inline static stbtt__buf stbtt__cid_get_glyph_subrs(const stbtt_fontinfo* info, const std::int_fast32_t glyph_index)
{
	stbtt__buf fdselect{ info->fdselect };
	std::int_fast32_t fdselector{ -1 };

	stbtt__buf_seek(&fdselect, 0);
	const std::int_fast32_t fmt{ stbtt__buf_get8(&fdselect) };

	if (fmt == 0)
	{
		// untested
		stbtt__buf_skip(&fdselect, glyph_index);
		fdselector = stbtt__buf_get8(&fdselect);
	}
	else if (fmt == 3)
	{
		const std::int_fast32_t nranges{ stbtt__buf_get((&fdselect), 2) };
		std::int_fast32_t start{ stbtt__buf_get((&fdselect), 2) };

		for (std::int_fast32_t i{}; i < nranges; ++i)
		{
			const std::int_fast32_t v{ stbtt__buf_get8(&fdselect) };
			const std::int_fast32_t end{ stbtt__buf_get((&fdselect), 2) };

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

	return stbtt__get_subrs(info->cff, stbtt__cff_index_get(info->fontdicts, fdselector));
}

inline static std::int_fast32_t stbtt__run_charstring(const stbtt_fontinfo* info, const std::int_fast32_t glyph_index, stbtt__csctx* c)
{
	std::int_fast32_t in_header{ 1 };
	std::int_fast32_t maskbits{};
	std::int_fast32_t subr_stack_height{};
	std::int_fast32_t sp{};
	std::int_fast32_t has_subrs{};
	std::vector<float> s(48);
	std::vector<stbtt__buf> subr_stack(10);
	stbtt__buf subrs{ info->subrs };

	#define STBTT__CSERR(s) (0)

	// this currently ignores the initial width value, which isn't needed if we have hmtx
	stbtt__buf b{ stbtt__cff_index_get(info->charstrings, glyph_index) };

	while (b.cursor < b.size)
	{
		std::int_fast32_t i{};
		std::int_fast32_t clear_stack{ 1 };
		const std::int_fast32_t b0{ stbtt__buf_get8(&b) };

		switch (b0)
		{
			// @TODO implement hinting
			case 0x13: // hintmask
			{
				break;
			}
			case 0x14: // cntrmask
			{
				if (in_header != 0)
				{
					maskbits += (sp / 2); // implicit "vstem"
				}

				in_header = 0;
				stbtt__buf_skip(&b, (maskbits + 7) >> 3);
				break;
			}
			case 0x01: // hstem
			{
				break;
			}
			case 0x03: // vstem
			{
				break;
			}
			case 0x12: // hstemhm
			{
				break;
			}
			case 0x17: // vstemhm
			{
				maskbits += (sp >> 1);
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
					if (i >= sp)
					{
						break;
					}
				}

				stbtt__csctx_rline_to(c, s[i], 0);
				++i;

				vlineto:
				if (i >= sp)
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
					if (info->fdselect.size != 0)
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

				const std::int_fast32_t v{ static_cast<std::int_fast32_t>(s[--sp]) };

				if (subr_stack_height >= 10)
				{
					return STBTT__CSERR("recursion limit");
				}

				subr_stack[subr_stack_height++] = b;
				b = stbtt__get_subr(b0 == 0x0A ? subrs : info->gsubrs, v);

				if (b.size == 0)
				{
					return STBTT__CSERR("subr not found");
				}

				b.cursor = 0;
				clear_stack = 0;
				break;
			}
			case 0x0B: // return
			{
				if (subr_stack_height <= 0)
				{
					return STBTT__CSERR("return outside subr");
				}

				b = subr_stack[--subr_stack_height];
				clear_stack = 0;
				break;
			}
			case 0x0E: // endchar
			{
				stbtt__csctx_close_shape(c);
				return 1;
			}
			case 0x0C: // two-byte escape
			{
				float dx1{};
				float dx2{};
				float dx3{};
				float dx4{};
				float dx5{};
				float dx6{};
				float dy1{};
				float dy2{};
				float dy3{};
				float dy4{};
				float dy5{};
				float dy6{};
				float dx{};
				float dy{};

				switch (stbtt__buf_get8(&b))
				{
					// @TODO These "flex" implementations ignore the flex-depth and resolution,
					// and always draw beziers.
					case 0x22: // hflex
					{
						if (sp < 7)
						{
							return STBTT__CSERR("hflex stack");
						}

						dx1 = s[0];
						dx2 = s[1];
						dy2 = s[2];
						dx3 = s[3];
						dx4 = s[4];
						dx5 = s[5];
						dx6 = s[6];
						stbtt__csctx_rccurve_to(c, dx1, 0.0F, dx2, dy2, dx3, 0.0F);
						stbtt__csctx_rccurve_to(c, dx4, 0.0F, dx5, -dy2, dx6, 0.0F);
						break;
					}
					case 0x23: // flex
					{
						if (sp < 13)
						{
							return STBTT__CSERR("flex stack");
						}

						dx1 = s[0];
						dy1 = s[1];
						dx2 = s[2];
						dy2 = s[3];
						dx3 = s[4];
						dy3 = s[5];
						dx4 = s[6];
						dy4 = s[7];
						dx5 = s[8];
						dy5 = s[9];
						dx6 = s[10];
						dy6 = s[11];
						//fd is s[12]
						stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
						stbtt__csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
						break;
					}
					case 0x24: // hflex1
					{
						if (sp < 9)
						{
							return STBTT__CSERR("hflex1 stack");
						}

						dx1 = s[0];
						dy1 = s[1];
						dx2 = s[2];
						dy2 = s[3];
						dx3 = s[4];
						dx4 = s[5];
						dx5 = s[6];
						dy5 = s[7];
						dx6 = s[8];
						stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, 0.0F);
						stbtt__csctx_rccurve_to(c, dx4, 0.0F, dx5, dy5, dx6, -(dy1 + dy2 + dy5));
						break;
					}
					case 0x25: // flex1
					{
						if (sp < 11)
						{
							return STBTT__CSERR("flex1 stack");
						}

						dx1 = s[0];
						dy1 = s[1];
						dx2 = s[2];
						dy2 = s[3];
						dx3 = s[4];
						dy3 = s[5];
						dx4 = s[6];
						dy4 = s[7];
						dx5 = s[8];
						dy5 = s[9];
						dx6 = dy6 = s[10];
						dx = dx1 + dx2 + dx3 + dx4 + dx5;
						dy = dy1 + dy2 + dy3 + dy4 + dy5;

						std::fabs(dx) > std::fabs(dy) ? dy6 = -dy : dx6 = -dx;

						stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
						stbtt__csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
						break;
					}
					default:
					{
						return STBTT__CSERR("unimplemented");
					}
				}
				break;
			}
			default:
			{
				if (b0 != 255 && b0 != 28 && (b0 < 32 || b0 > 254))
				{
					return STBTT__CSERR("reserved operator");
				}

				float f{};

				// push immediate
				if (b0 == 255)
				{
					f = static_cast<float>(static_cast<std::int_fast32_t>(stbtt__buf_get((&b), 4)) / 0x10000);
				}
				else
				{
					stbtt__buf_skip(&b, -1);
					f = static_cast<float>(stbtt__cff_int(&b));
				}

				if (sp >= 48)
				{
					return STBTT__CSERR("push stack overflow");
				}

				s[sp++] = f;
				clear_stack = 0;
				break;
			}

			if (clear_stack != 0)
			{
				sp = 0;
			}
		}
	}

	return STBTT__CSERR("no endchar");

	#undef STBTT__CSERR
}

inline static std::int_fast32_t stbtt__GetGlyphShapeT2(const stbtt_fontinfo* info, const std::int_fast32_t glyph_index, stbtt_vertex** pvertices)
{
	// runs the charstring twice, once to count and once to output (to avoid realloc)
	stbtt__csctx count_ctx{ 1, 0, 0.0F, 0.0F, 0.0F, 0.0F, 0, 0, 0, 0, nullptr, 0 };
	stbtt__csctx output_ctx{ 0, 0, 0.0F, 0.0F, 0.0F, 0.0F, 0, 0, 0, 0, nullptr, 0 };

	if (stbtt__run_charstring(info, glyph_index, &count_ctx) != 0)
	{
		*pvertices = new stbtt_vertex[count_ctx.num_vertices * sizeof(stbtt_vertex)];

		output_ctx.pvertices = *pvertices;

		if (stbtt__run_charstring(info, glyph_index, &output_ctx) != 0)
		{
			return output_ctx.num_vertices;
		}
	}

	delete *pvertices;
	return 0;
}

inline static std::int_fast32_t stbtt__GetGlyphInfoT2(const stbtt_fontinfo *info, const std::int_fast32_t glyph_index, std::int_fast32_t *x0, std::int_fast32_t *y0, std::int_fast32_t *x1, std::int_fast32_t *y1)
{
	stbtt__csctx c{ 1, 0, 0.0F, 0.0F, 0.0F, 0.0F, 0, 0, 0, 0, nullptr, 0 };
	const std::int_fast32_t r{ stbtt__run_charstring(info, glyph_index, &c) };

	if (x0 != nullptr)
	{
		*x0 = r != 0 ? c.min_x : 0;
	}

	if (y0 != nullptr)
	{
		*y0 = r != 0 ? c.min_y : 0;
	}

	if (x1 != nullptr)
	{
		*x1 = r != 0 ? c.max_x : 0;
	}

	if (y1 != nullptr)
	{
		*y1 = r != 0 ? c.max_y : 0;
	}

	return r != 0 ? c.num_vertices : 0;
}

inline std::int_fast32_t stbtt_GetGlyphShape(const stbtt_fontinfo* info, const std::int_fast32_t glyph_index, stbtt_vertex** pvertices)
{
	if (info->cff.size == 0)
	{
		return stbtt__GetGlyphShapeTT(info, glyph_index, pvertices);
	}

	return stbtt__GetGlyphShapeT2(info, glyph_index, pvertices);
}

inline void stbtt_GetGlyphHMetrics(const stbtt_fontinfo* info, const std::int_fast32_t glyph_index, std::int_fast32_t* advanceWidth, std::int_fast32_t* leftSideBearing)
{
	const std::int_fast32_t numOfLongHorMetrics{ ttUSHORT(info->data + info->hhea + 34) };

	if (glyph_index < numOfLongHorMetrics)
	{
		if (advanceWidth != nullptr)
		{
			*advanceWidth = ttSHORT(info->data + info->hmtx + 4 * glyph_index);
		}

		if (leftSideBearing != nullptr)
		{
			*leftSideBearing = ttSHORT(info->data + info->hmtx + 4 * glyph_index + 2);
		}
	}
	else
	{
		if (advanceWidth != nullptr)
		{
			*advanceWidth = ttSHORT(info->data + info->hmtx + 4 * (numOfLongHorMetrics - 1));
		}

		if (leftSideBearing != nullptr)
		{
			*leftSideBearing = ttSHORT(info->data + info->hmtx + 4 * numOfLongHorMetrics + 2 * (glyph_index - numOfLongHorMetrics));
		}
	}
}

inline static std::int_fast32_t stbtt__GetGlyphKernInfoAdvance(const stbtt_fontinfo* info, const std::int_fast32_t glyph1, const std::int_fast32_t glyph2)
{
	unsigned char* data{ info->data + info->kern };

	// we only look at the first table. it must be 'horizontal' and format 0.
	if (info->kern == 0)
	{
		return 0;
	}

	if (ttUSHORT(data + 2) < 1) // number of tables, need at least 1
	{
		return 0;
	}

	if (ttUSHORT(data + 8) != 1) // horizontal flag must be set in format
	{
		return 0;
	}

	std::int_fast32_t l{};
	std::int_fast32_t r{ ttUSHORT(data + 10) - 1 };
	const std::int_fast32_t needle{ glyph1 << 16 | glyph2 };

	while (l <= r)
	{
		const std::int_fast32_t m{ (l + r) >> 1 };
		const std::int_fast32_t straw{ ttULONG(data + 18 + (m * 6)) }; // note: unaligned read

		if (needle < straw)
		{
			r = m - 1;
		}
		else if (needle > straw)
		{
			l = m + 1;
		}
		else
		{
			return ttSHORT(data + 22 + (m * 6));
		}
	}

	return 0;
}

inline static std::int_fast32_t stbtt__GetCoverageIndex(unsigned char* coverageTable, const std::int_fast32_t glyph)
{
	switch(ttUSHORT(coverageTable))
	{
		case 1:
		{
			// Binary search.
			std::int_fast32_t l{};
			std::int_fast32_t r{ ttUSHORT(coverageTable + 2) - 1 };

			while (l <= r)
			{
				unsigned char* glyphArray{ coverageTable + 4 };
				const std::int_fast32_t m{ (l + r) >> 1 };
				const std::int_fast32_t straw{ ttUSHORT(glyphArray + (m << 1)) };

				if (glyph < straw)
				{
					r = m - 1;
				}
				else if (glyph > straw)
				{
					l = m + 1;
				}
				else
				{
					return m;
				}
			}
			break;
		}

		case 2:
		{
			// Binary search.
			unsigned char* rangeArray{ coverageTable + 4 };
			std::int_fast32_t l{};
			std::int_fast32_t r{ ttUSHORT(coverageTable + 2) - 1 };

			while (l <= r)
			{
				const std::int_fast32_t m{ (l + r) >> 1 };
				unsigned char* rangeRecord{ rangeArray + 6 * m };
				const std::int_fast32_t strawStart{ ttUSHORT(rangeRecord) };
				const std::int_fast32_t strawEnd{ ttUSHORT(rangeRecord + 2) };

				if (glyph < strawStart)
				{
					r = m - 1;
				}
				else if (glyph > strawEnd)
				{
					l = m + 1;
				}
				else
				{
					return ttUSHORT(rangeRecord + 4) + glyph - strawStart;
				}
			}
			break;
		}

		default: {}
	}

	return -1;
}

inline static std::int_fast32_t stbtt__GetGlyphClass(unsigned char* classDefTable, const std::int_fast32_t glyph)
{
	switch(ttUSHORT(classDefTable))
	{
		case 1:
		{
			const std::int_fast32_t startGlyphID{ ttUSHORT(classDefTable + 2) };
			const std::int_fast32_t glyphCount{ ttUSHORT(classDefTable + 4) };
			unsigned char* classDef1ValueArray{ classDefTable + 6 };

			if (glyph >= startGlyphID && glyph < startGlyphID + glyphCount)
			{
				return ttUSHORT(classDef1ValueArray + 2 * (glyph - startGlyphID));
			}

			classDefTable = classDef1ValueArray + 2 * glyphCount;
			break;
		}

		case 2:
		{
			const std::int_fast32_t classRangeCount{ ttUSHORT(classDefTable + 2) };
			unsigned char* classRangeRecords{ classDefTable + 4 };

			// Binary search.
			std::int_fast32_t l{};
			std::int_fast32_t r{ classRangeCount - 1};
			const std::int_fast32_t needle{ glyph };

			while (l <= r)
			{
				const std::int_fast32_t m{ (l + r) >> 1 };
				unsigned char* classRangeRecord{ classRangeRecords + 6 * m };

				if (needle < ttUSHORT(classRangeRecord))
				{
					r = m - 1;
				}
				else if (needle > ttUSHORT(classRangeRecord + 2))
				{
					l = m + 1;
				}
				else
				{
					return ttUSHORT(classRangeRecord + 4);
				}
			}

			classDefTable = classRangeRecords + 6 * classRangeCount;
			break;
		}
		default: {}
	}

	return -1;
}

inline static std::int_fast32_t stbtt__GetGlyphGPOSInfoAdvance(const stbtt_fontinfo* info, const std::int_fast32_t glyph1, const std::int_fast32_t glyph2)
{
	if (info->gpos == 0)
	{
		return 0;
	}

	unsigned char* data{ info->data + info->gpos };

	if (ttUSHORT(data + 0) != 1)
	{
		return 0; // Major version 1
	}

	if (ttUSHORT(data + 2) != 0)
	{
		return 0; // Minor version 0
	}

	unsigned char* lookupList{ data + ttUSHORT(data + 8) };
	const std::int_fast32_t lookupCount{ ttUSHORT(lookupList) };

	for (std::int_fast32_t i{}; i < lookupCount; ++i)
	{
		unsigned char* lookupTable{ lookupList + ttUSHORT(lookupList + 2 + 2 * i) };

        switch(ttUSHORT(lookupTable))
		{
            case 2:
			{ // Pair Adjustment Positioning Subtable
				const std::int_fast32_t subTableCount{ ttUSHORT(lookupTable + 4) };
				unsigned char* subTableOffsets{ lookupTable + 6 };

				for (std::int_fast32_t sti{}; sti < subTableCount; sti++)
				{
					unsigned char* table{ lookupTable + ttUSHORT(subTableOffsets + 2 * sti) };
					const std::int_fast32_t coverageIndex{ stbtt__GetCoverageIndex(table + ttUSHORT(table + 2), glyph1) };

					if (coverageIndex == -1)
					{
						continue;
					}

                    switch (ttUSHORT(table))
					{
                        case 1:
						{
							const std::int_fast32_t valueFormat1{ ttUSHORT(table + 4) };
							const std::int_fast32_t valueFormat2{ ttUSHORT(table + 6) };
							const std::int_fast32_t valueRecordPairSizeInBytes{ 2 };
							const std::int_fast32_t pairSetCount{ ttUSHORT(table + 8) };
							const std::int_fast32_t pairPosOffset{ ttUSHORT(table + 10 + 2 * coverageIndex) };
							unsigned char* pairValueTable{ table + pairPosOffset };
							const std::int_fast32_t pairValueCount{ ttUSHORT(pairValueTable) };
							unsigned char* pairValueArray{ pairValueTable + 2 };

							if ((valueFormat1 != 4) || (valueFormat2 != 0))
							{
								return 0;
							}

                            STBTT__NOTUSED(pairSetCount);

							std::int_fast32_t needle{ glyph2 };
							std::int_fast32_t r{ pairValueCount - 1 };
							std::int_fast32_t l{};

                            // Binary search.
                            while (l <= r)
							{
								std::int_fast32_t m{ (l + r) >> 1 };
								unsigned char* pairValue{ pairValueArray + (2 + valueRecordPairSizeInBytes) * m };
								const std::int_fast32_t secondGlyph{ ttUSHORT(pairValue) };

								if (needle < secondGlyph)
								{
									r = m - 1;
								}
								else if (needle > secondGlyph)
								{
									l = m + 1;
								}
								else
								{
									return ttSHORT(pairValue + 2);
								}
                            }

							break;
                        }
                        case 2:
						{
							const std::int_fast32_t valueFormat1{ ttUSHORT(table + 4) };
							const std::int_fast32_t valueFormat2{ ttUSHORT(table + 6) };
							const std::int_fast32_t glyph1class{ stbtt__GetGlyphClass(table + ttUSHORT(table + 8), glyph1) };
							const std::int_fast32_t glyph2class{ stbtt__GetGlyphClass(table + ttUSHORT(table + 10), glyph2) };
							const std::int_fast32_t class1Count{ ttUSHORT(table + 12) };
							const std::int_fast32_t class2Count{ ttUSHORT(table + 14) };

							if (valueFormat1 != 4)
							{
								return 0;
							}

							if (valueFormat2 != 0)
							{
								return 0;
							}

                            if (glyph1class >= 0 && glyph1class < class1Count && glyph2class >= 0 && glyph2class < class2Count)
							{
                                return ttSHORT(table + 16 + 2 * (glyph1class * class2Count) + 2 * glyph2class);
                            }

							break;
                        }
						default: {}
                    }
                }

                break;
            };

            default:
			{
				break;
			}
        }
    }

    return 0;
}

inline std::int_fast32_t stbtt_GetGlyphKernAdvance(const stbtt_fontinfo* info, const std::int_fast32_t glyph1, const std::int_fast32_t glyph2)
{
	std::int_fast32_t xAdvance{};

	if (info->gpos != 0)
	{
		xAdvance += stbtt__GetGlyphGPOSInfoAdvance(info, glyph1, glyph2);
	}

	if (info->kern != 0)
	{
		xAdvance += stbtt__GetGlyphKernInfoAdvance(info, glyph1, glyph2);
	}

	return xAdvance;
}

inline std::int_fast32_t stbtt_GetCodepointKernAdvance(const stbtt_fontinfo* info, const std::int_fast32_t ch1, const std::int_fast32_t ch2)
{
	if (info->kern == 0 && info->gpos == 0) // if no kerning table, don't waste time looking up both codepoint->glyphs
	{
		return 0;
	}

	return stbtt_GetGlyphKernAdvance(info, stbtt_FindGlyphIndex(info,ch1), stbtt_FindGlyphIndex(info,ch2));
}

inline void stbtt_GetCodepointHMetrics(const stbtt_fontinfo* info, const std::int_fast32_t codepoint, std::int_fast32_t* advanceWidth, std::int_fast32_t* leftSideBearing)
{
	stbtt_GetGlyphHMetrics(info, stbtt_FindGlyphIndex(info,codepoint), advanceWidth, leftSideBearing);
}

inline void stbtt_GetFontVMetrics(const stbtt_fontinfo* info, std::int_fast32_t* ascent, std::int_fast32_t* descent, std::int_fast32_t* lineGap)
{
	if (ascent != nullptr)
	{
		*ascent = ttSHORT(info->data + info->hhea + 4);
	}

	if (descent != nullptr)
	{
		*descent = ttSHORT(info->data + info->hhea + 6);
	}

	if (lineGap != nullptr)
	{
		*lineGap = ttSHORT(info->data + info->hhea + 8);
	}
}

inline std::int_fast32_t stbtt_GetFontVMetricsOS2(const stbtt_fontinfo* info, std::int_fast32_t* typoAscent, std::int_fast32_t* typoDescent, std::int_fast32_t* typoLineGap)
{
	const std::int_fast32_t tab{ stbtt__find_table(info->data, info->fontstart, "OS/2") };

	if (tab == 0)
	{
		return 0;
	}

	if (typoAscent != nullptr)
	{
		*typoAscent = ttSHORT(info->data + tab + 68);
	}

	if (typoDescent != nullptr)
	{
		*typoDescent = ttSHORT(info->data + tab + 70);
	}

	if (typoLineGap != nullptr)
	{
		*typoLineGap = ttSHORT(info->data + tab + 72);
	}

	return 1;
}

inline void stbtt_GetFontBoundingBox(const stbtt_fontinfo* info, std::int_fast32_t* x0, std::int_fast32_t* y0, std::int_fast32_t* x1, std::int_fast32_t* y1)
{
	*x0 = ttSHORT(info->data + info->head + 36);
	*y0 = ttSHORT(info->data + info->head + 38);
	*x1 = ttSHORT(info->data + info->head + 40);
	*y1 = ttSHORT(info->data + info->head + 42);
}

inline float stbtt_ScaleForPixelHeight(const stbtt_fontinfo* info, const float height)
{
	return static_cast<float>(height / (ttSHORT(info->data + info->hhea + 4) - ttSHORT(info->data + info->hhea + 6)));
}

inline float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo* info, const float pixels)
{
	return pixels / ttUSHORT(info->data + info->head + 18);
}

inline void stbtt_FreeShape(const stbtt_fontinfo* info, stbtt_vertex* v)
{
	static_cast<void>(info->userdata), free(v);
}

//////////////////////////////////////////////////////////////////////////////
//
// antialiasing software rasterizer
//

inline void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo* font, const std::int_fast32_t glyph, const float scale_x, const float scale_y, const float shift_x, const float shift_y, std::int_fast32_t* ix0, std::int_fast32_t* iy0, std::int_fast32_t* ix1, std::int_fast32_t* iy1)
{
	std::int_fast32_t x0{};
	std::int_fast32_t y0{};
	std::int_fast32_t x1{};
	std::int_fast32_t y1{};

	if (stbtt_GetGlyphBox(font, glyph, &x0, &y0, &x1, &y1) == 0)
	{
		// e.g. space character
		if (ix0 != nullptr)
		{
			*ix0 = 0;
		}

		if (iy0 != nullptr)
		{
			*iy0 = 0;
		}

		if (ix1 != nullptr)
		{
			*ix1 = 0;
		}

		if (iy1 != nullptr)
		{
			*iy1 = 0;
		}
	}
	else
	{
		// move to integral bboxes (treating pixels as little squares, what pixels get touched)?
		if (ix0 != nullptr)
		{
			*ix0 = static_cast<std::int_fast32_t>(std::floor(x0 * scale_x + shift_x));
		}

		if (iy0 != nullptr)
		{
			*iy0 = static_cast<std::int_fast32_t>(std::floor(-y1 * scale_y + shift_y));
		}

		if (ix1 != nullptr)
		{
			*ix1 = static_cast<std::int_fast32_t>(std::ceil(x1 * scale_x + shift_x));
		}

		if (iy1 != nullptr)
		{
			*iy1 = static_cast<std::int_fast32_t>(std::ceil(-y0 * scale_y + shift_y));
		}
	}
}

inline void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo* font, const std::int_fast32_t glyph, const float scale_x, const float scale_y, std::int_fast32_t* ix0, std::int_fast32_t* iy0, std::int_fast32_t* ix1, std::int_fast32_t* iy1)
{
   stbtt_GetGlyphBitmapBoxSubpixel(font, glyph, scale_x, scale_y, 0.0F, 0.0F, ix0, iy0, ix1, iy1);
}

inline void stbtt_GetCodepointBitmapBoxSubpixel(const stbtt_fontinfo* font, const std::int_fast32_t codepoint, const float scale_x, const float scale_y, const float shift_x, const float shift_y, std::int_fast32_t* ix0, std::int_fast32_t* iy0, std::int_fast32_t* ix1, std::int_fast32_t* iy1)
{
   stbtt_GetGlyphBitmapBoxSubpixel(font, stbtt_FindGlyphIndex(font,codepoint), scale_x, scale_y, shift_x, shift_y, ix0, iy0, ix1, iy1);
}

inline void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo* font, const std::int_fast32_t codepoint, const float scale_x, const float scale_y, std::int_fast32_t* ix0, std::int_fast32_t* iy0, std::int_fast32_t* ix1, std::int_fast32_t* iy1)
{
   stbtt_GetCodepointBitmapBoxSubpixel(font, codepoint, scale_x, scale_y, 0.0F, 0.0F, ix0, iy0, ix1, iy1);
}

//////////////////////////////////////////////////////////////////////////////
//
//  Rasterizer

using stbtt__hheap_chunk = struct stbtt__hheap_chunk
{
	struct stbtt__hheap_chunk* next{};
};

using stbtt__hheap = struct stbtt__hheap
{
	struct stbtt__hheap_chunk* head{};
	void* first_free{};
	std::int_fast32_t num_remaining_in_head_chunk{};
};

inline static void *stbtt__hheap_alloc(stbtt__hheap* hh, const size_t size, void* userdata)
{
	if (hh->first_free != nullptr)
	{
		void* p{ hh->first_free };
		hh->first_free = *reinterpret_cast<void**>(p);
		return p;
	}

	if (hh->num_remaining_in_head_chunk == 0)
	{
		const std::int_fast32_t count{ (size < 32 ? 2000 : size < 128 ? 800 : 100) };
		stbtt__hheap_chunk* c{ new stbtt__hheap_chunk[sizeof(stbtt__hheap_chunk) + size * count] };

		if (c == nullptr)
		{
			return nullptr;
		}

		c->next = hh->head;
		hh->head = c;
		hh->num_remaining_in_head_chunk = count;
	}

	--hh->num_remaining_in_head_chunk;
	return reinterpret_cast<char*>(hh->head) + sizeof(stbtt__hheap_chunk) + size * hh->num_remaining_in_head_chunk;
}

inline static void stbtt__hheap_free(stbtt__hheap* hh, void* p)
{
	*reinterpret_cast<void**>(p) = hh->first_free;
	hh->first_free = p;
}

inline static void stbtt__hheap_cleanup(stbtt__hheap* hh, void* userdata)
{
	stbtt__hheap_chunk* c{ hh->head };

	while (c != nullptr)
	{
		stbtt__hheap_chunk* n{ c->next };
		static_cast<void>(userdata), free(c);

		c = n;
	}
}

using stbtt__edge = struct stbtt__edge
{
	float x0{};
	float y0{};
	float x1{};
	float y1{};
	std::int_fast32_t invert{};
};

using stbtt__active_edge = struct stbtt__active_edge
{
	struct stbtt__active_edge* next{};
	float fx{};
	float fdx{};
	float fdy{};
	float direction{};
	float sy{};
	float ey{};
};

inline static stbtt__active_edge* stbtt__new_active(stbtt__hheap* hh, stbtt__edge* e, const std::int_fast32_t off_x, const float start_point, void* userdata)
{
	stbtt__active_edge* z{ static_cast<stbtt__active_edge*>(stbtt__hheap_alloc(hh, sizeof(*z), userdata)) };
	const float dxdy{ (e->x1 - e->x0) / (e->y1 - e->y0) };

	if (z == nullptr)
	{
		return z;
	}

	z->fdx = dxdy;
	z->fdy = dxdy != 0.0F ? (1.0F / dxdy) : 0.0F; //-V550
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
	if (y0 == y1) //-V550
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
	else if (x0 >= x + 1 && x1 >= x + 1)
	{
	}
	else
	{
		scanline[x] += e->direction * (y1 - y0) * (1.0F - ((x0 - x) + (x1 - x)) / 2.0F); // coverage = 1 - average x position
	}
}

inline static void stbtt__fill_active_edges_new(float* scanline, float* scanline_fill, const std::int_fast32_t len, stbtt__active_edge* e, const float y_top)
{
	const float y_bottom{ y_top + 1.0F };

	while (e != nullptr)
	{
		// brute force every pixel

		// compute intersection points with top & bottom

		if (e->fdx == 0.0F)
		{
			const float x0{ e->fx };

			if (x0 < len)
			{
				if (x0 >= 0.0F)
				{
					stbtt__handle_clipped_edge(scanline, static_cast<std::int_fast32_t>(x0), e, x0, y_top, x0, y_bottom);
					stbtt__handle_clipped_edge(scanline_fill-1, static_cast<std::int_fast32_t>(x0) + 1, e, x0, y_top, x0, y_bottom);
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
			float dx{ e->fdx };
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

			float dy{ e->fdy };

			if (x_top >= 0.0F && x_bottom >= 0.0F && x_top < len && x_bottom < len)
			{
				// from here on, we don't have to range check x values

				if (static_cast<std::int_fast32_t>(x_top) == static_cast<std::int_fast32_t>(x_bottom))
				{
					// simple case, only spans one pixel
					const std::int_fast32_t x{ static_cast<std::int_fast32_t>(x_top) };
					const float height{ sy1 - sy0 };

					scanline[x] += e->direction * (1 - ((x_top - x) + (x_bottom - x)) / 2)  * height;
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

						dx = -dx;
						dy = -dy;
					}

					const std::int_fast32_t x1{ static_cast<std::int_fast32_t>(x_top) };
					const std::int_fast32_t x2{ static_cast<std::int_fast32_t>(x_bottom) };
					// compute intersection with y axis at x1+1
					float y_crossing{ (x1 + 1 - x0) * dy + y_top };
					const float sign{ e->direction };
					// area of the rectangle covered from y0..y_crossing
					float area{ sign * (y_crossing - sy0) };
					// area of the triangle (x_top,y0), (x+1,y0), (x+1,y_crossing)
					scanline[x1] += area * (1 - ((x_top - x1) + (x1 + 1 - x1)) / 2);
					const float step{ sign * dy };

					for (std::int_fast32_t x{ x1 + 1 }; x < x2; ++x)
					{
						scanline[x] += area + step / 2.0F;
						area += step;
					}

					y_crossing += dy * (x2 - (x1 + 1));
					scanline[x2] += area + sign * (1 - ((x2 - x2) + (x_bottom - x2)) / 2.0F) * (sy1 - y_crossing);
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
					const float x2{ static_cast<float>(x + 1) };
					const float y1{ (x - x0) / dx + y_top };
					const float y2{ (x + 1 - x0) / dx + y_top };

					if (x0 < x1 && xb > x2)
					{         // three segments descending down-right
						stbtt__handle_clipped_edge(scanline, x, e, x0, y_top, x1, y1);
						stbtt__handle_clipped_edge(scanline, x, e, x1, y1, x2, y2);
						stbtt__handle_clipped_edge(scanline, x, e, x2, y2, xb, y_bottom);
					}
					else if (xb < x1 && x0 > x2)
					{  // three segments descending down-left
						stbtt__handle_clipped_edge(scanline, x, e, x0, y_top, x2, y2);
						stbtt__handle_clipped_edge(scanline, x, e, x2, y2, x1, y1);
						stbtt__handle_clipped_edge(scanline, x, e, x1, y1, xb, y_bottom);
					}
					else if (x0 < x1 && xb > x1)
					{  // two segments across x, down-right
						stbtt__handle_clipped_edge(scanline, x, e, x0, y_top, x1, y1);
						stbtt__handle_clipped_edge(scanline, x, e, x1, y1, xb, y_bottom);
					}
					else if (xb < x1 && x0 > x1)
					{  // two segments across x, down-left
						stbtt__handle_clipped_edge(scanline, x, e, x0, y_top, x1, y1);
						stbtt__handle_clipped_edge(scanline, x, e, x1, y1, xb, y_bottom);
					}
					else if (x0 < x2 && xb > x2)
					{  // two segments across x+1, down-right
						stbtt__handle_clipped_edge(scanline, x, e, x0, y_top, x2, y2);
						stbtt__handle_clipped_edge(scanline, x, e, x2, y2, xb, y_bottom);
					}
					else if (xb < x2 && x0 > x2)
					{  // two segments across x+1, down-left
						stbtt__handle_clipped_edge(scanline, x, e, x0, y_top, x2, y2);
						stbtt__handle_clipped_edge(scanline, x, e, x2, y2, xb, y_bottom);
					}
					else
					{  // one segment
						stbtt__handle_clipped_edge(scanline, x, e, x0, y_top, xb, y_bottom);
					}
				}
			}
		}

		e = e->next;
	}
}

// directly AA rasterize edges w/o supersampling
inline static void stbtt__rasterize_sorted_edges(stbtt__bitmap* result, stbtt__edge* e, const std::int_fast32_t n, const std::int_fast32_t vsubsample, const std::int_fast32_t off_x, const std::int_fast32_t off_y, void* userdata)
{
	stbtt__hheap hh{};
	stbtt__active_edge* active{};
	std::vector<float> scanline_data(129);

	STBTT__NOTUSED(vsubsample);

	float* scanline{ (result->w > 64) ? new float[(result->w * 2 + 1) * sizeof(float)] : scanline_data.data() };
	float* scanline2{ scanline + result->w };
	std::int_fast32_t y{ off_y };

	e[n].y0 = static_cast<float>((off_y + result->h)) + 1.0F;

	std::int_fast32_t j{};

	while (j < result->h)
	{
		// find center of pixel for this scanline
		const float scan_y_top{ y + 0.0F };
		const float scan_y_bottom{ y + 1.0F };

		stbtt__active_edge** step{ &active };

		std::memset(scanline, 0, result->w * sizeof(scanline[0]));
		std::memset(scanline2, 0, (result->w + 1) * sizeof(scanline[0]));

		// update all active edges;
		// remove all active edges that terminate before the top of this scanline
		while (*step != nullptr)
		{
			stbtt__active_edge* z{ *step };

			if (z->ey <= scan_y_top)
			{
				*step = z->next; // delete from list
				z->direction = 0;
				stbtt__hheap_free(&hh, z);
			}
			else
			{
				step = &((*step)->next); // advance through list
			}
		}

		// insert all edges that start before the bottom of this scanline
		while (e->y0 <= scan_y_bottom)
		{
			if (e->y0 != e->y1)
			{
				stbtt__active_edge* z{ stbtt__new_active(&hh, e, off_x, scan_y_top, userdata) };

				if (z != nullptr)
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
			stbtt__fill_active_edges_new(scanline, scanline2 + 1, result->w, active, scan_y_top);
		}

		{
			float sum{};

			for (std::int_fast32_t i{}; i < result->w; ++i)
			{
				sum += scanline2[i];
				std::int_fast32_t m{ static_cast<std::int_fast32_t>(std::fabsf(scanline[i] + sum) * 255.0F + 0.5F) };

				if (m > 255)
				{
					m = 255;
				}

				result->pixels[j * result->stride + i] = static_cast<unsigned char>(m);
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

	stbtt__hheap_cleanup(&hh, userdata);

	if (scanline != scanline_data.data())
	{
		delete scanline;
	}
}

inline static void stbtt__sort_edges_ins_sort(stbtt__edge* p, const std::int_fast32_t n)
{
	for (std::int_fast32_t i{ 1 }; i < n; ++i)
	{
		stbtt__edge t{ p[i] };
		stbtt__edge* a{ &t };

		std::int_fast32_t j{ i };

		while (j > 0)
		{
			stbtt__edge* b{ &p[j - 1] };

			if (!(a->y0 < b->y0))
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
	/* threshold for transitioning to insertion sort */
	while (n > 12)
	{
		/* compute median of three */
		const std::int_fast32_t m{ n >> 1 };
		const std::int_fast32_t c01{ static_cast<std::int_fast32_t>((&p[0])->y0 < (&p[m])->y0) };
		const std::int_fast32_t c12{ static_cast<std::int_fast32_t>((&p[m])->y0 < (&p[n - 1])->y0) };

		/* if 0 >= mid >= end, or 0 < mid < end, then use mid */
		if (c01 != c12)
		{
			/* otherwise, we'll need to swap something else to middle */
			const std::int_fast32_t c{ static_cast<std::int_fast32_t>((&p[0])->y0 < (&p[n - 1])->y0) };

			/* 0>mid && mid<n:  0>n => n; 0<n => 0 */
			/* 0<mid && mid>n:  0>n => 0; 0<n => n */
			const std::int_fast32_t z{ (c == c12) ? 0 : n - 1 };

			std::swap(p[z], p[m]);
		}

		/* now p[m] is the median-of-three */
		/* swap it to the beginning so it won't move around */
		std::swap(p[0], p[m]);

		/* partition loop */
		std::int_fast32_t i{ 1 };
		std::int_fast32_t j{ n - 1 };

		for(;;)
		{
			/* handling of equality is crucial here */
			/* for sentinels & efficiency with duplicates */
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

			/* make sure we haven't crossed */
			if (i >= j)
			{
				break;
			}

			std::swap(p[i], p[j]);

			++i;
			--j;
		}

		/* recurse on smaller side, iterate on larger */
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

inline static void stbtt__sort_edges(stbtt__edge* p, const std::int_fast32_t n)
{
	stbtt__sort_edges_quicksort(p, n);
	stbtt__sort_edges_ins_sort(p, n);
}

using stbtt__point = struct
{
	float x{};
	float y{};
};

inline static void stbtt__rasterize(stbtt__bitmap* result, stbtt__point* pts, const std::int_fast32_t* wcount, const std::int_fast32_t windings, const float scale_x, const float scale_y, const float shift_x, const float shift_y, const std::int_fast32_t off_x, const std::int_fast32_t off_y, const std::int_fast32_t invert, void* userdata)
{
	constexpr std::int_fast32_t vsubsample{ 1 };
	// vsubsample should divide 255 evenly; otherwise we won't reach full opacity

	// now we have to blow out the windings into explicit edge lists
	std::int_fast32_t n{};

	for (std::int_fast32_t i{}; i < windings; ++i)
	{
		n += wcount[i];
	}

	stbtt__edge* e{ new stbtt__edge[sizeof(*e) * (n + 1)] }; // add an extra one as a sentinel

	if (e == nullptr)
	{
		return;
	}

	n = 0;

	std::int_fast32_t m{};
	const float y_scale_inv{ (invert != 0) ? -scale_y : scale_y };

	for (std::int_fast32_t i{}; i < windings; ++i)
	{
		const stbtt__point* p{ pts + m };
		m += wcount[i];
		std::int_fast32_t j{ wcount[i] - 1 };

		for (std::int_fast32_t k{}; k < wcount[i]; j = k++)
		{
			std::int_fast32_t a{ k };
			std::int_fast32_t b{ j };

			// skip the edge if horizontal
			if (p[j].y == p[k].y)
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
			e[n].y0 = (p[a].y * y_scale_inv + shift_y) * vsubsample;
			e[n].x1 = p[b].x * scale_x + shift_x;
			e[n].y1 = (p[b].y * y_scale_inv + shift_y) * vsubsample;

			++n;
		}
	}

	// now sort the edges by their highest point (should snap to integer, and then by x)
	//STBTT_sort(e, n, sizeof(e[0]), stbtt__edge_compare);
	stbtt__sort_edges(e, n);

	// now, traverse the scanlines and find the intersections on each scanline, use xor winding rule
	stbtt__rasterize_sorted_edges(result, e, n, vsubsample, off_x, off_y, userdata);

	delete e;
}

inline static void stbtt__add_point(stbtt__point* points, const std::int_fast32_t n, const float x, const float y)
{
	if (points == nullptr)
	{
		return; // during first pass, it's unallocated
	}

	points[n].x = x;
	points[n].y = y;
}

// tessellate until threshold p is happy... @TODO warped to compensate for non-linear stretching
inline static std::int_fast32_t stbtt__tesselate_curve(stbtt__point* points, std::int_fast32_t* num_points, const float x0, const float y0, const float x1, const float y1, const float x2, const float y2, const float objspace_flatness_squared, const std::int_fast32_t n)
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
		stbtt__add_point(points, *num_points, x2, y2);
		*num_points = *num_points + 1;
	}

	return 1;
}

inline static void stbtt__tesselate_cubic(stbtt__point* points, std::int_fast32_t* num_points, const float x0, const float y0, const float x1, const float y1, const float x2, const float y2, const float x3, const float y3, const float objspace_flatness_squared, const std::int_fast32_t n)
{
	if (n > 16) // 65536 segments on one curve better be enough!
	{
		return;
	}

	// @TODO this "flatness" calculation is just made-up nonsense that seems to work well enough
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
		stbtt__add_point(points, *num_points, x3, y3);
		*num_points = *num_points +1 ;
	}
}

// returns number of contours
inline static stbtt__point* stbtt_FlattenCurves(stbtt_vertex* vertices, const std::int_fast32_t num_verts, const float objspace_flatness, std::int_fast32_t** contour_lengths, std::int_fast32_t* num_contours, void* userdata)
{
	stbtt__point* points{};
	std::int_fast32_t num_points{};
	const float objspace_flatness_squared{ objspace_flatness * objspace_flatness };
	std::int_fast32_t n{};
	std::int_fast32_t start{};

	// count how many "moves" there are to get the contour count
	for (std::int_fast32_t i{}; i < num_verts; ++i)
	{
		if (vertices[i].type == static_cast<unsigned char>(GlyphShapeType::STBTT_vmove))
		{
			++n;
		}
	}

	*num_contours = n;

	if (n == 0)
	{
		return nullptr;
	}

	*contour_lengths = new int_fast32_t[sizeof(**contour_lengths) * n];

	if (*contour_lengths == nullptr)
	{
		*num_contours = 0;
		return nullptr;
	}

	// make two passes through the points so we don't need to realloc
	for (std::int_fast32_t pass{}; pass < 2; ++pass)
	{
		float x{};
		float y{};

		if (pass == 1)
		{
			points = new stbtt__point[num_points * sizeof(points[0])];

			if (points == nullptr)
			{
				goto error;
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
						(*contour_lengths)[n] = num_points - start;
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
					stbtt__tesselate_curve(points, &num_points, x, y, vertices[i].cx, vertices[i].cy, vertices[i].x, vertices[i].y,	objspace_flatness_squared, 0);
					x = vertices[i].x;
					y = vertices[i].y;
					break;
				}
				case static_cast<unsigned char>(GlyphShapeType::STBTT_vcubic):
				{
					stbtt__tesselate_cubic(points, &num_points, x, y, vertices[i].cx, vertices[i].cy, vertices[i].cx1, vertices[i].cy1, vertices[i].x, vertices[i].y, objspace_flatness_squared, 0);
					x = vertices[i].x;
					y = vertices[i].y;
					break;
				}
			}
		}

		(*contour_lengths)[n] = num_points - start;
	}

	return points;

	error:
		delete points;
		delete *contour_lengths;
		*num_contours = 0;
		return nullptr;
}

inline void stbtt_Rasterize(stbtt__bitmap* result, const float flatness_in_pixels, stbtt_vertex* vertices, const std::int_fast32_t num_verts, const float scale_x, const float scale_y, const float shift_x, const float shift_y, const std::int_fast32_t x_off, const std::int_fast32_t y_off, const std::int_fast32_t invert, void* userdata)
{
	const float scale{ scale_x > scale_y ? scale_y : scale_x };
	std::int_fast32_t winding_count{};
	std::unique_ptr<std::int_fast32_t> winding_lengths;

	stbtt__point* windings{ stbtt_FlattenCurves(vertices, num_verts, flatness_in_pixels / scale, reinterpret_cast<std::int_fast32_t**>(&winding_lengths), &winding_count, userdata) };

	if (windings != nullptr)
	{
		stbtt__rasterize(result, windings, winding_lengths.get(), winding_count, scale_x, scale_y, shift_x, shift_y, x_off, y_off, invert, userdata);
		static_cast<void>(userdata), free(windings);
	}
}

unsigned char* stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo* info, float scale_x, float scale_y, const float shift_x, const float shift_y, const std::int_fast32_t glyph, std::int_fast32_t* width, std::int_fast32_t* height, std::int_fast32_t* xoff, std::int_fast32_t* yoff)
{
	std::unique_ptr<stbtt_vertex> vertices;
	const std::int_fast32_t num_verts{ stbtt_GetGlyphShape(info, glyph, reinterpret_cast<stbtt_vertex**>(&vertices)) };

	if (scale_x == 0.0F) //-V550
	{
		scale_x = scale_y;
	}

	if (scale_y == 0.0F) //-V550
	{
		if (scale_x == 0.0F) //-V550
		{
			return nullptr;
		}

		scale_y = scale_x;
	}

	std::int_fast32_t ix0{};
	std::int_fast32_t iy0{};
	std::int_fast32_t ix1{};
	std::int_fast32_t iy1{};

	stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0, &iy0, &ix1, &iy1);
	stbtt__bitmap gbm{};

	// now we get the size
	gbm.w = (ix1 - ix0);
	gbm.h = (iy1 - iy0);
	gbm.pixels = nullptr; // in case we error

	if (width != nullptr)
	{
		*width = gbm.w;
	}

	if (height != nullptr)
	{
		*height = gbm.h;
	}

	if (xoff != nullptr)
	{
		*xoff = ix0;
	}

	if (yoff != nullptr)
	{
		*yoff = iy0;
	}

	if (gbm.w != 0 && gbm.h != 0)
	{
		gbm.pixels = new unsigned char[gbm.w * gbm.h];

		if (gbm.pixels != nullptr)
		{
			gbm.stride = gbm.w;
			stbtt_Rasterize(&gbm, 0.35F, vertices.get(), num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1, info->userdata);
		}
	}

	return gbm.pixels;
}

inline unsigned char* stbtt_GetGlyphBitmap(const stbtt_fontinfo* info, const float scale_x, const float scale_y, const std::int_fast32_t glyph, std::int_fast32_t* width, std::int_fast32_t* height, std::int_fast32_t* xoff, std::int_fast32_t* yoff)
{
	return stbtt_GetGlyphBitmapSubpixel(info, scale_x, scale_y, 0.0F, 0.0F, glyph, width, height, xoff, yoff);
}

inline void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo* info, unsigned char* output, std::int_fast32_t out_w, std::int_fast32_t out_h, std::int_fast32_t out_stride, float scale_x, float scale_y, float shift_x, float shift_y, std::int_fast32_t glyph)
{
	stbtt_vertex* vertices{};
	const std::int_fast32_t num_verts{ stbtt_GetGlyphShape(info, glyph, &vertices) };
	std::int_fast32_t ix0{};
	std::int_fast32_t iy0{};

	stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0, &iy0, nullptr, nullptr);

	stbtt__bitmap gbm{ out_w, out_h, out_stride, output };

	if (gbm.w != 0 && gbm.h != 0)
	{
		stbtt_Rasterize(&gbm, 0.35F, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1, info->userdata);
	}

	static_cast<void>(info->userdata), free(vertices);
}

inline void stbtt_MakeGlyphBitmap(const stbtt_fontinfo* info, unsigned char* output, const std::int_fast32_t out_w, const std::int_fast32_t out_h, const std::int_fast32_t out_stride, const float scale_x, const float scale_y, const std::int_fast32_t glyph)
{
	stbtt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0F,0.0F, glyph);
}

inline unsigned char* stbtt_GetCodepointBitmapSubpixel(const stbtt_fontinfo* info, const float scale_x, const float scale_y, const float shift_x, const float shift_y, const std::int_fast32_t codepoint, std::int_fast32_t* width, std::int_fast32_t* height, std::int_fast32_t* xoff, std::int_fast32_t* yoff)
{
	return stbtt_GetGlyphBitmapSubpixel(info, scale_x, scale_y,shift_x,shift_y, stbtt_FindGlyphIndex(info, codepoint), width, height, xoff, yoff);
}

void stbtt_MakeCodepointBitmapSubpixelPrefilter(const stbtt_fontinfo* info, unsigned char* output, const std::int_fast32_t out_w, const std::int_fast32_t out_h, const std::int_fast32_t out_stride, const float scale_x, const float scale_y, const float shift_x, const float shift_y, const std::int_fast32_t prefilter_x, const std::int_fast32_t prefilter_y, float *sub_x, float *sub_y, const std::int_fast32_t codepoint)
{
	stbtt_MakeGlyphBitmapSubpixelPrefilter(info, output, out_w, out_h, out_stride, scale_x, scale_y, shift_x, shift_y, prefilter_x, prefilter_y, sub_x, sub_y, stbtt_FindGlyphIndex(info, codepoint));
}

inline void stbtt_MakeCodepointBitmapSubpixel(const stbtt_fontinfo* info, unsigned char* output, const std::int_fast32_t out_w, const std::int_fast32_t out_h, const std::int_fast32_t out_stride, const float scale_x, const float scale_y, const float shift_x, const float shift_y, const std::int_fast32_t codepoint)
{
	stbtt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, shift_x, shift_y, stbtt_FindGlyphIndex(info, codepoint));
}

inline unsigned char* stbtt_GetCodepointBitmap(const stbtt_fontinfo* info, const float scale_x, const float scale_y, const std::int_fast32_t codepoint, std::int_fast32_t* width, std::int_fast32_t* height, std::int_fast32_t* xoff, std::int_fast32_t* yoff)
{
   return stbtt_GetCodepointBitmapSubpixel(info, scale_x, scale_y, 0.0F, 0.0F, codepoint, width, height, xoff, yoff);
}

inline void stbtt_MakeCodepointBitmap(const stbtt_fontinfo* info, unsigned char* output, const std::int_fast32_t out_w, const std::int_fast32_t out_h, const std::int_fast32_t out_stride, const float scale_x, const float scale_y, const std::int_fast32_t codepoint)
{
	stbtt_MakeCodepointBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0F, 0.0F, codepoint);
}

//////////////////////////////////////////////////////////////////////////////
//
// bitmap baking
//
// This is SUPER-CRAPPY packing to keep source code small

inline static std::int_fast32_t stbtt_BakeFontBitmap_internal(const unsigned char* data, const std::int_fast32_t offset, const float pixel_height, unsigned char* pixels, const std::int_fast32_t pw, const std::int_fast32_t ph, const std::int_fast32_t first_char, const std::int_fast32_t num_chars, stbtt_bakedchar* chardata)
{
	std::int_fast32_t x{ 1 };
	std::int_fast32_t y{ 1 };
	std::int_fast32_t bottom_y{ 1 };

	stbtt_fontinfo f{};
	f.userdata = nullptr;

	if (stbtt_InitFont(&f, data, offset) == 0)
	{
		return -1;
	}

	std::memset(pixels, 0, pw * ph); // background of 0 around pixels

	float scale{ stbtt_ScaleForPixelHeight(&f, pixel_height) };

	for (std::int_fast32_t i{}; i < num_chars; ++i)
	{
		std::int_fast32_t advance{};
		std::int_fast32_t lsb{};
		std::int_fast32_t x0{};
		std::int_fast32_t y0{};
		std::int_fast32_t x1{};
		std::int_fast32_t y1{};
		const std::int_fast32_t g{ stbtt_FindGlyphIndex(&f, first_char + i) };

		stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
		stbtt_GetGlyphBitmapBox(&f, g, scale, scale, &x0, &y0, &x1, &y1);

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

		stbtt_MakeGlyphBitmap(&f, pixels + x + y * pw, gw, gh, pw, scale, scale, g);

		chardata[i].x0 = x;
		chardata[i].y0 = y;
		chardata[i].x1 = x + gw;
		chardata[i].y1 = y + gh;
		chardata[i].xadvance = scale * advance;
		chardata[i].xoff = static_cast<float>(x0);
		chardata[i].yoff = static_cast<float>(y0);

		x = x + gw + 1;

		if (y + gh + 1 > bottom_y)
		{
			bottom_y = y + gh + 1;
		}
	}

	return bottom_y;
}

inline void stbtt_GetBakedQuad(const stbtt_bakedchar* chardata, const std::int_fast32_t pw, const std::int_fast32_t ph, const std::int_fast32_t char_index, float* xpos, const float* ypos, stbtt_aligned_quad* q, const std::int_fast32_t opengl_fillrule)
{
	const float d3d_bias{ (opengl_fillrule != 0) ? 0.0F : -0.5F };
	const float ipw{ 1.0F / pw };
	const float iph{ 1.0F / ph };
	const stbtt_bakedchar* b{ chardata + char_index };
	const std::int_fast32_t round_x{ static_cast<std::int_fast32_t>(std::floor((*xpos + b->xoff) + 0.5F)) };
	const std::int_fast32_t round_y{ static_cast<std::int_fast32_t>(std::floor((*ypos + b->yoff) + 0.5F)) };

	q->x0 = round_x + d3d_bias;
	q->y0 = round_y + d3d_bias;
	q->x1 = round_x + b->x1 - b->x0 + d3d_bias;
	q->y1 = round_y + b->y1 - b->y0 + d3d_bias;

	q->s0 = b->x0 * ipw;
	q->t0 = b->y0 * iph;
	q->s1 = b->x1 * ipw;
	q->t1 = b->y1 * iph;

	*xpos += b->xadvance;
}

//////////////////////////////////////////////////////////////////////////////
//
// rectangle packing replacement routines if you don't have stb_rect_pack.h
//

using stbrp_context = struct
{
	std::int_fast32_t width{};
	std::int_fast32_t height{};
	std::int_fast32_t x{};
	std::int_fast32_t y{};
	std::int_fast32_t bottom_y{};
};

using stbrp_node = struct
{
	unsigned char x{};
};

struct stbrp_rect
{
	std::int_fast32_t x{};
	std::int_fast32_t y{};
	std::int_fast32_t id{};
	std::int_fast32_t w{};
	std::int_fast32_t h{};
	std::int_fast32_t was_packed{};
};

inline static void stbrp_init_target(stbrp_context* con, const std::int_fast32_t pw, const std::int_fast32_t ph, stbrp_node* nodes, const std::int_fast32_t num_nodes)
{
	con->width = pw;
	con->height = ph;
	con->x = 0;
	con->y = 0;
	con->bottom_y = 0;
	STBTT__NOTUSED(nodes);
	STBTT__NOTUSED(num_nodes);
}

inline static void stbrp_pack_rects(stbrp_context* con, stbrp_rect* rects, const std::int_fast32_t num_rects)
{
	for (std::int_fast32_t i{}; i < num_rects; ++i)
	{
		if (con->x + rects[i].w > con->width)
		{
			con->x = 0;
			con->y = con->bottom_y;
		}

		if (con->y + rects[i].h > con->height)
		{
			break;
		}

		rects[i].x = con->x;
		rects[i].y = con->y;
		rects[i].was_packed = 1;
		con->x += rects[i].w;

		if (con->y + rects[i].h > con->bottom_y)
		{
			con->bottom_y = con->y + rects[i].h;
		}
	}

	for (std::int_fast32_t i{}; i < num_rects; ++i)
	{
		rects[i].was_packed = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// bitmap baking
//
// This is SUPER-AWESOME (tm Ryan Gordon) packing.

inline std::int_fast32_t stbtt_PackBegin(stbtt_pack_context* spc, unsigned char* pixels, const std::int_fast32_t pw, const std::int_fast32_t ph, const std::int_fast32_t stride_in_bytes, const std::int_fast32_t padding, void* alloc_context)
{
	stbrp_context* context{ new stbrp_context[sizeof(*context)] };
	const std::int_fast32_t num_nodes{ pw - padding };
	stbrp_node* nodes{ new stbrp_node[sizeof(*nodes) * num_nodes] };

	if (context == nullptr || nodes == nullptr)
	{
		if (context != nullptr)
		{
			delete context;
		}

		if (nodes != nullptr)
		{
			delete nodes;
		}

		return 0;
	}

	spc->user_allocator_context = alloc_context;
	spc->width = pw;
	spc->height = ph;
	spc->pixels = pixels;
	spc->pack_info = context;
	spc->nodes = nodes;
	spc->padding = padding;
	spc->stride_in_bytes = stride_in_bytes != 0 ? stride_in_bytes : pw;
	spc->h_oversample = 1;
	spc->v_oversample = 1;
	spc->skip_missing = 0;

	stbrp_init_target(context, pw-padding, ph-padding, nodes, num_nodes);

	if (pixels != nullptr)
	{
		std::memset(pixels, 0, pw * ph); // background of 0 around pixels
	}

	return 1;
}

inline void stbtt_PackEnd (stbtt_pack_context* spc)
{
	static_cast<void>(spc->user_allocator_context), free(spc->nodes);
	static_cast<void>(spc->user_allocator_context), free(spc->pack_info);
}

inline void stbtt_PackSetOversampling(stbtt_pack_context* spc, const std::int_fast32_t h_oversample, const std::int_fast32_t v_oversample)
{
	if (h_oversample <= STBTT_MAX_OVERSAMPLE)
	{
		spc->h_oversample = h_oversample;
	}

	if (v_oversample <= STBTT_MAX_OVERSAMPLE)
	{
		spc->v_oversample = v_oversample;
	}
}

inline void stbtt_PackSetSkipMissingCodepoints(stbtt_pack_context* spc, const std::int_fast32_t skip)
{
	spc->skip_missing = skip;
}

inline static void stbtt__h_prefilter(unsigned char* pixels, const std::int_fast32_t w, const std::int_fast32_t h, const std::int_fast32_t stride_in_bytes, const std::int_fast32_t kernel_width)
{
	unsigned char buffer[STBTT_MAX_OVERSAMPLE];
	const std::int_fast32_t safe_w{ w - kernel_width };

	std::memset(buffer, 0, STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze

	for (std::int_fast32_t j{}; j < h; ++j)
	{
		std::int_fast32_t total{};
		std::memset(buffer, 0, kernel_width);

		// make kernel_width a constant in common cases so compiler can optimize out the divide
		switch (kernel_width)
		{
			case 2:
			{
				for (std::int_fast32_t i{}; i <= safe_w; ++i)
				{
					total += pixels[i] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i];
					pixels[i] = static_cast<unsigned char>(total / 2);
				}
				break;
			}
			case 3:
			{
				for (std::int_fast32_t i{}; i <= safe_w; ++i)
				{
					total += pixels[i] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i];
					pixels[i] = static_cast<unsigned char>(total / 3);
				}
				break;
			}
			case 4:
			{
				for (std::int_fast32_t i{}; i <= safe_w; ++i)
				{
					total += pixels[i] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i];
					pixels[i] = static_cast<unsigned char>(total / 4);
				}
				break;
			}
			case 5:
			{
				for (std::int_fast32_t i{}; i <= safe_w; ++i)
				{
					total += pixels[i] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i];
					pixels[i] = static_cast<unsigned char>(total / 5);
				}
				break;
			}
			default:
			{
				for (std::int_fast32_t i{}; i <= safe_w; ++i)
				{
					total += pixels[i] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i];
					pixels[i] = static_cast<unsigned char>(total / kernel_width);
				}
				break;
			}
		}

		for (std::int_fast32_t i{}; i < w; ++i)
		{
			total -= buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
			pixels[i] = static_cast<unsigned char>(total / kernel_width);
		}

		pixels += stride_in_bytes;
	}
}

inline static void stbtt__v_prefilter(unsigned char* pixels, const std::int_fast32_t w, const std::int_fast32_t h, const std::int_fast32_t stride_in_bytes, const std::int_fast32_t kernel_width)
{
	unsigned char buffer[STBTT_MAX_OVERSAMPLE];
	const std::int_fast32_t safe_h{ h - kernel_width };

	std::memset(buffer, 0, STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze

	for (std::int_fast32_t j{}; j < w; ++j)
	{
		std::int_fast32_t total{};
		std::memset(buffer, 0, kernel_width);

		// make kernel_width a constant in common cases so compiler can optimize out the divide
		switch (kernel_width)
		{
			case 2:
			{
				for (std::int_fast32_t i{}; i <= safe_h; ++i)
				{
					total += pixels[i * stride_in_bytes] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i * stride_in_bytes];
					pixels[i * stride_in_bytes] = static_cast<unsigned char>(total / 2);
				}
				break;
			}
			case 3:
			{
				for (std::int_fast32_t i{}; i <= safe_h; ++i)
				{
					total += pixels[i * stride_in_bytes] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i * stride_in_bytes];
					pixels[i * stride_in_bytes] = static_cast<unsigned char>(total / 3);
				}
				break;
			}
			case 4:
			{
				for (std::int_fast32_t i{}; i <= safe_h; ++i)
				{
					total += pixels[i * stride_in_bytes] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i * stride_in_bytes];
					pixels[i * stride_in_bytes] = static_cast<unsigned char>(total / 4);
				}
				break;
			}
			case 5:
			{
				for (std::int_fast32_t i{}; i <= safe_h; ++i) {
					total += pixels[i * stride_in_bytes] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i * stride_in_bytes];
					pixels[i * stride_in_bytes] = static_cast<unsigned char>(total / 5);
				}
				break;
			}
			default:
			{
				for (std::int_fast32_t i{}; i <= safe_h; ++i)
				{
					total += pixels[i * stride_in_bytes] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i * stride_in_bytes];
					pixels[i * stride_in_bytes] = static_cast<unsigned char>(total / kernel_width);
				}
				break;
			}
		}

		for (std::int_fast32_t i{}; i < h; ++i)
		{
			total -= buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
			pixels[i*stride_in_bytes] = static_cast<unsigned char>(total / kernel_width);
		}

		pixels += 1;
	}
}

inline static float stbtt__oversample_shift(const std::int_fast32_t oversample)
{
	if (oversample == 0)
	{
		return 0.0F;
	}

	// The prefilter is a box filter of width "oversample",
	// which shifts phase by (oversample - 1)/2 pixels in
	// oversampled space. We want to shift in the opposite
	// direction to counter this.
	return static_cast<float>(-(oversample - 1)) / (2.0F * static_cast<float>(oversample));
}

// rects array must be big enough to accommodate all characters in the given ranges
inline std::int_fast32_t stbtt_PackFontRangesGatherRects(stbtt_pack_context* spc, const stbtt_fontinfo* info, stbtt_pack_range* ranges, const std::int_fast32_t num_ranges, stbrp_rect* rects)
{
	std::int_fast32_t k{};

	for (std::int_fast32_t i{}; i < num_ranges; ++i)
	{
		const float fh{ ranges[i].font_size };
		const float scale{ fh > 0.0F ? stbtt_ScaleForPixelHeight(info, fh) : stbtt_ScaleForMappingEmToPixels(info, -fh) };

		ranges[i].h_oversample = static_cast<unsigned char>(spc->h_oversample);
		ranges[i].v_oversample = static_cast<unsigned char>(spc->v_oversample);

		for (std::int_fast32_t j{}; j < ranges[i].num_chars; ++j)
		{
			std::int_fast32_t x0{};
			std::int_fast32_t y0{};
			std::int_fast32_t x1{};
			std::int_fast32_t y1{};

			const std::int_fast32_t codepoint{ ranges[i].array_of_unicode_codepoints == nullptr ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j] };
			const std::int_fast32_t glyph{ stbtt_FindGlyphIndex(info, codepoint) };

			if (glyph == 0 && spc->skip_missing != 0)
			{
				rects[k].w = 0;
				rects[k].h = 0;
			}
			else
			{
				stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale * spc->h_oversample, scale * spc->v_oversample, 0,0, &x0, &y0, &x1, &y1);
				rects[k].w = x1 - x0 + spc->padding + spc->h_oversample - 1;
				rects[k].h = y1 - y0 + spc->padding + spc->v_oversample - 1;
			}

			++k;
		}
	}

	return k;
}

inline void stbtt_MakeGlyphBitmapSubpixelPrefilter(const stbtt_fontinfo* info, unsigned char* output, const std::int_fast32_t out_w, const std::int_fast32_t out_h, const std::int_fast32_t out_stride, const float scale_x, const float scale_y, const float shift_x, const float shift_y, const std::int_fast32_t prefilter_x, const std::int_fast32_t prefilter_y, float* sub_x, float* sub_y, const std::int_fast32_t glyph)
{
	stbtt_MakeGlyphBitmapSubpixel(info, output, out_w - (prefilter_x - 1), out_h - (prefilter_y - 1), out_stride, scale_x, scale_y, shift_x, shift_y, glyph);

	if (prefilter_x > 1)
	{
		stbtt__h_prefilter(output, out_w, out_h, out_stride, prefilter_x);
	}

	if (prefilter_y > 1)
	{
		stbtt__v_prefilter(output, out_w, out_h, out_stride, prefilter_y);
	}

	*sub_x = stbtt__oversample_shift(prefilter_x);
	*sub_y = stbtt__oversample_shift(prefilter_y);
}

// rects array must be big enough to accommodate all characters in the given ranges
inline std::int_fast32_t stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context* spc, const stbtt_fontinfo* info, stbtt_pack_range* ranges, const std::int_fast32_t num_ranges, stbrp_rect* rects)
{
	std::int_fast32_t k{};
	std::int_fast32_t return_value{ 1 };

	// save current values
	const std::int_fast32_t old_h_over{ spc->h_oversample };
	const std::int_fast32_t old_v_over{ spc->v_oversample };

	for (std::int_fast32_t i{}; i < num_ranges; ++i)
	{
		const float fh{ ranges[i].font_size };
		const float scale{ fh > 0.0F ? stbtt_ScaleForPixelHeight(info, fh) : stbtt_ScaleForMappingEmToPixels(info, -fh) };
		float recip_h{};
		float recip_v{};
		float sub_x{};
		float sub_y{};

		spc->h_oversample = ranges[i].h_oversample;
		spc->v_oversample = ranges[i].v_oversample;
		recip_h = 1.0F / spc->h_oversample;
		recip_v = 1.0F / spc->v_oversample;
		sub_x = stbtt__oversample_shift(spc->h_oversample);
		sub_y = stbtt__oversample_shift(spc->v_oversample);

		for (std::int_fast32_t j{}; j < ranges[i].num_chars; ++j)
		{
			stbrp_rect* r{ &rects[k] };

			if (r->was_packed != 0 && r->w != 0 && r->h != 0)
			{
				stbtt_packedchar* bc{ &ranges[i].chardata_for_range[j] };
				std::int_fast32_t advance{};
				std::int_fast32_t lsb{};
				std::int_fast32_t x0{};
				std::int_fast32_t y0{};
				std::int_fast32_t x1{};
				std::int_fast32_t y1{};
				std::int_fast32_t codepoint{ ranges[i].array_of_unicode_codepoints == nullptr ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j] };
				std::int_fast32_t glyph{ stbtt_FindGlyphIndex(info, codepoint) };
				std::int_fast32_t pad{ spc->padding };

				// pad on left and top
				r->x += pad;
				r->y += pad;
				r->w -= pad;
				r->h -= pad;

				stbtt_GetGlyphHMetrics(info, glyph, &advance, &lsb);
				stbtt_GetGlyphBitmapBox(info, glyph, scale * spc->h_oversample, scale * spc->v_oversample, &x0, &y0, &x1, &y1);
				stbtt_MakeGlyphBitmapSubpixel(info, spc->pixels + r->x + r->y * spc->stride_in_bytes, r->w - spc->h_oversample+1, r->h - spc->v_oversample + 1, spc->stride_in_bytes, scale * spc->h_oversample, scale * spc->v_oversample, 0, 0, glyph);

				if (spc->h_oversample > 1)
				{
					stbtt__h_prefilter(spc->pixels + r->x + r->y * spc->stride_in_bytes, r->w, r->h, spc->stride_in_bytes, spc->h_oversample);
				}

				if (spc->v_oversample > 1)
				{
					stbtt__v_prefilter(spc->pixels + r->x + r->y * spc->stride_in_bytes, r->w, r->h, spc->stride_in_bytes, spc->v_oversample);
				}

				bc->x0 = r->x;
				bc->y0 = r->y;
				bc->x1 = r->x + r->w;
				bc->y1 = r->y + r->h;
				bc->xadvance = scale * advance;
				bc->xoff = static_cast<float>(x0) * recip_h + sub_x;
				bc->yoff = static_cast<float>(y0) * recip_v + sub_y;
				bc->xoff2 = (x0 + r->w) * recip_h + sub_x;
				bc->yoff2 = (y0 + r->h) * recip_v + sub_y;
			}
			else
			{
				return_value = 0; // if any fail, report failure
			}

			++k;
		}
	}

	// restore original values
	spc->h_oversample = old_h_over;
	spc->v_oversample = old_v_over;

	return return_value;
}

inline void stbtt_PackFontRangesPackRects(stbtt_pack_context* spc, stbrp_rect* rects, const std::int_fast32_t num_rects)
{
	stbrp_pack_rects(static_cast<stbrp_context*>(spc->pack_info), rects, num_rects);
}

inline std::int_fast32_t stbtt_PackFontRanges(stbtt_pack_context* spc, const unsigned char* fontdata, const std::int_fast32_t font_index, stbtt_pack_range* ranges, const std::int_fast32_t num_ranges)
{
	stbtt_fontinfo info{};
	std::int_fast32_t n{};
	std::int_fast32_t return_value{ 1 };
	//stbrp_context *context = (stbrp_context *) spc->pack_info;
	stbrp_rect* rects{};

	// flag all characters as NOT packed
	for (std::int_fast32_t i{}; i < num_ranges; ++i)
	{
		for (std::int_fast32_t j{}; j < ranges[i].num_chars; ++j)
		{
			ranges[i].chardata_for_range[j].x0 = 0;
			ranges[i].chardata_for_range[j].y0 = 0;
			ranges[i].chardata_for_range[j].x1 = 0;
			ranges[i].chardata_for_range[j].y1 = 0;
		}
	}

	for (std::int_fast32_t i{}; i < num_ranges; ++i)
	{
		n += ranges[i].num_chars;
	}

	rects = new stbrp_rect[sizeof(*rects) * n];

	if (rects == nullptr)
	{
		return 0;
	}

	info.userdata = spc->user_allocator_context;
	stbtt_InitFont(&info, fontdata, stbtt_GetFontOffsetForIndex(fontdata,font_index));
	n = stbtt_PackFontRangesGatherRects(spc, &info, ranges, num_ranges, rects);
	stbtt_PackFontRangesPackRects(spc, rects, n);
	return_value = stbtt_PackFontRangesRenderIntoRects(spc, &info, ranges, num_ranges, rects);

	delete rects;
	return return_value;
}

inline std::int_fast32_t stbtt_PackFontRange(stbtt_pack_context* spc, const unsigned char* fontdata, const std::int_fast32_t font_index, const float font_size, const std::int_fast32_t first_unicode_codepoint_in_range, const std::int_fast32_t num_chars_in_range, stbtt_packedchar* chardata_for_range)
{
	stbtt_pack_range range{};
	range.first_unicode_codepoint_in_range = first_unicode_codepoint_in_range;
	range.array_of_unicode_codepoints = nullptr;
	range.num_chars = num_chars_in_range;
	range.chardata_for_range = chardata_for_range;
	range.font_size = font_size;
	return stbtt_PackFontRanges(spc, fontdata, font_index, &range, 1);
}

inline void stbtt_GetScaledFontVMetrics(const unsigned char* fontdata, const std::int_fast32_t index, const float size, float* ascent, float* descent, float* lineGap)
{
	stbtt_fontinfo info{};
	stbtt_InitFont(&info, fontdata, stbtt_GetFontOffsetForIndex(fontdata, index));

	const float scale{ (size > 0) ? stbtt_ScaleForPixelHeight(&info, size) : stbtt_ScaleForMappingEmToPixels(&info, -size) };

	std::int_fast32_t i_ascent{};
	std::int_fast32_t i_descent{};
	std::int_fast32_t i_lineGap{};

	stbtt_GetFontVMetrics(&info, &i_ascent, &i_descent, &i_lineGap);
	*ascent  = static_cast<float>(i_ascent) * scale;
	*descent = static_cast<float>(i_descent) * scale;
	*lineGap = static_cast<float>(i_lineGap) * scale;
}

inline void stbtt_GetPackedQuad(const stbtt_packedchar* chardata, const std::int_fast32_t pw, const std::int_fast32_t ph, const std::int_fast32_t char_index, float* xpos, const float* ypos, stbtt_aligned_quad* q, const std::int_fast32_t align_to_integer)
{
	float ipw{ 1.0F / pw };
	float iph{ 1.0F / ph };
	const stbtt_packedchar* b{ chardata + char_index };

	if (align_to_integer != 0)
	{
		auto x{ static_cast<float>(static_cast<std::int_fast32_t>(std::floor((*xpos + b->xoff) + 0.5f))) };
		auto y{ static_cast<float>(static_cast<std::int_fast32_t>(std::floor((*ypos + b->yoff) + 0.5f))) };

		q->x0 = x;
		q->y0 = y;
		q->x1 = x + b->xoff2 - b->xoff;
		q->y1 = y + b->yoff2 - b->yoff;
	}
	else
	{
		q->x0 = *xpos + b->xoff;
		q->y0 = *ypos + b->yoff;
		q->x1 = *xpos + b->xoff2;
		q->y1 = *ypos + b->yoff2;
	}

	q->s0 = b->x0 * ipw;
	q->t0 = b->y0 * iph;
	q->s1 = b->x1 * ipw;
	q->t1 = b->y1 * iph;

	*xpos += b->xadvance;
}

//////////////////////////////////////////////////////////////////////////////
//
// sdf computation
//

inline static std::int_fast32_t stbtt__ray_intersect_bezier(const float orig[2], const float ray[2], const float q0[2], const float q1[2], const float q2[2], float hits[2][2])
{
	const float q0perp{ q0[1] * ray[0] - q0[0] * ray[1] };
	const float q1perp{ q1[1] * ray[0] - q1[0] * ray[1] };
	const float q2perp{ q2[1] * ray[0] - q2[0] * ray[1] };
	const float roperp{ orig[1] * ray[0] - orig[0] * ray[1] };

	const float a{ q0perp - 2.0F * q1perp + q2perp };
	const float b{ q1perp - q0perp };
	const float c{ q0perp - roperp };

	float s0{};
	float s1{};
	std::int_fast32_t num_s{};

	if (a != 0.0F) //-V550
	{
		const float discr{ b * b - a * c };

		if (discr > 0.0F)
		{
			const float rcpna{ -1.0F / a };
			const auto d{ std::sqrtf(discr) };

			s0 = (b + d) * rcpna;
			s1 = (b - d) * rcpna;

			if (s0 >= 0.0F && s0 <= 1.0F)
			{
				num_s = 1;
			}

			if (d > 0.0F && s1 >= 0.0F && s1 <= 1.0F)
			{
				if (num_s == 0)
				{
					s0 = s1;
				}

				++num_s;
			}
		}
	}
	else
	{
		// 2*b*s + c = 0
		// s = -c / (2*b)
		s0 = c / (-2.0F * b);

		if (s0 >= 0.0F && s0 <= 1.0F)
		{
			num_s = 1;
		}
	}

	if (num_s == 0)
	{
		return 0;
	}

	const float rcp_len2{ 1.0F / (ray[0] * ray[0] + ray[1] * ray[1]) };
	const float rayn_x{ ray[0] * rcp_len2 };
	const float rayn_y{ ray[1] * rcp_len2 };

	const float q0d{ q0[0] * rayn_x + q0[1] * rayn_y };
	const float q1d{ q1[0] * rayn_x + q1[1] * rayn_y };
	const float q2d{ q2[0] * rayn_x + q2[1] * rayn_y };
	const float rod{ orig[0] * rayn_x + orig[1] * rayn_y };

	const float q10d{ q1d - q0d }; //-V525
	const float q20d{ q2d - q0d };
	const float q0rd{ q0d - rod };

	hits[0][0] = q0rd + s0*(2.0F - 2.0F * s0) * q10d + s0 * s0 * q20d;
	hits[0][1] = a * s0 + b;

	if (num_s > 1)
	{
		hits[1][0] = q0rd + s1 * (2.0F - 2.0F * s1) * q10d + s1 * s1 * q20d;
		hits[1][1] = a * s1 + b;
		return 2;
	}

	return 1;
}

inline static std::int_fast32_t equal(const float* a, const float* b)
{
	return static_cast<std::int_fast32_t>(a[0] == b[0] && a[1] == b[1]); //-V550
}

inline static std::int_fast32_t stbtt__compute_crossings_x(const float x, float y, const std::int_fast32_t nverts, stbtt_vertex* verts)
{
	std::vector<float> orig{x, y};
	std::vector<float> ray{ 1.0F, 0.0F };
	std::int_fast32_t winding{};

	// make sure y never passes through a vertex of the shape
	const float y_frac{ std::fmodf(y, 1.0F) };

	if (y_frac < 0.01F)
	{
		y += 0.01F;
	}
	else if (y_frac > 0.99F)
	{
		y -= 0.01F;
	}

	// test a ray from (-infinity,y) to (x,y)
	for (std::int_fast32_t i{}; i < nverts; ++i)
	{
		if (verts[i].type == static_cast<unsigned char>(GlyphShapeType::STBTT_vline))
		{
			const std::int_fast32_t x0{ static_cast<std::int_fast32_t>(verts[i - 1].x) };
			const std::int_fast32_t y0{ static_cast<std::int_fast32_t>(verts[i - 1].y) };
			const std::int_fast32_t x1{ static_cast<std::int_fast32_t>(verts[i].x) };
			const std::int_fast32_t y1{ static_cast<std::int_fast32_t>(verts[i].y) };

			if (y > (std::min)(y0, y1) && y < (std::max)(y0, y1) && x > (std::min)(x0, x1))
			{
				const float x_inter{ (y - y0) / (y1 - y0) * (x1 - x0) + x0 };

				if (x_inter < x)
				{
					winding += (y0 < y1) ? 1 : -1;
				}
			}
		}

		if (verts[i].type == static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve))
		{
			std::int_fast32_t x0{ static_cast<std::int_fast32_t>(verts[i - 1].x) };
			std::int_fast32_t y0{ static_cast<std::int_fast32_t>(verts[i - 1].y) };
			std::int_fast32_t x1{ static_cast<std::int_fast32_t>(verts[i].cx) };
			std::int_fast32_t y1{ static_cast<std::int_fast32_t>(verts[i].cy) };
			std::int_fast32_t x2{ static_cast<std::int_fast32_t>(verts[i].x) };
			std::int_fast32_t y2{ static_cast<std::int_fast32_t>(verts[i].y) };
			const std::int_fast32_t ax{ (std::min)(x0, (std::min)(x1, x2)) };
			const std::int_fast32_t ay{ (std::min)(y0, (std::min)(y1, y2)) };
			const std::int_fast32_t by{ (std::max)(y0, (std::max)(y1, y2)) };

			if (y > ay && y < by && x > ax)
			{
				const float q0[2]{ static_cast<float>(x0), static_cast<float>(y0) };
				const float q1[2]{ static_cast<float>(x1), static_cast<float>(y1) };
				const float q2[2]{ static_cast<float>(x2), static_cast<float>(y2) };
				float hits[2][2]{};

				if (equal(q0, q1) != 0 || equal(q1, q2) != 0)
				{
					x0 = static_cast<std::int_fast32_t>(verts[i-1].x);
					y0 = static_cast<std::int_fast32_t>(verts[i-1].y);
					x1 = static_cast<std::int_fast32_t>(verts[i].x);
					y1 = static_cast<std::int_fast32_t>(verts[i].y);

					if (y > (std::min)(y0,y1) && y < (std::max)(y0,y1) && x >(std::min)(x0,x1))
					{
						const float x_inter{ (y - y0) / (y1 - y0) * (x1 - x0) + x0 };

						if (x_inter < x)
						{
							winding += (y0 < y1) ? 1 : -1;
						}
					}
				}
				else
				{
					const std::int_fast32_t num_hits{ stbtt__ray_intersect_bezier(orig.data(), ray.data(), q0, q1, q2, hits) };

					if (num_hits >= 1)
					{
						if (hits[0][0] < 0)
						{
							winding += (hits[0][1] < 0 ? -1 : 1);
						}
					}

					if (num_hits >= 2)
					{
						if (hits[1][0] < 0)
						{
							winding += (hits[1][1] < 0 ? -1 : 1);
						}
					}
				}
			}
		}
	}

	return winding;
}

// x^3 + c*x^2 + b*x + a = 0
inline static std::int_fast32_t stbtt__solve_cubic(const float a, const float b, const float c, float* r)
{
	const float s{ -a / 3.0F };
	const float p{ b - a * a / 3.0F };
	const float q{ a * (2.0F * a * a - 9.0F * b) / 27.0F + c };
	const float p3{ p * p * p };
	const float d{ q * q + 4.0F * p3 / 27.0F };

	if (d >= 0.0F)
	{
		const float z{ std::sqrtf(d) };
		float u{ (-q + z) / 2.0F };
		float v{ (-q - z) / 2.0F };

		u = std::cbrtf(u);
		v = std::cbrtf(v);
		r[0] = s + u + v;

		return 1;
	}

	const float u{ std::sqrtf(-p / 3.0F) };
	const float v{ std::acosf(-std::sqrtf(-27.0F / p3) * q / 2.0F) / 3.0F }; // p3 must be negative, since d is negative
	const float m{ std::cosf(v) };
	const float n{ std::cosf(v - 3.141592026F / 2.0F) * 1.732050808F };

	r[0] = s + u * 2.0F * m;
	r[1] = s - u * (m + n);
	r[2] = s - u * (m - n);

	return 3;
}

inline unsigned char* stbtt_GetGlyphSDF(const stbtt_fontinfo *info, float scale, const std::int_fast32_t glyph, const std::int_fast32_t padding, const unsigned char onedge_value, const float pixel_dist_scale, std::int_fast32_t *width, std::int_fast32_t *height, std::int_fast32_t *xoff, std::int_fast32_t *yoff)
{
	float scale_x{ scale };
	float scale_y{ scale };
	unsigned char* data{};

	// if one scale is 0, use same scale for both
	if (scale_x == 0.0F) //-V550
	{
		scale_x = scale_y;
	}

	if (scale_y == 0.0F) //-V550
	{
		if (scale_x == 0.0F) //-V550
		{
			return nullptr;  // if both scales are 0, return NULL
		}

		scale_y = scale_x;
	}

	std::int_fast32_t ix0{};
	std::int_fast32_t iy0{};
	std::int_fast32_t ix1{};
	std::int_fast32_t iy1{};

	stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale, scale, 0.0F, 0.0F, &ix0, &iy0, &ix1, &iy1);

	// if empty, return NULL
	if (ix0 == ix1 || iy0 == iy1)
	{
		return nullptr;
	}

	ix0 -= padding;
	iy0 -= padding;
	ix1 += padding;
	iy1 += padding;

	const std::int_fast32_t w{ (ix1 - ix0) };
	const std::int_fast32_t h{ (iy1 - iy0) };

	if (width != nullptr)
	{
		*width = w;
	}

	if (height != nullptr)
	{
		*height = h;
	}

	if (xoff != nullptr)
	{
		*xoff = ix0;
	}

	if (yoff != nullptr)
	{
		*yoff = iy0;
	}

	// invert for y-downwards bitmaps
	scale_y = -scale_y;

	{
		stbtt_vertex* verts{};
		const std::int_fast32_t num_verts{ stbtt_GetGlyphShape(info, glyph, &verts) };
		data = new unsigned char[w * h];
		float* precompute{ new float[num_verts * sizeof(float)] };

		for (std::int_fast32_t i{}, j = num_verts - 1; i < num_verts; j = i++)
		{
			if (verts[i].type == static_cast<unsigned char>(GlyphShapeType::STBTT_vline))
			{
				const float x0{ verts[i].x * scale_x };
				const float y0{ verts[i].y * scale_y };
				const float x1{ verts[j].x * scale_x };
				const float y1{ verts[j].y * scale_y };
				const float dist{ std::sqrtf((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) };
				precompute[i] = (dist == 0.0F) ? 0.0F : 1.0F / dist; //-V550
			}
			else if (verts[i].type == static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve))
			{
				const float x2{ verts[j].x * scale_x };
				const float y2{ verts[j].y * scale_y };
				const float x1{ verts[i].cx * scale_x };
				const float y1{ verts[i].cy * scale_y };
				const float x0{ verts[i].x * scale_x };
				const float y0{ verts[i].y * scale_y };
				const float bx{ x0 - 2.0F * x1 + x2 };
				const float by{ y0 - 2.0F * y1 + y2 };
				const float len2{ bx * bx + by * by };

				if (len2 != 0.0F) //-V550
				{
					precompute[i] = 1.0F / (bx * bx + by * by);
				}
				else
				{
					precompute[i] = 0.0F;
				}
			}
			else
			{
				precompute[i] = 0.0F;
			}
		}

		for (std::int_fast32_t y{ iy0 }; y < iy1; ++y)
		{
			for (std::int_fast32_t x{ ix0 }; x < ix1; ++x)
			{
				float min_dist{ 999999.0F };
				const float sx{ static_cast<float>(x) + 0.5F };
				const float sy{ static_cast<float>(y) + 0.5F };
				const float x_gspace{ (sx / scale_x) };
				const float y_gspace{ (sy / scale_y) };

				const std::int_fast32_t winding{ stbtt__compute_crossings_x(x_gspace, y_gspace, num_verts, verts) }; // @OPTIMIZE: this could just be a rasterization, but needs to be line vs. non-tesselated curves so a new path

				for (std::int_fast32_t i{}; i < num_verts; ++i)
				{
					const float x0{ verts[i].x * scale_x };
					const float y0{ verts[i].y * scale_y };

					// check against every point here rather than inside line/curve primitives -- @TODO: wrong if multiple 'moves' in a row produce a garbage point, and given culling, probably more efficient to do within line/curve
					float dist2{ (x0 - sx) * (x0 - sx) + (y0 - sy) * (y0 - sy) };

					if (dist2 < min_dist * min_dist)
					{
						min_dist = std::sqrtf(dist2);
					}

					if (verts[i].type == static_cast<unsigned char>(GlyphShapeType::STBTT_vline))
					{
						const float x1{ verts[i - 1].x * scale_x };
						const float y1{ verts[i - 1].y * scale_y };

						// coarse culling against bbox
						//if (sx > STBTT_min(x0,x1)-min_dist && sx < STBTT_max(x0,x1)+min_dist &&
						//    sy > STBTT_min(y0,y1)-min_dist && sy < STBTT_max(y0,y1)+min_dist)
						const float dist{ static_cast<float>(std::fabs((x1 - x0) * (y0 - sy) - (y1 - y0) * (x0 - sx))) * precompute[i] };

						if (dist < min_dist)
						{
							// check position along line
							// x' = x0 + t*(x1-x0), y' = y0 + t*(y1-y0)
							// minimize (x'-sx)*(x'-sx)+(y'-sy)*(y'-sy)
							const float dx{ x1 - x0 };
							const float dy{ y1 - y0 };
							const float px{ x0 - sx };
							const float py{ y0 - sy };
							// minimize (px+t*dx)^2 + (py+t*dy)^2 = px*px + 2*px*dx*t + t^2*dx*dx + py*py + 2*py*dy*t + t^2*dy*dy
							// derivative: 2*px*dx + 2*py*dy + (2*dx*dx+2*dy*dy)*t, set to 0 and solve
							const float t{ -(px * dx + py * dy) / (dx * dx + dy * dy) };

							if (t >= 0.0F && t <= 1.0F)
							{
								min_dist = dist;
							}
						}
					}
					else if (verts[i].type == static_cast<unsigned char>(GlyphShapeType::STBTT_vcurve))
					{
						const float x2{ verts[i - 1].x * scale_x };
						const float y2{ verts[i - 1].y * scale_y };
						const float x1{ verts[i].cx * scale_x };
						const float y1{ verts[i].cy * scale_y };
						const float box_x0{ (std::min)((std::min)(x0, x1), x2) };
						const float box_y0{ (std::min)((std::min)(y0, y1), y2) };
						const float box_x1{ (std::max)((std::max)(x0, x1), x2) };
						const float box_y1{ (std::max)((std::max)(y0, y1), y2) };

						// coarse culling against bbox to avoid computing cubic unnecessarily
						if (sx > box_x0 - min_dist && sx < box_x1 + min_dist && sy > box_y0 - min_dist && sy < box_y1 + min_dist)
						{
							std::int_fast32_t num{};
							const float ax{ x1 - x0 };
							const float ay{ y1 - y0 };
							const float bx{ x0 - 2.0F * x1 + x2 };
							const float by{ y0 - 2.0F * y1 + y2 };
							const float mx{ x0 - sx };
							const float my{ y0 - sy };
							std::vector<float> res(3);
							const float a_inv{ precompute[i] };

							if (a_inv == 0.0F) // if a_inv is 0, it's 2nd degree so use quadratic formula //-V550
							{
								const float a{ 3.0F * (ax * bx + ay * by) };
								const float b{ 2.0F * (ax * ax + ay * ay) + (mx * bx + my * by) };
								const float c{ mx * ax + my * ay };

								if (a == 0.0F) // if a is 0, it's linear //-V550
								{
									if (b != 0.0F) //-V550
									{
										res[num++] = -c/b;
									}
								}
								else
								{
									const float discriminant{ b * b - 4.0F * a * c };

									if (discriminant < 0.0F)
									{
										num = 0;
									}
									else
									{
										const float root{ std::sqrtf(discriminant) };
										res[0] = (-b - root) / (2.0F * a);
										res[1] = (-b + root) / (2.0F * a);
										num = 2; // don't bother distinguishing 1-solution case, as code below will still work
									}
								}
							}
							else
							{
								const float b{ 3.0F * (ax * bx + ay * by) * a_inv }; // could precompute this as it doesn't depend on sample point
								const float c{ (2.0F * (ax * ax + ay * ay) + (mx * bx + my * by)) * a_inv };
								const float d{ (mx * ax + my * ay) * a_inv };
								num = stbtt__solve_cubic(b, c, d, res.data());
							}

							if (num >= 1 && res[0] >= 0.0F && res[0] <= 1.0F)
							{
								const float t{ res[0] };
								const float it{ 1.0F - t };
								const float px{ it * it * x0 + 2.0F * t * it * x1 + t * t * x2 };
								const float py{ it * it * y0 + 2.0F * t * it * y1 + t * t * y2 };
								dist2 = (px - sx) * (px - sx) + (py - sy) * (py - sy);

								if (dist2 < min_dist * min_dist)
								{
									min_dist = std::sqrtf(dist2);
								}
							}

							if (num >= 2 && res[1] >= 0.0F && res[1] <= 1.0F)
							{
								const float t{ res[1] };
								const float it{ 1.0F - t };
								const float px{ it * it * x0 + 2.0F * t * it * x1 + t * t * x2 };
								const float py{ it * it * y0 + 2.0F * t * it * y1 + t * t * y2 };
								dist2 = (px - sx) * (px - sx) + (py - sy) * (py - sy);

								if (dist2 < min_dist * min_dist)
								{
									min_dist = std::sqrtf(dist2);
								}
							}

							if (num >= 3 && res[2] >= 0.0f && res[2] <= 1.0f)
							{
								const float t{ res[2] };
								const float it{ 1.0F - t };
								const float px{ it * it * x0 + 2.0F * t * it * x1 + t * t * x2 };
								const float py{ it * it * y0 + 2.0F * t * it * y1 + t * t * y2 };
								dist2 = (px - sx) * (px - sx) + (py - sy) * (py - sy);

								if (dist2 < min_dist * min_dist)
								{
									min_dist = std::sqrtf(dist2);
								}
							}
						}
					}
				}

				if (winding == 0)
				{
					min_dist = -min_dist;  // if outside the shape, value is negative
				}

				data[(y - iy0) * w + (x - ix0)] = static_cast<unsigned char>(std::clamp((onedge_value + pixel_dist_scale * min_dist), 0.0F, 255.0F));
			}
		}

		delete precompute;
		static_cast<void>(info->userdata), free(verts);
	}

	return data;
}

inline unsigned char* stbtt_GetCodepointSDF(const stbtt_fontinfo* info, const float scale, const std::int_fast32_t codepoint, const std::int_fast32_t padding, const unsigned char onedge_value, const float pixel_dist_scale, std::int_fast32_t* width, std::int_fast32_t* height, std::int_fast32_t* xoff, std::int_fast32_t* yoff)
{
	return stbtt_GetGlyphSDF(info, scale, stbtt_FindGlyphIndex(info, codepoint), padding, onedge_value, pixel_dist_scale, width, height, xoff, yoff);
}

#if defined(__GNUC__) || defined(__clang__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

inline std::int_fast32_t stbtt_BakeFontBitmap(const unsigned char* data, const std::int_fast32_t offset, const float pixel_height, unsigned char* pixels, const std::int_fast32_t pw, const std::int_fast32_t ph, const std::int_fast32_t first_char, const std::int_fast32_t num_chars, stbtt_bakedchar* chardata)
{
	return stbtt_BakeFontBitmap_internal(const_cast<unsigned char*>(data), offset, pixel_height, pixels, pw, ph, first_char, num_chars, chardata);
}

inline std::int_fast32_t stbtt_GetFontOffsetForIndex(const unsigned char* data, const std::int_fast32_t index)
{
	return stbtt_GetFontOffsetForIndex_internal(const_cast<unsigned char*>(data), index);
}

inline std::int_fast32_t stbtt_GetNumberOfFonts(const unsigned char* data)
{
	return stbtt_GetNumberOfFonts_internal(const_cast<unsigned char*>(data));
}

inline std::int_fast32_t stbtt_InitFont(stbtt_fontinfo* info, const unsigned char* data, const std::int_fast32_t offset)
{
	return stbtt_InitFont_internal(info, const_cast<unsigned char*>(data), offset);
}

#if defined(__GNUC__) || defined(__clang__)
	#pragma GCC diagnostic pop
#endif

#endif // STB_TRUETYPE_IMPLEMENTATION

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
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