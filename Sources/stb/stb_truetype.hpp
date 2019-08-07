// stb_truetype.hpp - under development - public domain
// authored in 2019 by Stefan Kubsch
// conversion to C++
//
// Notes:
//		- removed Rasterizer version 1
//
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
//   Todo:
//        non-MS cmaps
//        crashproof on bad data
//        hinting? (no longer patented)
//        cleartype-style AA?
//        optimize: use simple memory allocator for intermediates
//        optimize: build edge-list directly from curves
//        optimize: rasterize directly from curves?
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
// VERSION HISTORY
//
//   1.21 (2019-02-25) fix warning
//   1.20 (2019-02-07) PackFontRange skips missing codepoints; GetScaleFontVMetrics()
//   1.19 (2018-02-11) GPOS kerning, STBTT_fmod
//   1.18 (2018-01-29) add missing function
//   1.17 (2017-07-23) make more arguments const; doc fix
//   1.16 (2017-07-12) SDF support
//   1.15 (2017-03-03) make more arguments const
//   1.14 (2017-01-16) num-fonts-in-TTC function
//   1.13 (2017-01-02) support OpenType fonts, certain Apple fonts
//   1.12 (2016-10-25) suppress warnings about casting away const with -Wcast-qual
//   1.11 (2016-04-02) fix unused-variable warning
//   1.10 (2016-04-02) user-defined fabs(); rare memory leak; remove duplicate typedef
//   1.09 (2016-01-16) warning fix; avoid crash on outofmem; use allocation userdata properly
//   1.08 (2015-09-13) document stbtt_Rasterize(); fixes for vertical & horizontal edges
//   1.07 (2015-08-01) allow PackFontRanges to accept arrays of sparse codepoints;
//                     variant PackFontRanges to pack and render in separate phases;
//                     fix stbtt_GetFontOFfsetForIndex (never worked for non-0 input?);
//                     fixed an assert() bug in the new rasterizer
//                     replace assert() with STBTT_assert() in new rasterizer
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
//   To make the implementation private to the file that generates the implementation,
//      #define STBTT_STATIC
//
//   Simple 3D API (don't ship this, but it's fine for tools and quick start)
//           stbtt_BakeFontBitmap()               -- bake a font to a bitmap for use as texture
//           stbtt_GetBakedQuad()                 -- compute quad to draw for a given char
//
//   Improved 3D API (more shippable):
//           #include "stb_rect_pack.h"           -- optional, but you really want it
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
//   given file is in a general way. I provide an API for this, but I don't
//   recommend it.
//
//
// PERFORMANCE MEASUREMENTS FOR 1.06:
//
//                      32-bit     64-bit
//   Previous release:  8.83 s     7.68 s
//   Pool allocations:  7.72 s     6.34 s
//   Inline sort     :  6.54 s     5.65 s
//   New rasterizer  :  5.63 s     5.00 s

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////
////   INTEGRATION WITH YOUR CODEBASE
////
////   The following sections allow you to supply alternate definitions
////   of C library functions used by stb_truetype, e.g. if you don't
////   link with the C runtime library.

#pragma once

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstring>
#include <vector>

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
	int cursor{};
	int size{};
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
	unsigned short x0{};
	unsigned short y0{};
	unsigned short x1{};
	unsigned short y1{};
	float xoff{};
	float yoff{};
	float xadvance{};
};

int stbtt_BakeFontBitmap(const unsigned char* data, int offset,			// font location (use offset=0 for plain .ttf)
                                float pixel_height,                     // height of font in pixels
                                unsigned char* pixels, int pw, int ph,  // bitmap to be filled in
                                int first_char, int num_chars,          // characters to bake
                                stbtt_bakedchar* chardata);             // you allocate this, it's num_chars long
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

void stbtt_GetBakedQuad(const stbtt_bakedchar* chardata, int pw, int ph,  // same data as above
                               int char_index,             // character to display
                               float* xpos, const float* ypos,   // pointers to current position in screen pixel space
                               stbtt_aligned_quad* q,      // output: quad to draw
                               int opengl_fillrule);       // true if opengl fill rule; false if DX9 or earlier
// Call GetBakedQuad with char_index = 'character - first_char', and it
// creates the quad you need to draw and advances the current position.
//
// The coordinate system used assumes y increases downwards.
//
// Characters will extend both above and below the current position;
// see discussion of "BASELINE" above.
//
// It's inefficient; you might want to c&p it and optimize it.

void stbtt_GetScaledFontVMetrics(const unsigned char* fontdata, int index, float size, float* ascent, float* descent, float* lineGap);
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
	unsigned short x0{};
	unsigned short y0{};
	unsigned short x1{};
	unsigned short y1{};
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

int  stbtt_PackBegin(stbtt_pack_context* spc, unsigned char* pixels, int pw, int ph, int stride_in_bytes, int padding, void* alloc_context);
// Initializes a packing context stored in the passed-in stbtt_pack_context.
// Future calls using this context will pack characters into the bitmap passed
// in here: a 1-channel bitmap that is width * height. stride_in_bytes is
// the distance from one row to the next (or 0 to mean they are packed tightly
// together). "padding" is the amount of padding to leave between each
// character (normally you want '1' for bitmaps you'll use as textures with
// bilinear filtering).
//
// Returns 0 on failure, 1 on success.

void stbtt_PackEnd  (stbtt_pack_context* spc);
// Cleans up the packing context and frees all memory.

#define STBTT_POINT_SIZE(x)   (-(x))

int stbtt_PackFontRange(stbtt_pack_context* spc, const unsigned char* fontdata, int font_index, float font_size, int first_unicode_codepoint_in_range, int num_chars_in_range, stbtt_packedchar* chardata_for_range);
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
	int first_unicode_codepoint_in_range{};  // if non-zero, then the chars are continuous, and this is the first codepoint
	int* array_of_unicode_codepoints{};       // if non-zero, then this is an array of unicode codepoints
	int num_chars{};
	stbtt_packedchar* chardata_for_range{}; // output
	// don't set these, they're used internally
	unsigned char h_oversample{};
	unsigned char v_oversample{};
};

int stbtt_PackFontRanges(stbtt_pack_context* spc, const unsigned char* fontdata, int font_index, stbtt_pack_range* ranges, int num_ranges);
// Creates character bitmaps from multiple ranges of characters stored in
// ranges. This will usually create a better-packed bitmap than multiple
// calls to stbtt_PackFontRange. Note that you can call this multiple
// times within a single PackBegin/PackEnd.

void stbtt_PackSetOversampling(stbtt_pack_context* spc, unsigned int h_oversample, unsigned int v_oversample);
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

void stbtt_PackSetSkipMissingCodepoints(stbtt_pack_context* spc, int skip);
// If skip != 0, this tells stb_truetype to skip any codepoints for which
// there is no corresponding glyph. If skip=0, which is the default, then
// codepoints without a glyph recived the font's "missing character" glyph,
// typically an empty box by convention.

void stbtt_GetPackedQuad(const stbtt_packedchar* chardata, int pw, int ph,  // same data as above
                               int char_index,								// character to display
                               float* xpos, const float* ypos,				// pointers to current position in screen pixel space
                               stbtt_aligned_quad* q,						// output: quad to draw
                               int align_to_integer);

int stbtt_PackFontRangesGatherRects(stbtt_pack_context* spc, const stbtt_fontinfo* info, stbtt_pack_range* ranges, int num_ranges, stbrp_rect* rects);
void stbtt_PackFontRangesPackRects(stbtt_pack_context* spc, stbrp_rect* rects, int num_rects);
int stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context* spc, const stbtt_fontinfo* info, stbtt_pack_range* ranges, int num_ranges, stbrp_rect* rects);
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
	int width{};
	int height{};
	int stride_in_bytes{};
	int padding{};
	int skip_missing{};
	unsigned int h_oversample{};
	unsigned int v_oversample{};
	unsigned char* pixels{};
	void* nodes{};
};

//////////////////////////////////////////////////////////////////////////////
//
// FONT LOADING
//
//

int stbtt_GetNumberOfFonts(const unsigned char* data);
// This function will determine the number of fonts in a font file.  TrueType
// collection (.ttc) files may contain multiple fonts, while TrueType font
// (.ttf) files only contain one font. The number of fonts can be used for
// indexing with the previous function where the index is between zero and one
// less than the total fonts. If an error occurs, -1 is returned.

int stbtt_GetFontOffsetForIndex(const unsigned char* data, int index);
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
	unsigned char* data{};				// pointer to .ttf file
	int fontstart{};					// offset of start of font
	int numGlyphs{};                    // number of glyphs, needed for range checking
	// table locations as offset from start of .ttf
	int loca{};
	int head{};
	int glyf{};
	int hhea{};
	int hmtx{};
	int kern{};
	int gpos{};
	int index_map{};                   // a cmap mapping for our chosen character encoding
	int indexToLocFormat{};            // format needed to map from glyph index to glyph

	stbtt__buf cff;                    // cff font data
	stbtt__buf charstrings;            // the charstring index
	stbtt__buf gsubrs;                 // global charstring subroutines index
	stbtt__buf subrs;                  // private charstring subroutines index
	stbtt__buf fontdicts;              // array of font dicts
	stbtt__buf fdselect;               // map from glyph to fontdict
};

int stbtt_InitFont(stbtt_fontinfo* info, const unsigned char* data, int offset);
// Given an offset into the file that defines a font, this function builds
// the necessary cached info for the rest of the system. You must allocate
// the stbtt_fontinfo yourself, and stbtt_InitFont will fill it out. You don't
// need to do anything special to free it, because the contents are pure
// value data with no additional data structures. Returns 0 on failure.


//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER TO GLYPH-INDEX CONVERSIOn

int stbtt_FindGlyphIndex(const stbtt_fontinfo* info, int unicode_codepoint);
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

void stbtt_GetFontVMetrics(const stbtt_fontinfo* info, int* ascent, int* descent, int* lineGap);
// ascent is the coordinate above the baseline the font extends; descent
// is the coordinate below the baseline the font extends (i.e. it is typically negative)
// lineGap is the spacing between one row's descent and the next row's ascent...
// so you should advance the vertical position by "*ascent - *descent + *lineGap"
//   these are expressed in unscaled coordinates, so you must multiply by
//   the scale factor for a given size

int  stbtt_GetFontVMetricsOS2(const stbtt_fontinfo* info, int* typoAscent, int* typoDescent, int* typoLineGap);
// analogous to GetFontVMetrics, but returns the "typographic" values from the OS/2
// table (specific to MS/Windows TTF files).
//
// Returns 1 on success (table present), 0 on failure.

void stbtt_GetFontBoundingBox(const stbtt_fontinfo* info, int* x0, int* y0, int* x1, int* y1);
// the bounding box around all possible characters

void stbtt_GetCodepointHMetrics(const stbtt_fontinfo* info, int codepoint, int* advanceWidth, int* leftSideBearing);
// leftSideBearing is the offset from the current horizontal position to the left edge of the character
// advanceWidth is the offset from the current horizontal position to the next horizontal position
//   these are expressed in unscaled coordinates

int stbtt_GetCodepointKernAdvance(const stbtt_fontinfo* info, int ch1, int ch2);
// an additional amount to add to the 'advance' value between ch1 and ch2

int stbtt_GetCodepointBox(const stbtt_fontinfo* info, int codepoint, int* x0, int* y0, int* x1, int* y1);
// Gets the bounding box of the visible part of the glyph, in unscaled coordinates

void stbtt_GetGlyphHMetrics(const stbtt_fontinfo* info, int glyph_index, int* advanceWidth, int* leftSideBearing);
int stbtt_GetGlyphKernAdvance(const stbtt_fontinfo* info, int glyph1, int glyph2);
int stbtt_GetGlyphBox(const stbtt_fontinfo* info, int glyph_index, int* x0, int* y0, int* x1, int* y1);
// as above, but takes one or more glyph indices for greater efficiency

//////////////////////////////////////////////////////////////////////////////
//
// GLYPH SHAPES (you probably don't need these, but they have to go before
// the bitmaps for C declaration-order reasons)
//

enum
{
	STBTT_vmove = 1,
	STBTT_vline,
	STBTT_vcurve,
	STBTT_vcubic
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
	unsigned charpadding{};
};

int stbtt_IsGlyphEmpty(const stbtt_fontinfo* info, int glyph_index);
// returns non-zero if nothing is drawn for this glyph

int stbtt_GetCodepointShape(const stbtt_fontinfo* info, int unicode_codepoint, stbtt_vertex** vertices);
int stbtt_GetGlyphShape(const stbtt_fontinfo* info, int glyph_index, stbtt_vertex** vertices);
// returns # of vertices and fills *vertices with the pointer to them
//   these are expressed in "unscaled" coordinates
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

void stbtt_FreeBitmap(unsigned char* bitmap, void* userdata);
// frees the bitmap allocated below

unsigned char* stbtt_GetCodepointBitmap(const stbtt_fontinfo* info, float scale_x, float scale_y, int codepoint, int* width, int* height, int* xoff, int* yoff);
// allocates a large-enough single-channel 8bpp bitmap and renders the
// specified character/glyph at the specified scale into it, with
// antialiasing. 0 is no coverage (transparent), 255 is fully covered (opaque).
// *width & *height are filled out with the width & height of the bitmap,
// which is stored left-to-right, top-to-bottom.
//
// xoff/yoff are the offset it pixel space from the glyph origin to the top-left of the bitmap

unsigned char* stbtt_GetCodepointBitmapSubpixel(const stbtt_fontinfo* info, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint, int* width, int* height, int* xoff, int* yoff);
// the same as stbtt_GetCodepoitnBitmap, but you can specify a subpixel
// shift for the character

void stbtt_MakeCodepointBitmap(const stbtt_fontinfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int codepoint);
// the same as stbtt_GetCodepointBitmap, but you pass in storage for the bitmap
// in the form of 'output', with row spacing of 'out_stride' bytes. the bitmap
// is clipped to out_w/out_h bytes. Call stbtt_GetCodepointBitmapBox to get the
// width and height and positioning info for it first.

void stbtt_MakeCodepointBitmapSubpixel(const stbtt_fontinfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint);
// same as stbtt_MakeCodepointBitmap, but you can specify a subpixel
// shift for the character

void stbtt_MakeCodepointBitmapSubpixelPrefilter(const stbtt_fontinfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int prefilter_x, int prefilter_y, float *sub_x, float *sub_y, int codepoint);
// same as stbtt_MakeCodepointBitmapSubpixel, but prefiltering
// is performed (see stbtt_PackSetOversampling)

void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo* font, int codepoint, float scale_x, float scale_y, int* ix0, int* iy0, int* ix1, int* iy1);
// get the bbox of the bitmap centered around the glyph origin; so the
// bitmap width is ix1-ix0, height is iy1-iy0, and location to place
// the bitmap top left is (leftSideBearing*scale,iy0).
// (Note that the bitmap uses y-increases-down, but the shape uses
// y-increases-up, so CodepointBitmapBox and CodepointBox are inverted.)

void stbtt_GetCodepointBitmapBoxSubpixel(const stbtt_fontinfo* font, int codepoint, float scale_x, float scale_y, float shift_x, float shift_y, int* ix0, int* iy0, int* ix1, int* iy1);
// same as stbtt_GetCodepointBitmapBox, but you can specify a subpixel
// shift for the character

// the following functions are equivalent to the above functions, but operate
// on glyph indices instead of Unicode codepoints (for efficiency)
unsigned char* stbtt_GetGlyphBitmap(const stbtt_fontinfo* info, float scale_x, float scale_y, int glyph, int* width, int* height, int* xoff, int* yoff);
unsigned char* stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo* info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int* width, int* height, int* xoff, int* yoff);
void stbtt_MakeGlyphBitmap(const stbtt_fontinfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph);
void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph);
void stbtt_MakeGlyphBitmapSubpixelPrefilter(const stbtt_fontinfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int prefilter_x, int prefilter_y, float* sub_x, float* sub_y, int glyph);
void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo* font, int glyph, float scale_x, float scale_y, int* ix0, int* iy0, int* ix1, int* iy1);
void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y,float shift_x, float shift_y, int* ix0, int* iy0, int* ix1, int* iy1);


// @TODO: don't expose this structure
using stbtt__bitmap = struct
{
	int w{};
	int h{};
	int stride{};
	unsigned char* pixels{};
};

// rasterize a shape with quadratic beziers into a bitmap
void stbtt_Rasterize(stbtt__bitmap* result,        // 1-channel bitmap to draw into
                               float flatness_in_pixels,     // allowable error of curve in pixels
                               stbtt_vertex* vertices,       // array of vertices defining shape
                               int num_verts,                // number of vertices in above array
                               float scale_x, float scale_y, // scale applied to input vertices
                               float shift_x, float shift_y, // translation applied to input vertices
                               int x_off, int y_off,         // another translation applied to input
                               int invert,                   // if non-zero, vertically flip shape
                               void* userdata);              // context for to STBTT_MALLOC

//////////////////////////////////////////////////////////////////////////////
//
// Signed Distance Function (or Field) rendering

void stbtt_FreeSDF(unsigned char* bitmap, void* userdata);
// frees the SDF bitmap allocated below

unsigned char* stbtt_GetGlyphSDF(const stbtt_fontinfo* info, float scale, int glyph, int padding, unsigned char onedge_value, float pixel_dist_scale, int* width, int* height, int* xoff, int* yoff);
unsigned char* stbtt_GetCodepointSDF(const stbtt_fontinfo* info, float scale, int codepoint, int padding, unsigned char onedge_value, float pixel_dist_scale, int* width, int* height, int* xoff, int* yoff);
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

//////////////////////////////////////////////////////////////////////////////
//
// Finding the right font...
//
// You should really just solve this offline, keep your own tables
// of what font is what, and don't try to get it out of the .ttf file.
// That's because getting it out of the .ttf file is really hard, because
// the names in the file can appear in many possible encodings, in many
// possible languages, and e.g. if you need a case-insensitive comparison,
// the details of that depend on the encoding & language in a complex way
// (actually underspecified in truetype, but also gigantic).
//
// But you can use the provided functions in two possible ways:
//     stbtt_FindMatchingFont() will use *case-sensitive* comparisons on
//             unicode-encoded names to try to find the font you want;
//             you can run this before calling stbtt_InitFont()
//
//     stbtt_GetFontNameString() lets you get any of the various strings
//             from the file yourself and do your own comparisons on them.
//             You have to have called stbtt_InitFont() first.

int stbtt_FindMatchingFont(const unsigned char* fontdata, const char* name, int flags);
// returns the offset (not index) of the font that matches, or -1 if none
//   if you use STBTT_MACSTYLE_DONTCARE, use a font name like "Arial Bold".
//   if you use any other flag, use a font name like "Arial"; this checks
//     the 'macStyle' header field; i don't know if fonts set this consistently
#define STBTT_MACSTYLE_DONTCARE     0
#define STBTT_MACSTYLE_BOLD         1
#define STBTT_MACSTYLE_ITALIC       2
#define STBTT_MACSTYLE_UNDERSCORE   4
#define STBTT_MACSTYLE_NONE         8   // <= not same as 0, this makes us check the bitfield is 0

int stbtt_CompareUTF8toUTF16_bigendian(const char* s1, int len1, const char* s2, int len2);
// returns 1/0 whether the first string interpreted as utf8 is identical to
// the second string interpreted as big-endian utf16... useful for strings from next func

const char* stbtt_GetFontNameString(const stbtt_fontinfo* font, int* length, int platformID, int encodingID, int languageID, int nameID);
// returns the string (which may be big-endian double byte, e.g. for unicode)
// and puts the length in bytes in *length.
//
// some of the values for the IDs are below; for more see the truetype spec:
//     http://developer.apple.com/textfonts/TTRefMan/RM06/Chap6name.html
//     http://www.microsoft.com/typography/otspec/name.htm

enum { // platformID
	STBTT_PLATFORM_ID_UNICODE   = 0,
	STBTT_PLATFORM_ID_MAC       = 1,
	STBTT_PLATFORM_ID_ISO       = 2,
	STBTT_PLATFORM_ID_MICROSOFT = 3
};

enum { // encodingID for STBTT_PLATFORM_ID_UNICODE
	STBTT_UNICODE_EID_UNICODE_1_0		= 0,
	STBTT_UNICODE_EID_UNICODE_1_1		= 1,
	STBTT_UNICODE_EID_ISO_10646			= 2,
	STBTT_UNICODE_EID_UNICODE_2_0_BMP	= 3,
	STBTT_UNICODE_EID_UNICODE_2_0_FULL	= 4
};

enum { // encodingID for STBTT_PLATFORM_ID_MICROSOFT
	STBTT_MS_EID_SYMBOL        = 0,
	STBTT_MS_EID_UNICODE_BMP   = 1,
	STBTT_MS_EID_SHIFTJIS      = 2,
	STBTT_MS_EID_UNICODE_FULL  = 10
};

enum { // encodingID for STBTT_PLATFORM_ID_MAC; same as Script Manager codes
	STBTT_MAC_EID_ROMAN        = 0,   STBTT_MAC_EID_ARABIC       = 4,
	STBTT_MAC_EID_JAPANESE     = 1,   STBTT_MAC_EID_HEBREW       = 5,
	STBTT_MAC_EID_CHINESE_TRAD = 2,   STBTT_MAC_EID_GREEK        = 6,
	STBTT_MAC_EID_KOREAN       = 3,   STBTT_MAC_EID_RUSSIAN      = 7
};

enum { // languageID for STBTT_PLATFORM_ID_MICROSOFT; same as LCID...
       // problematic because there are e.g. 16 english LCIDs and 16 arabic LCIDs
	STBTT_MS_LANG_ENGLISH     = 0x0409,   STBTT_MS_LANG_ITALIAN     = 0x0410,
	STBTT_MS_LANG_CHINESE     = 0x0804,   STBTT_MS_LANG_JAPANESE    = 0x0411,
	STBTT_MS_LANG_DUTCH       = 0x0413,   STBTT_MS_LANG_KOREAN      = 0x0412,
	STBTT_MS_LANG_FRENCH      = 0x040c,   STBTT_MS_LANG_RUSSIAN     = 0x0419,
	STBTT_MS_LANG_GERMAN      = 0x0407,   STBTT_MS_LANG_SPANISH     = 0x0409,
	STBTT_MS_LANG_HEBREW      = 0x040d,   STBTT_MS_LANG_SWEDISH     = 0x041D
};

enum { // languageID for STBTT_PLATFORM_ID_MAC
	STBTT_MAC_LANG_ENGLISH      = 0,	STBTT_MAC_LANG_JAPANESE				= 11,
	STBTT_MAC_LANG_ARABIC       = 12,	STBTT_MAC_LANG_KOREAN				= 23,
	STBTT_MAC_LANG_DUTCH        = 4,	STBTT_MAC_LANG_RUSSIAN				= 32,
	STBTT_MAC_LANG_FRENCH       = 1,	STBTT_MAC_LANG_SPANISH				= 6,
	STBTT_MAC_LANG_GERMAN       = 2,	STBTT_MAC_LANG_SWEDISH				= 5,
	STBTT_MAC_LANG_HEBREW       = 10,	STBTT_MAC_LANG_CHINESE_SIMPLIFIED	= 33,
	STBTT_MAC_LANG_ITALIAN      = 3,	STBTT_MAC_LANG_CHINESE_TRAD			= 19
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////   IMPLEMENTATION
////
////

#ifdef STB_TRUETYPE_IMPLEMENTATION
#define STBTT_MAX_OVERSAMPLE   8
using stbtt__test_oversample_pow2 = int[(STBTT_MAX_OVERSAMPLE & (STBTT_MAX_OVERSAMPLE - 1)) == 0 ? 1 : -1];

#ifdef _MSC_VER
	#define STBTT__NOTUSED(v)  (void)(v)
#else
	#define STBTT__NOTUSED(v)  (void)sizeof(v)
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

inline static void stbtt__buf_seek(stbtt__buf* b, int o)
{
	assert(!(o > b->size || o < 0));
	b->cursor = (o > b->size || o < 0) ? b->size : o;
}

inline static void stbtt__buf_skip(stbtt__buf* b, int o)
{
	stbtt__buf_seek(b, b->cursor + o);
}

inline static unsigned int stbtt__buf_get(stbtt__buf* b, int n)
{
	unsigned int v{};

	assert(n >= 1 && n <= 4);

	for (int i{}; i < n; i++)
	{
		v = (v << 8) | stbtt__buf_get8(b);
	}

	return v;
}

inline static stbtt__buf stbtt__new_buf(void* p, size_t size)
{
	stbtt__buf r{};
	assert(size < 0x40000000);
	r.data = static_cast<unsigned char*>(p);
	r.size = static_cast<int>(size);
	r.cursor = 0;
	return r;
}

#define stbtt__buf_get16(b)  stbtt__buf_get((b), 2)
#define stbtt__buf_get32(b)  stbtt__buf_get((b), 4)

inline static stbtt__buf stbtt__buf_range(const stbtt__buf* b, int o, int s)
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
	const int start{ b->cursor };
	const int count{ static_cast<int>(stbtt__buf_get16(b)) };

	if (count != 0)
	{
		const int offsize{ stbtt__buf_get8(b) };
		assert(offsize >= 1 && offsize <= 4);
		stbtt__buf_skip(b, offsize * count);
		stbtt__buf_skip(b, stbtt__buf_get(b, offsize) - 1);
	}

	return stbtt__buf_range(b, start, b->cursor - start);
}

inline static unsigned int stbtt__cff_int(stbtt__buf* b)
{
	const int b0{ stbtt__buf_get8(b) };

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
		return stbtt__buf_get16(b);
	}

	if (b0 == 29)
	{
	   return stbtt__buf_get32(b);
	}

	assert(0);

	return 0;
}

inline static void stbtt__cff_skip_operand(stbtt__buf* b)
{
	const int b0{ stbtt__buf_peek8(b) };
	assert(b0 >= 28);

	if (b0 == 30)
	{
		stbtt__buf_skip(b, 1);

		while (b->cursor < b->size)
		{
			const int v{ stbtt__buf_get8(b) };

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

inline static stbtt__buf stbtt__dict_get(stbtt__buf* b, int key)
{
	stbtt__buf_seek(b, 0);

	while (b->cursor < b->size)
	{
		const int start{ b->cursor };

		while (stbtt__buf_peek8(b) >= 28)
		{
			stbtt__cff_skip_operand(b);
		}

		const int end{ b->cursor };
		int op{ stbtt__buf_get8(b) };

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

inline static void stbtt__dict_get_ints(stbtt__buf* b, int key, int outcount, unsigned int* out)
{
	stbtt__buf operands{ stbtt__dict_get(b, key) };

	for (int i{}; i < outcount && operands.cursor < operands.size; i++)
	{
		out[i] = stbtt__cff_int(&operands);
	}
}

inline static int stbtt__cff_index_count(stbtt__buf* b)
{
	stbtt__buf_seek(b, 0);

	return stbtt__buf_get16(b);
}

inline static stbtt__buf stbtt__cff_index_get(stbtt__buf b, int i)
{
	stbtt__buf_seek(&b, 0);

	const int count{ static_cast<int>(stbtt__buf_get16(&b)) };
	const int offsize{ stbtt__buf_get8(&b) };

	assert(i >= 0 && i < count);
	assert(offsize >= 1 && offsize <= 4);
	stbtt__buf_skip(&b, i * offsize);

	const int start{ static_cast<int>(stbtt__buf_get(&b, offsize)) };

	return stbtt__buf_range(&b, 2 + (count + 1) * offsize + start, stbtt__buf_get(&b, offsize) - start);
}

//////////////////////////////////////////////////////////////////////////
//
// accessors to parse data from file
//

// on platforms that don't allow misaligned reads, if we want to allow
// truetype fonts that aren't padded to alignment, define ALLOW_UNALIGNED_TRUETYPE

#define ttBYTE(p)     (*static_cast<unsigned char*>(p))
#define ttCHAR(p)     (*reinterpret_cast<signed char*>(p))
#define ttFixed(p)    ttLONG(p)

static unsigned short ttUSHORT(unsigned char* p)	{ return p[0] * 256 + p[1]; }
static signed short ttSHORT(unsigned char* p)		{ return p[0] * 256 + p[1]; }
static unsigned int ttULONG(unsigned char* p)		{ return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; }
static signed int ttLONG(unsigned char* p)			{ return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; }

#define stbtt_tag4(p, c0, c1, c2, c3) ((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define stbtt_tag(p, str)	stbtt_tag4(p, str[0], str[1], str[2], str[3])

inline static int stbtt__isfont(const unsigned char* font)
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
inline static unsigned int stbtt__find_table(unsigned char* data, unsigned int fontstart, const char* tag)
{
	const signed int num_tables{ ttUSHORT(data + fontstart + 4) };
	const unsigned int tabledir{ fontstart + 12 };

	for (signed int i{}; i < num_tables; ++i)
	{
		const unsigned int loc{ tabledir + 16 * i };

		if (stbtt_tag(data + loc + 0, tag) != 0)
		{
			return ttULONG(data + loc + 8);
		}
	}

	return 0;
}

inline static int stbtt_GetFontOffsetForIndex_internal(unsigned char* font_collection, int index)
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

inline static int stbtt_GetNumberOfFonts_internal(unsigned char* font_collection)
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
		if (ttULONG(font_collection+4) == 0x00010000 || ttULONG(font_collection+4) == 0x00020000)
		{
			return ttLONG(font_collection+8);
		}
	}

	return 0;
}

inline static stbtt__buf stbtt__get_subrs(stbtt__buf cff, stbtt__buf fontdict)
{
	std::vector<unsigned int> private_loc(2);

	stbtt__dict_get_ints(&fontdict, 18, 2, private_loc.data());

	if (private_loc[1] == 0 || private_loc[0] == 0)
	{
		return stbtt__new_buf(nullptr, 0);
	}

	stbtt__buf pdict{ stbtt__buf_range(&cff, private_loc[1], private_loc[0]) };

	unsigned int subrsoff{};

	stbtt__dict_get_ints(&pdict, 19, 1, &subrsoff);

	if (subrsoff == 0)
	{
		return stbtt__new_buf(nullptr, 0);
	}

	stbtt__buf_seek(&cff, private_loc[1] + subrsoff);

	return stbtt__cff_get_index(&cff);
}

inline static int stbtt_InitFont_internal(stbtt_fontinfo* info, unsigned char* data, int fontstart)
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

	const unsigned int cmap{ stbtt__find_table(data, fontstart, "cmap") };       // required

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
		const unsigned int cff{ stbtt__find_table(data, fontstart, "CFF ") };

		if (cff == 0)
		{
			return 0;
		}

		info->fontdicts = stbtt__new_buf(nullptr, 0);
		info->fdselect = stbtt__new_buf(nullptr, 0);

		// @TODO this should use size from table (not 512MB)
		info->cff = stbtt__new_buf(data + cff, 512 * 1024 * 1024);
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

		unsigned int cstype{ 2 };
		unsigned int charstrings{};
		unsigned int fdarrayoff{};
		unsigned int fdselectoff{};

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

	const unsigned int t{ stbtt__find_table(data, fontstart, "maxp") };
	info->numGlyphs = (t != 0) ? ttUSHORT(data + t + 4) : 0xFFFF;

	// find a cmap encoding table we understand *now* to avoid searching
	// later. (todo: could make this installable)
	// the same regardless of glyph.
	const signed int numTables{ ttUSHORT(data + cmap + 2) };
	info->index_map = 0;

	for (int i{}; i < numTables; ++i)
	{
		const unsigned int encoding_record{ cmap + 4 + 8 * i };

		// find an encoding we understand:
		switch(ttUSHORT(data+encoding_record))
		{
			case STBTT_PLATFORM_ID_MICROSOFT:
			{
				switch (ttUSHORT(data + encoding_record + 2))
				{
					case STBTT_MS_EID_UNICODE_BMP:
					{
						break;
					}
					case STBTT_MS_EID_UNICODE_FULL:
					{
						// MS/Unicode
						info->index_map = cmap + ttULONG(data + encoding_record + 4);
						break;
					}
					default: {}
				}
				break;
			}
			case STBTT_PLATFORM_ID_UNICODE:
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

inline int stbtt_FindGlyphIndex(const stbtt_fontinfo* info, int unicode_codepoint)
{
	unsigned char* data{ info->data };
	const unsigned int index_map{ static_cast<unsigned int>(info->index_map) };
	const unsigned short format{ ttUSHORT(data + index_map + 0) };

	if (format == 0)
	{ // apple byte encoding
		const signed int bytes{ ttUSHORT(data + index_map + 2) };

		if (unicode_codepoint < bytes - 6)
		{
			return ttBYTE(data + index_map + 6 + unicode_codepoint);
		}

		return 0;
	}

	if (format == 6)
	{
		const unsigned int first{ ttUSHORT(data + index_map + 6) };
		const unsigned int count{ ttUSHORT(data + index_map + 8) };

		if (static_cast<unsigned int>(unicode_codepoint) >= first && static_cast<unsigned int>(unicode_codepoint) < first + count)
		{
			return ttUSHORT(data + index_map + 10 + (unicode_codepoint - first) * 2);
		}

		return 0;
	}

	if (format == 2)
	{
		assert(0); // @TODO: high-byte mapping for japanese/chinese/korean
		return 0;
	}

	if (format == 4)
	{
		if (unicode_codepoint > 0xFFFF)
		{
			return 0;
		}

		// standard mapping for windows fonts: binary search collection of ranges
		const unsigned short segcount{ static_cast<unsigned short>(ttUSHORT(data + index_map + 6) >> 1) };
		unsigned short searchRange{ static_cast<unsigned short>(ttUSHORT(data + index_map + 8) >> 1) };
		unsigned short entrySelector{ ttUSHORT(data + index_map + 10) };
		const unsigned short rangeShift{ static_cast<unsigned short>(ttUSHORT(data + index_map + 12) >> 1) };

		// do a binary search of the segments
		const unsigned int endCount{ index_map + 14 };
		unsigned int search{ endCount };

		// they lie from endCount .. endCount + segCount
		// but searchRange is the nearest power of two, so...
		if (unicode_codepoint >= ttUSHORT(data + search + rangeShift * 2))
		{
			search += rangeShift * 2;
		}

		// now decrement to bias correctly to find smallest
		search -= 2;

		while (entrySelector != 0)
		{
			searchRange >>= 1;
			const unsigned short end{ ttUSHORT(data + search + searchRange * 2) };

			if (unicode_codepoint > end)
			{
				search += searchRange * 2;
			}

			--entrySelector;
		}

		search += 2;

		{
			const unsigned short item{ static_cast<unsigned short>((search - endCount) >> 1) };

			assert(unicode_codepoint <= ttUSHORT(data + endCount + 2 * item));

			const unsigned short start{ ttUSHORT(data + index_map + 14 + segcount * 2 + 2 + 2 * item) };

			if (unicode_codepoint < start)
			{
				return 0;
			}

			unsigned short offset{ ttUSHORT(data + index_map + 14 + segcount * 6 + 2 + 2 * item) };

			if (offset == 0)
			{
				return static_cast<unsigned short>(unicode_codepoint + ttSHORT(data + index_map + 14 + segcount * 4 + 2 + 2 * item));
			}

			return ttUSHORT(data + offset + (unicode_codepoint - start) * 2 + index_map + 14 + segcount * 6 + 2 + 2 * item);
		}
	}

	if (format == 12 || format == 13)
	{
		const unsigned int ngroups{ ttULONG(data + index_map + 12) };
		signed int low{};
		signed int high{ static_cast<signed int>(ngroups) };
		// Binary search the right group.

		while (low < high)
		{
			const signed int mid{ low + ((high - low) >> 1) }; // rounds down, so low <= mid < high
			unsigned int start_char{ ttULONG(data + index_map + 16 + mid * 12) };
			unsigned int end_char{ ttULONG(data + index_map + 16 + mid * 12 + 4) };

			if (static_cast<unsigned int>(unicode_codepoint) < start_char)
			{
				high = mid;
			}
			else if (static_cast<unsigned int>(unicode_codepoint) > end_char)
			{
				low = mid + 1;
			}
			else
			{
				const unsigned int start_glyph{ ttULONG(data + index_map + 16 + mid * 12 + 8) };

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

	// @TODO
	assert(0);
	return 0;
}

inline int stbtt_GetCodepointShape(const stbtt_fontinfo* info, int unicode_codepoint, stbtt_vertex** vertices)
{
	return stbtt_GetGlyphShape(info, stbtt_FindGlyphIndex(info, unicode_codepoint), vertices);
}

inline static void stbtt_setvertex(stbtt_vertex* v, unsigned char type, signed int x, signed int y, signed int cx, signed int cy)
{
	v->type = type;
	v->x = static_cast<signed short>(x);
	v->y = static_cast<signed short>(y);
	v->cx = static_cast<signed short>(cx);
	v->cy = static_cast<signed short>(cy);
}

inline static int stbtt__GetGlyfOffset(const stbtt_fontinfo* info, int glyph_index)
{
	int g1{};
	int g2{};

	assert(!info->cff.size);

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

static int stbtt__GetGlyphInfoT2(const stbtt_fontinfo* info, int glyph_index, int* x0, int* y0, int* x1, int* y1);

inline int stbtt_GetGlyphBox(const stbtt_fontinfo* info, int glyph_index, int* x0, int* y0, int* x1, int* y1)
{
	if (info->cff.size != 0)
	{
		stbtt__GetGlyphInfoT2(info, glyph_index, x0, y0, x1, y1);
	}
	else
	{
		int g{ stbtt__GetGlyfOffset(info, glyph_index) };

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

inline int stbtt_GetCodepointBox(const stbtt_fontinfo* info, int codepoint, int* x0, int* y0, int* x1, int* y1)
{
	return stbtt_GetGlyphBox(info, stbtt_FindGlyphIndex(info,codepoint), x0, y0, x1, y1);
}

inline int stbtt_IsGlyphEmpty(const stbtt_fontinfo* info, int glyph_index)
{
	if (info->cff.size != 0)
	{
		return stbtt__GetGlyphInfoT2(info, glyph_index, nullptr, nullptr, nullptr, nullptr) == 0;
	}

	const int g{ stbtt__GetGlyfOffset(info, glyph_index) };

	if (g < 0)
	{
		return 1;
	}

	const signed short numberOfContours{ ttSHORT(info->data + g) };
	return numberOfContours == 0;
}

inline static int stbtt__close_shape(stbtt_vertex* vertices, int num_vertices, int was_off, int start_off, signed int sx, signed int sy, signed int scx, signed int scy, signed int cx, signed int cy)
{
	if (start_off != 0)
	{
		if (was_off != 0)
		{
			stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx + scx) >> 1, (cy + scy) >> 1, cx, cy);
		}

		stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, sx, sy, scx, scy);
	}
	else
	{
		if (was_off != 0)
		{
			stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, sx, sy, cx, cy);
		}
		else
		{
			stbtt_setvertex(&vertices[num_vertices++], STBTT_vline, sx, sy, 0, 0);
		}
	}

	return num_vertices;
}

inline static int stbtt__GetGlyphShapeTT(const stbtt_fontinfo* info, int glyph_index, stbtt_vertex** pvertices)
{
	int g{ stbtt__GetGlyfOffset(info, glyph_index) };

	if (g < 0)
	{
		return 0;
	}

	unsigned char* data{ info->data };
	int num_vertices{};
	signed short numberOfContours{ ttSHORT(data + g) };
	stbtt_vertex* vertices{};

	if (numberOfContours > 0)
	{
		unsigned char flags{};
		signed int j{};
		signed int was_off{};
		signed int start_off{};
		signed int x{};
		signed int y{};
		signed int cx{};
		signed int cy{};
		signed int sx{};
		signed int sy{};
		signed int scx{};
		signed int scy{};
		unsigned char* endPtsOfContours{ (data + g + 10) };
		const signed int ins{ ttUSHORT(data + g + 10 + numberOfContours * 2) };
		unsigned char* points{ data + g + 10 + numberOfContours * 2 + 2 + ins };
		const signed int n{ 1 + ttUSHORT(endPtsOfContours + numberOfContours * 2 - 2) };
		const signed int m{ n + 2 * numberOfContours };  // a loose bound on how many vertices we might need
		vertices = static_cast<stbtt_vertex*>(static_cast<void>(info->userdata), malloc(m * sizeof(vertices[0])));

		if (vertices == nullptr)
		{
			return 0;
		}

		signed int next_move{};
		unsigned char flagcount{};

		// in first pass, we load uninterpreted data into the allocated array
		// above, shifted to the end of the array so we won't overwrite it when
		// we create our final data starting from the front

		const signed int off{ m - n }; // starting offset for uninterpreted data, regardless of how m ends up being calculated

		// first load flags

		for (int i{}; i < n; ++i)
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
		for (int i{}; i < n; ++i)
		{
			flags = vertices[off + i].type;

			if ((flags & 2) != 0)
			{
				const signed short dx{ *points++ };
				x += (flags & 16) != 0 ? dx : -dx; // ???
			}
			else
			{
				if ((flags & 16) == 0)
				{
					x = x + static_cast<signed short>(points[0] * 256 + points[1]);
					points += 2;
				}
			}

			vertices[off+i].x = static_cast<signed short>(x);
		}

		// now load y coordinates
		for (int i{}; i < n; ++i)
		{
			flags = vertices[off + i].type;

			if ((flags & 4) != 0)
			{
				signed short dy{ *points++ };
				y += (flags & 32) != 0 ? dy : -dy; // ???
			}
			else
			{
				if ((flags & 32) == 0)
				{
					y = y + static_cast<signed short>(points[0] * 256 + points[1]);
					points += 2;
				}
			}
			vertices[off + i].y = static_cast<signed short>(y);
		}

		// now convert them to our format
		for (int i{}; i < n; ++i)
		{
			flags = vertices[off + i].type;
			x = static_cast<signed short>(vertices[off + i].x);
			y = static_cast<signed short>(vertices[off + i].y);

			if (next_move == i)
			{
				if (i != 0)
				{
					num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx, sy, scx, scy, cx, cy);
				}

				// now start the new one
				start_off = !(flags & 1);

				if (start_off != 0)
				{
					// if we start off with an off-curve point, then when we need to find a point on the curve
					// where we can start, and we need to save some state for when we wraparound.
					scx = x;
					scy = y;

					if ((vertices[off + i + 1].type & 1) == 0)
					{
						// next point is also a curve point, so interpolate an on-point curve
						sx = (x + static_cast<signed int>(vertices[off + i + 1].x)) >> 1;
						sy = (y + static_cast<signed int>(vertices[off + i + 1].y)) >> 1;
					}
					else
					{
						// otherwise just use the next point as our start point
						sx = static_cast<signed int>(vertices[off + i + 1].x);
						sy = static_cast<signed int>(vertices[off + i + 1].y);
						++i; // we're using point i+1 as the starting point, so skip it
					}
				}
				else
				{
					sx = x;
					sy = y;
				}

				stbtt_setvertex(&vertices[num_vertices++], STBTT_vmove, sx, sy, 0, 0);
				was_off = 0;
				next_move = 1 + ttUSHORT(endPtsOfContours + j * 2);
				++j;
			}
			else
			{
				if ((flags & 1) == 0)
				{ // if it's a curve
					if (was_off != 0) // two off-curve control points in a row means interpolate an on-curve midpoint
					{
						stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx + x) >> 1, (cy + y) >> 1, cx, cy);
					}

					cx = x;
					cy = y;
					was_off = 1;
				}
				else
				{
					if (was_off != 0)
					{
						stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, x, y, cx, cy);
					}
					else
					{
						stbtt_setvertex(&vertices[num_vertices++], STBTT_vline, x, y, 0, 0);
					}

					was_off = 0;
				}
			}
		}

		num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx, sy, scx, scy, cx, cy);
	}
	else if (numberOfContours == -1)
	{
		// Compound shapes.
		int more{ 1 };
		unsigned char* comp{ data + g + 10 };

		while (more != 0)
		{
			stbtt_vertex* comp_verts{};
			float mtx[6]{ 1, 0, 0, 1, 0, 0 };

			const unsigned short flags{ static_cast<unsigned short>(ttSHORT(comp)) };
			comp += 2;
			const unsigned short gidx{ static_cast<unsigned short>(ttSHORT(comp)) };
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
					mtx[4] = ttCHAR(comp);
					comp += 1;
					mtx[5] = ttCHAR(comp);
					comp += 1;
				}
			}
			else
			{
				// @TODO handle matching point
					assert(0);
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
			const int comp_num_verts{ stbtt_GetGlyphShape(info, gidx, &comp_verts) };

			if (comp_num_verts > 0)
			{
				// Transform vertices.
				for (int i{}; i < comp_num_verts; ++i)
				{
					stbtt_vertex* v{ &comp_verts[i] };
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
				stbtt_vertex* tmp{ static_cast<stbtt_vertex*>(static_cast<void>(info->userdata), malloc((num_vertices + comp_num_verts) * sizeof(stbtt_vertex))) };

				if (tmp == nullptr)
				{
					if (vertices != nullptr)
					{
						static_cast<void>(info->userdata), free(vertices);
					}

					if (comp_verts != nullptr)
					{
						static_cast<void>(info->userdata), free(comp_verts);
					}

					return 0;
				}

				if (num_vertices > 0)
				{
					std::memcpy(tmp, vertices, num_vertices * sizeof(stbtt_vertex));
				}

				std::memcpy(tmp + num_vertices, comp_verts, comp_num_verts * sizeof(stbtt_vertex));

				if (vertices != nullptr)
				{
					static_cast<void>(info->userdata), free(vertices);
				}

				vertices = tmp;
				static_cast<void>(info->userdata), free(comp_verts);
				num_vertices += comp_num_verts;
			}

			// More components ?
			more = flags & (1 << 5);
		}
	}
	else if (numberOfContours < 0)
	{
		// @TODO other compound variations?
		assert(0);
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
	int bounds{};
	int started{};
	float first_x{};
	float first_y{};
	float x{};
	float y{};
	signed int min_x{};
	signed int max_x{};
	signed int min_y{};
	signed int max_y{};
	stbtt_vertex* pvertices{};
	int num_vertices{};
};

#define STBTT__CSCTX_INIT(bounds) {bounds, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0}

inline static void stbtt__track_vertex(stbtt__csctx* c, signed int x, signed int y)
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

inline static void stbtt__csctx_v(stbtt__csctx* c, unsigned char type, signed int x, signed int y, signed int cx, signed int cy, signed int cx1, signed int cy1)
{
	if (c->bounds != 0)
	{
		stbtt__track_vertex(c, x, y);

		if (type == STBTT_vcubic)
		{
			stbtt__track_vertex(c, cx, cy);
			stbtt__track_vertex(c, cx1, cy1);
		}
	}
	else
	{
		stbtt_setvertex(&c->pvertices[c->num_vertices], type, x, y, cx, cy);
		c->pvertices[c->num_vertices].cx1 = static_cast<signed short>(cx1);
		c->pvertices[c->num_vertices].cy1 = static_cast<signed short>(cy1);
	}

	c->num_vertices++;
}

inline static void stbtt__csctx_close_shape(stbtt__csctx* ctx)
{
	if (ctx->first_x != ctx->x || ctx->first_y != ctx->y)
	{
		stbtt__csctx_v(ctx, STBTT_vline, static_cast<int>(ctx->first_x), static_cast<int>(ctx->first_y), 0, 0, 0, 0);
	}
}

inline static void stbtt__csctx_rmove_to(stbtt__csctx* ctx, float dx, float dy)
{
	stbtt__csctx_close_shape(ctx);
	ctx->first_x = ctx->x = ctx->x + dx;
	ctx->first_y = ctx->y = ctx->y + dy;
	stbtt__csctx_v(ctx, STBTT_vmove, static_cast<int>(ctx->x), static_cast<int>(ctx->y), 0, 0, 0, 0);
}

inline static void stbtt__csctx_rline_to(stbtt__csctx* ctx, float dx, float dy)
{
	ctx->x += dx;
	ctx->y += dy;
	stbtt__csctx_v(ctx, STBTT_vline, static_cast<int>(ctx->x), static_cast<int>(ctx->y), 0, 0, 0, 0);
}

inline static void stbtt__csctx_rccurve_to(stbtt__csctx* ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3)
{
	const float cx1{ ctx->x + dx1 };
	const float cy1{ ctx->y + dy1 };
	const float cx2{ cx1 + dx2 };
	const float cy2{ cy1 + dy2 };

	ctx->x = cx2 + dx3;
	ctx->y = cy2 + dy3;

	stbtt__csctx_v(ctx, STBTT_vcubic, static_cast<int>(ctx->x), static_cast<int>(ctx->y), static_cast<int>(cx1), static_cast<int>(cy1), static_cast<int>(cx2), static_cast<int>(cy2));
}

inline static stbtt__buf stbtt__get_subr(stbtt__buf idx, int n)
{
	const int count{ stbtt__cff_index_count(&idx) };
	int bias{ 107 };

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

inline static stbtt__buf stbtt__cid_get_glyph_subrs(const stbtt_fontinfo* info, int glyph_index)
{
	stbtt__buf fdselect{ info->fdselect };
	int fdselector{ -1 };

	stbtt__buf_seek(&fdselect, 0);
	const int fmt{ stbtt__buf_get8(&fdselect) };

	if (fmt == 0)
	{
		// untested
		stbtt__buf_skip(&fdselect, glyph_index);
		fdselector = stbtt__buf_get8(&fdselect);
	}
	else if (fmt == 3)
	{
		const int nranges{ static_cast<int>(stbtt__buf_get16(&fdselect)) };
		int start{ static_cast<int>(stbtt__buf_get16(&fdselect)) };

		for (int i{}; i < nranges; ++i)
		{
			const int v{ stbtt__buf_get8(&fdselect) };
			const int end{ static_cast<int>(stbtt__buf_get16(&fdselect)) };

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

inline static int stbtt__run_charstring(const stbtt_fontinfo* info, int glyph_index, stbtt__csctx* c)
{
	int in_header{ 1 };
	int maskbits{};
	int subr_stack_height{};
	int sp{};
	int has_subrs{};
	std::vector<float> s(48);
	std::vector<stbtt__buf> subr_stack(10);
	stbtt__buf subrs{ info->subrs };

	#define STBTT__CSERR(s) (0)

	// this currently ignores the initial width value, which isn't needed if we have hmtx
	stbtt__buf b{ stbtt__cff_index_get(info->charstrings, glyph_index) };

	while (b.cursor < b.size)
	{
		int i{};
		int clear_stack{ 1 };
		const int b0{ stbtt__buf_get8(&b) };

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
				stbtt__buf_skip(&b, (maskbits + 7) / 8);
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
				maskbits += (sp / 2);
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

					stbtt__csctx_rccurve_to(c, 0, s[i], s[i + 1], s[i + 2], s[i + 3], (sp - i == 5) ? s[i + 4] : 0.0f);
					i += 4;

					hvcurveto:
					if (i + 3 >= sp)
					{
						break;
					}

					stbtt__csctx_rccurve_to(c, s[i], 0, s[i + 1], s[i + 2], (sp - i == 5) ? s[i + 4] : 0.0f, s[i + 3]);
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
					if (b0 == 0x1B)
					{
						stbtt__csctx_rccurve_to(c, s[i], f, s[i + 1], s[i + 2], s[i + 3], 0.0);
					}
					else
					{
						stbtt__csctx_rccurve_to(c, f, s[i], s[i + 1], s[i + 2], 0.0, s[i + 3]);
					}

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

				const int v{ static_cast<int>(s[--sp]) };

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

						if (std::fabs(dx) > std::fabs(dy))
						{
							dy6 = -dy;
						}
						else
						{
							dx6 = -dx;
						}

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
					f = static_cast<float>(static_cast<signed int>(stbtt__buf_get32(&b)) / 0x10000);
				}
				else
				{
					stbtt__buf_skip(&b, -1);
					f = static_cast<float>(static_cast<signed short>(stbtt__cff_int(&b)));
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

inline static int stbtt__GetGlyphShapeT2(const stbtt_fontinfo* info, int glyph_index, stbtt_vertex** pvertices)
{
	// runs the charstring twice, once to count and once to output (to avoid realloc)
	stbtt__csctx count_ctx = STBTT__CSCTX_INIT(1);
	stbtt__csctx output_ctx = STBTT__CSCTX_INIT(0);

	if (stbtt__run_charstring(info, glyph_index, &count_ctx) != 0)
	{
		*pvertices = static_cast<stbtt_vertex*>(static_cast<void>(info->userdata), malloc(count_ctx.num_vertices * sizeof(stbtt_vertex)));

		output_ctx.pvertices = *pvertices;

		if (stbtt__run_charstring(info, glyph_index, &output_ctx) != 0)
		{
			assert(output_ctx.num_vertices == count_ctx.num_vertices);
			return output_ctx.num_vertices;
		}
	}

	*pvertices = nullptr;
	return 0;
}

inline static int stbtt__GetGlyphInfoT2(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1)
{
	stbtt__csctx c = STBTT__CSCTX_INIT(1);
	const int r{ stbtt__run_charstring(info, glyph_index, &c) };

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

inline int stbtt_GetGlyphShape(const stbtt_fontinfo* info, int glyph_index, stbtt_vertex** pvertices)
{
	if (info->cff.size == 0)
	{
		return stbtt__GetGlyphShapeTT(info, glyph_index, pvertices);
	}

	return stbtt__GetGlyphShapeT2(info, glyph_index, pvertices);
}

inline void stbtt_GetGlyphHMetrics(const stbtt_fontinfo* info, int glyph_index, int* advanceWidth, int* leftSideBearing)
{
	const unsigned short numOfLongHorMetrics{ ttUSHORT(info->data + info->hhea + 34) };

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

inline static int stbtt__GetGlyphKernInfoAdvance(const stbtt_fontinfo* info, int glyph1, int glyph2)
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

	int l{};
	int r{ ttUSHORT(data + 10) - 1 };
	const unsigned int needle{ static_cast<unsigned int>(glyph1 << 16 | glyph2) };

	while (l <= r)
	{
		const int m{ (l + r) >> 1 };
		const unsigned int straw{ ttULONG(data + 18 + (m * 6)) }; // note: unaligned read

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

inline static signed int stbtt__GetCoverageIndex(unsigned char* coverageTable, int glyph)
{
	switch(ttUSHORT(coverageTable))
	{
		case 1:
		{
			// Binary search.
			signed int l{};
			signed int r{ ttUSHORT(coverageTable + 2) - 1 };

			while (l <= r)
			{
				unsigned char* glyphArray{ coverageTable + 4 };
				const signed int m{ (l + r) >> 1 };
				const int straw{ ttUSHORT(glyphArray + 2 * m) };

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
			signed int l{};
			signed int r{ ttUSHORT(coverageTable + 2) - 1 };

			while (l <= r)
			{
				const signed int m{ (l + r) >> 1 };
				unsigned char* rangeRecord{ rangeArray + 6 * m };
				const int strawStart{ ttUSHORT(rangeRecord) };
				const int strawEnd{ ttUSHORT(rangeRecord + 2) };

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

		default:
		{
			// There are no other cases.
			assert(0);
			break;
		}
	}

	return -1;
}

inline static signed int stbtt__GetGlyphClass(unsigned char* classDefTable, int glyph)
{
	switch(ttUSHORT(classDefTable))
	{
		case 1:
		{
			const unsigned short startGlyphID{ ttUSHORT(classDefTable + 2) };
			const unsigned short glyphCount{ ttUSHORT(classDefTable + 4) };
			unsigned char* classDef1ValueArray{ classDefTable + 6 };

			if (glyph >= startGlyphID && glyph < startGlyphID + glyphCount)
			{
				return static_cast<signed int>(ttUSHORT(classDef1ValueArray + 2 * (glyph - startGlyphID)));
			}

			classDefTable = classDef1ValueArray + 2 * glyphCount;
			break;
		}

		case 2:
		{
			const unsigned short classRangeCount{ ttUSHORT(classDefTable + 2) };
			unsigned char* classRangeRecords{ classDefTable + 4 };

			// Binary search.
			signed int l{};
			signed int r{ classRangeCount - 1};
			const int needle{ glyph };

			while (l <= r)
			{
				const signed int m{ (l + r) >> 1 };
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
					return static_cast<signed int>(ttUSHORT(classRangeRecord + 4));
				}
			}

			classDefTable = classRangeRecords + 6 * classRangeCount;
			break;
		}

		default:
		{
			// There are no other cases.
			assert(0);
			break;
		}
	}

	return -1;
}

// Define to STBTT_assert(x) if you want to break on unimplemented formats.
#define STBTT_GPOS_TODO_assert(x)

inline static signed int stbtt__GetGlyphGPOSInfoAdvance(const stbtt_fontinfo* info, int glyph1, int glyph2)
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
	const unsigned short lookupCount{ ttUSHORT(lookupList) };

	for (signed int i{}; i < lookupCount; ++i)
	{
		unsigned char* lookupTable{ lookupList + ttUSHORT(lookupList + 2 + 2 * i) };

        switch(ttUSHORT(lookupTable))
		{
            case 2:
			{ // Pair Adjustment Positioning Subtable
				const unsigned short subTableCount{ ttUSHORT(lookupTable + 4) };
				unsigned char* subTableOffsets{ lookupTable + 6 };

				for (signed int sti{}; sti < subTableCount; sti++)
				{
					unsigned char* table{ lookupTable + ttUSHORT(subTableOffsets + 2 * sti) };
					const signed int coverageIndex{ stbtt__GetCoverageIndex(table + ttUSHORT(table + 2), glyph1) };

					if (coverageIndex == -1)
					{
						continue;
					}

                    switch (ttUSHORT(table))
					{
                        case 1:
						{
							const unsigned short valueFormat1{ ttUSHORT(table + 4) };
							const unsigned short valueFormat2{ ttUSHORT(table + 6) };
							const signed int valueRecordPairSizeInBytes{ 2 };
							const unsigned short pairSetCount{ ttUSHORT(table + 8) };
							const unsigned short pairPosOffset{ ttUSHORT(table + 10 + 2 * coverageIndex) };
							unsigned char* pairValueTable{ table + pairPosOffset };
							const unsigned short pairValueCount{ ttUSHORT(pairValueTable) };
							unsigned char* pairValueArray{ pairValueTable + 2 };

                            STBTT_GPOS_TODO_assert(valueFormat1 == 4);

							if (valueFormat1 != 4)
							{
								return 0;
							}

							STBTT_GPOS_TODO_assert(valueFormat2 == 0);

							if (valueFormat2 != 0)
							{
								return 0;
							}

							assert(coverageIndex < pairSetCount);
                            STBTT__NOTUSED(pairSetCount);

							int needle{ glyph2 };
							signed int r{ pairValueCount - 1 };
							signed int l{};

                            // Binary search.
                            while (l <= r)
							{
								signed int m{ (l + r) >> 1 };
								unsigned char* pairValue{ pairValueArray + (2 + valueRecordPairSizeInBytes) * m };
								const unsigned short secondGlyph{ ttUSHORT(pairValue) };

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
							const unsigned short valueFormat1{ ttUSHORT(table + 4) };
							const unsigned short valueFormat2{ ttUSHORT(table + 6) };
							const int glyph1class{ stbtt__GetGlyphClass(table + ttUSHORT(table + 8), glyph1) };
							const int glyph2class{ stbtt__GetGlyphClass(table + ttUSHORT(table + 10), glyph2) };
							const unsigned short class1Count{ ttUSHORT(table + 12) };
							const unsigned short class2Count{ ttUSHORT(table + 14) };

							assert(glyph1class < class1Count);
							assert(glyph2class < class2Count);

                            STBTT_GPOS_TODO_assert(valueFormat1 == 4);

							if (valueFormat1 != 4)
							{
								return 0;
							}

                            STBTT_GPOS_TODO_assert(valueFormat2 == 0);

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

                        default:
						{
                            // There are no other cases.
							assert(0);
                            break;
                        }
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

inline int stbtt_GetGlyphKernAdvance(const stbtt_fontinfo* info, int glyph1, int glyph2)
{
	int xAdvance{};

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

inline int stbtt_GetCodepointKernAdvance(const stbtt_fontinfo* info, int ch1, int ch2)
{
	if (info->kern == 0 && info->gpos == 0) // if no kerning table, don't waste time looking up both codepoint->glyphs
	{
		return 0;
	}

	return stbtt_GetGlyphKernAdvance(info, stbtt_FindGlyphIndex(info,ch1), stbtt_FindGlyphIndex(info,ch2));
}

inline void stbtt_GetCodepointHMetrics(const stbtt_fontinfo* info, int codepoint, int* advanceWidth, int* leftSideBearing)
{
	stbtt_GetGlyphHMetrics(info, stbtt_FindGlyphIndex(info,codepoint), advanceWidth, leftSideBearing);
}

inline void stbtt_GetFontVMetrics(const stbtt_fontinfo* info, int* ascent, int* descent, int* lineGap)
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

inline int stbtt_GetFontVMetricsOS2(const stbtt_fontinfo* info, int* typoAscent, int* typoDescent, int* typoLineGap)
{
	const int tab{ static_cast<int>(stbtt__find_table(info->data, info->fontstart, "OS/2")) };

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

inline void stbtt_GetFontBoundingBox(const stbtt_fontinfo* info, int* x0, int* y0, int* x1, int* y1)
{
	*x0 = ttSHORT(info->data + info->head + 36);
	*y0 = ttSHORT(info->data + info->head + 38);
	*x1 = ttSHORT(info->data + info->head + 40);
	*y1 = ttSHORT(info->data + info->head + 42);
}

inline float stbtt_ScaleForPixelHeight(const stbtt_fontinfo* info, float height)
{
	return static_cast<float>(height / (ttSHORT(info->data + info->hhea + 4) - ttSHORT(info->data + info->hhea + 6)));
}

inline float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo* info, float pixels)
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

inline void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo* font, int glyph, float scale_x, float scale_y,float shift_x, float shift_y, int* ix0, int* iy0, int* ix1, int* iy1)
{
	int x0{};
	int y0{};
	int x1{};
	int y1{}; // =0 suppresses compiler warning

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
			*ix0 = static_cast<int>(std::floor(x0 * scale_x + shift_x));
		}

		if (iy0 != nullptr)
		{
			*iy0 = static_cast<int>(std::floor(-y1 * scale_y + shift_y));
		}

		if (ix1 != nullptr)
		{
			*ix1 = static_cast<int>(std::ceil(x1 * scale_x + shift_x));
		}

		if (iy1 != nullptr)
		{
			*iy1 = static_cast<int>(std::ceil(-y0 * scale_y + shift_y));
		}
	}
}

inline void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo* font, int glyph, float scale_x, float scale_y, int* ix0, int* iy0, int* ix1, int* iy1)
{
   stbtt_GetGlyphBitmapBoxSubpixel(font, glyph, scale_x, scale_y, 0.0F, 0.0F, ix0, iy0, ix1, iy1);
}

inline void stbtt_GetCodepointBitmapBoxSubpixel(const stbtt_fontinfo* font, int codepoint, float scale_x, float scale_y, float shift_x, float shift_y, int* ix0, int* iy0, int* ix1, int* iy1)
{
   stbtt_GetGlyphBitmapBoxSubpixel(font, stbtt_FindGlyphIndex(font,codepoint), scale_x, scale_y, shift_x, shift_y, ix0, iy0, ix1, iy1);
}

inline void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo* font, int codepoint, float scale_x, float scale_y, int* ix0, int* iy0, int* ix1, int* iy1)
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
	int num_remaining_in_head_chunk{};
};

inline static void *stbtt__hheap_alloc(stbtt__hheap* hh, size_t size, void* userdata)
{
	if (hh->first_free != nullptr)
	{
		void* p{ hh->first_free };
		hh->first_free = *reinterpret_cast<void**>(p);
		return p;
	}

	if (hh->num_remaining_in_head_chunk == 0)
	{
		const int count{ (size < 32 ? 2000 : size < 128 ? 800 : 100) };
		stbtt__hheap_chunk* c{ static_cast<stbtt__hheap_chunk*>(static_cast<void>(userdata), malloc(sizeof(stbtt__hheap_chunk) + size * count)) };

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
	int invert{};
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

inline static stbtt__active_edge* stbtt__new_active(stbtt__hheap* hh, stbtt__edge* e, int off_x, float start_point, void* userdata)
{
	stbtt__active_edge* z{ static_cast<stbtt__active_edge*>(stbtt__hheap_alloc(hh, sizeof(*z), userdata)) };
	float dxdy{ (e->x1 - e->x0) / (e->y1 - e->y0) };
	assert(z != nullptr);

	//STBTT_assert(e->y0 <= start_point);
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
inline static void stbtt__handle_clipped_edge(float* scanline, int x, stbtt__active_edge* e, float x0, float y0, float x1, float y1)
{
	if (y0 == y1) //-V550
	{
		return;
	}

	assert(y0 < y1);
	assert(e->sy <= e->ey);

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

	if (x0 == x) //-V550
	{
		assert(x1 <= x + 1);
	}
	else if (x0 == x + 1) //-V550
	{
		assert(x1 >= x);
	}
	else if (x0 <= x)
	{
		assert(x1 <= x);
	}
	else if (x0 >= x + 1)
	{
		assert(x1 >= x + 1);
	}
	else
	{
		assert(x1 >= x && x1 <= x + 1);
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
		assert(x0 >= x && x0 <= x + 1 && x1 >= x && x1 <= x + 1);
		scanline[x] += e->direction * (y1 - y0) * (1.0F - ((x0 - x) + (x1 - x)) / 2.0F); // coverage = 1 - average x position
	}
}

inline static void stbtt__fill_active_edges_new(float* scanline, float* scanline_fill, int len, stbtt__active_edge* e, float y_top)
{
	const float y_bottom{ y_top + 1.0F };

	while (e != nullptr)
	{
		// brute force every pixel

		// compute intersection points with top & bottom
		assert(e->ey >= y_top);

		if (e->fdx == 0.0F)
		{
			const float x0{ e->fx };

			if (x0 < len)
			{
				if (x0 >= 0.0F)
				{
					stbtt__handle_clipped_edge(scanline, static_cast<int>(x0),e, x0, y_top, x0, y_bottom);
					stbtt__handle_clipped_edge(scanline_fill-1, static_cast<int>(x0) + 1, e, x0, y_top, x0, y_bottom);
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

			assert(e->sy <= y_bottom && e->ey >= y_top);

			// compute endpoints of line segment clipped to this scanline (if the
			// line segment starts on this scanline. x0 is the intersection of the
			// line with y_top, but that may be off the line segment.
			if (e->sy > y_top)
			{
				x_top = x0 + dx * (e->sy - y_top);
				sy0 = e->sy;
			}
			else
			{
				x_top = x0;
				sy0 = y_top;
			}

			if (e->ey < y_bottom)
			{
				x_bottom = x0 + dx * (e->ey - y_top);
				sy1 = e->ey;
			}
			else
			{
				x_bottom = xb;
				sy1 = y_bottom;
			}

			float dy{ e->fdy };

			if (x_top >= 0.0F && x_bottom >= 0.0F && x_top < len && x_bottom < len)
			{
				// from here on, we don't have to range check x values

				if (static_cast<int>(x_top) == static_cast<int>(x_bottom))
				{
					// simple case, only spans one pixel
					const int x{ static_cast<int>(x_top) };
					const float height{ sy1 - sy0 };

					assert(x >= 0 && x < len);
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

					const int x1{ static_cast<int>(x_top) };
					const int x2{ static_cast<int>(x_bottom) };
					// compute intersection with y axis at x1+1
					float y_crossing{ (x1 + 1 - x0) * dy + y_top };
					const float sign{ e->direction };
					// area of the rectangle covered from y0..y_crossing
					float area{ sign * (y_crossing - sy0) };
					// area of the triangle (x_top,y0), (x+1,y0), (x+1,y_crossing)
					scanline[x1] += area * (1 - ((x_top - x1) + (x1 + 1 - x1)) / 2);
					const float step{ sign * dy };

					for (int x{ x1 + 1 }; x < x2; ++x)
					{
						scanline[x] += area + step / 2.0F;
						area += step;
					}

					y_crossing += dy * (x2 - (x1 + 1));
					assert(std::fabs(area) <= 1.01F);
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
				for (int x{}; x < len; ++x)
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
inline static void stbtt__rasterize_sorted_edges(stbtt__bitmap* result, stbtt__edge* e, int n, int vsubsample, int off_x, int off_y, void* userdata)
{
	stbtt__hheap hh{};
	stbtt__active_edge* active{};
	std::vector<float> scanline_data(129);

	STBTT__NOTUSED(vsubsample);

	float* scanline{ (result->w > 64) ? static_cast<float*>(static_cast<void>(userdata), malloc((result->w * 2 + 1) * sizeof(float))) : scanline_data.data() };
	float* scanline2{ scanline + result->w };
	int y{ off_y };

	e[n].y0 = static_cast<float>((off_y + result->h)) + 1.0F;

	int j{};

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
				assert(z->direction);
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

					assert(z->ey >= scan_y_top); // if we get really unlucky a tiny bit of an edge can be out of bounds
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

			for (int i{}; i < result->w; ++i)
			{
				sum += scanline2[i];
				int m{ static_cast<int>(std::fabsf(scanline[i] + sum) * 255.0F + 0.5F) };

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
		static_cast<void>(userdata), free(scanline);
	}
}

#define STBTT__COMPARE(a,b)  ((a)->y0 < (b)->y0)

inline static void stbtt__sort_edges_ins_sort(stbtt__edge* p, int n)
{
	for (int i{ 1 }; i < n; ++i)
	{
		stbtt__edge t{ p[i] };
		stbtt__edge* a{ &t };

		int j{ i };

		while (j > 0)
		{
			stbtt__edge* b{ &p[j - 1] };

			if (STBTT__COMPARE(a, b) == 0)
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

inline static void stbtt__sort_edges_quicksort(stbtt__edge* p, int n)
{
	/* threshold for transitioning to insertion sort */
	while (n > 12)
	{
		/* compute median of three */
		const int m{ n >> 1 };
		const int c01{ STBTT__COMPARE(&p[0], &p[m]) };
		const int c12{ STBTT__COMPARE(&p[m], &p[n - 1]) };

		/* if 0 >= mid >= end, or 0 < mid < end, then use mid */
		if (c01 != c12)
		{
			/* otherwise, we'll need to swap something else to middle */
			const int c{ STBTT__COMPARE(&p[0], &p[n - 1]) };

			/* 0>mid && mid<n:  0>n => n; 0<n => 0 */
			/* 0<mid && mid>n:  0>n => 0; 0<n => n */
			const int z{ (c == c12) ? 0 : n - 1 };

			std::swap(p[z], p[m]);
		}

		/* now p[m] is the median-of-three */
		/* swap it to the beginning so it won't move around */
		std::swap(p[0], p[m]);

		/* partition loop */
		int i{ 1 };
		int j{ n - 1 };

		for(;;)
		{
			/* handling of equality is crucial here */
			/* for sentinels & efficiency with duplicates */
			for (;;++i)
			{
				if (!STBTT__COMPARE(&p[i], &p[0]))
				{
					break;
				}
			}

			for (;;--j)
			{
				if (!STBTT__COMPARE(&p[0], &p[j]))
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

inline static void stbtt__sort_edges(stbtt__edge* p, int n)
{
	stbtt__sort_edges_quicksort(p, n);
	stbtt__sort_edges_ins_sort(p, n);
}

using stbtt__point = struct
{
	float x{};
	float y{};
};

inline static void stbtt__rasterize(stbtt__bitmap* result, stbtt__point* pts, const int* wcount, int windings, float scale_x, float scale_y, float shift_x, float shift_y, int off_x, int off_y, int invert, void* userdata)
{
	const int vsubsample{ 1 };
	// vsubsample should divide 255 evenly; otherwise we won't reach full opacity

	// now we have to blow out the windings into explicit edge lists
	int n{};

	for (int i{}; i < windings; ++i)
	{
		n += wcount[i];
	}

	stbtt__edge* e{ static_cast<stbtt__edge*>(static_cast<void>(userdata), malloc(sizeof(*e) * (n + 1))) }; // add an extra one as a sentinel

	if (e == nullptr)
	{
		return;
	}

	n = 0;

	int m{};
	const float y_scale_inv{ (invert != 0) ? -scale_y : scale_y };

	for (int i{}; i < windings; ++i)
	{
		const stbtt__point* p{ pts + m };
		m += wcount[i];
		int j{ wcount[i] - 1 };

		for (int k{}; k < wcount[i]; j = k++)
		{
			int a{ k };
			int b{ j };

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

	static_cast<void>(userdata), free(e);
}

inline static void stbtt__add_point(stbtt__point* points, int n, float x, float y)
{
	if (points == nullptr)
	{
		return; // during first pass, it's unallocated
	}

	points[n].x = x;
	points[n].y = y;
}

// tessellate until threshold p is happy... @TODO warped to compensate for non-linear stretching
inline static int stbtt__tesselate_curve(stbtt__point* points, int* num_points, float x0, float y0, float x1, float y1, float x2, float y2, float objspace_flatness_squared, int n)
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

inline static void stbtt__tesselate_cubic(stbtt__point* points, int* num_points, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float objspace_flatness_squared, int n)
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
inline static stbtt__point* stbtt_FlattenCurves(stbtt_vertex* vertices, int num_verts, float objspace_flatness, int** contour_lengths, int* num_contours, void* userdata)
{
	stbtt__point* points{};
	int num_points{};
	const float objspace_flatness_squared{ objspace_flatness * objspace_flatness };
	int n{};
	int start{};

	// count how many "moves" there are to get the contour count
	for (int i{}; i < num_verts; ++i)
	{
		if (vertices[i].type == STBTT_vmove)
		{
			++n;
		}
	}

	*num_contours = n;

	if (n == 0)
	{
		return nullptr;
	}

	*contour_lengths = static_cast<int*>(static_cast<void>(userdata), malloc(sizeof(**contour_lengths) * n));

	if (*contour_lengths == nullptr)
	{
		*num_contours = 0;
		return nullptr;
	}

	// make two passes through the points so we don't need to realloc
	for (int pass{}; pass < 2; ++pass)
	{
		float x{};
		float y{};

		if (pass == 1)
		{
			points = static_cast<stbtt__point*>(static_cast<void>(userdata), malloc(num_points * sizeof(points[0])));

			if (points == nullptr)
			{
				goto error;
			}
		}

		num_points = 0;
		n= -1;

		for (int i{}; i < num_verts; ++i)
		{
			switch (vertices[i].type)
			{
				case STBTT_vmove:
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
				case STBTT_vline:
				{
					x = vertices[i].x;
					y = vertices[i].y;
					stbtt__add_point(points, num_points++, x, y);
					break;
				}
				case STBTT_vcurve:
				{
					stbtt__tesselate_curve(points, &num_points, x, y, vertices[i].cx, vertices[i].cy, vertices[i].x, vertices[i].y,	objspace_flatness_squared, 0);
					x = vertices[i].x;
					y = vertices[i].y;
					break;
				}
				case STBTT_vcubic:
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
		static_cast<void>(userdata), free(points);
		static_cast<void>(userdata), free(*contour_lengths);

		*contour_lengths = nullptr;
		*num_contours = 0;
		return nullptr;
}

inline void stbtt_Rasterize(stbtt__bitmap* result, float flatness_in_pixels, stbtt_vertex* vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert, void* userdata)
{
	const float scale{ scale_x > scale_y ? scale_y : scale_x };
	int winding_count{};
	int* winding_lengths{};

	stbtt__point* windings{ stbtt_FlattenCurves(vertices, num_verts, flatness_in_pixels / scale, &winding_lengths, &winding_count, userdata) };

	if (windings != nullptr)
	{
		stbtt__rasterize(result, windings, winding_lengths, winding_count, scale_x, scale_y, shift_x, shift_y, x_off, y_off, invert, userdata);
		static_cast<void>(userdata), free(winding_lengths);
		static_cast<void>(userdata), free(windings);
	}
}

inline void stbtt_FreeBitmap(unsigned char* bitmap, void* userdata)
{
	static_cast<void>(userdata), free(bitmap);
}

unsigned char* stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo* info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int* width, int* height, int* xoff, int* yoff)
{
	stbtt_vertex* vertices{};
	const int num_verts{ stbtt_GetGlyphShape(info, glyph, &vertices) };

	if (scale_x == 0.0F) //-V550
	{
		scale_x = scale_y;
	}

	if (scale_y == 0.0F) //-V550
	{
		if (scale_x == 0.0F) //-V550
		{
			static_cast<void>(info->userdata), free(vertices);

			return nullptr;
		}

		scale_y = scale_x;
	}

	int ix0{};
	int iy0{};
	int ix1{};
	int iy1{};

	stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0,&iy0,&ix1,&iy1);
	stbtt__bitmap gbm;

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
		gbm.pixels = static_cast<unsigned char*>(static_cast<void>(info->userdata), malloc(gbm.w * gbm.h));

		if (gbm.pixels != nullptr)
		{
			gbm.stride = gbm.w;
			stbtt_Rasterize(&gbm, 0.35F, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1, info->userdata);
		}
	}

	static_cast<void>(info->userdata), free(vertices);

	return gbm.pixels;
}

inline unsigned char* stbtt_GetGlyphBitmap(const stbtt_fontinfo* info, float scale_x, float scale_y, int glyph, int* width, int* height, int* xoff, int* yoff)
{
	return stbtt_GetGlyphBitmapSubpixel(info, scale_x, scale_y, 0.0f, 0.0f, glyph, width, height, xoff, yoff);
}

inline void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph)
{
	stbtt_vertex* vertices{};
	const int num_verts{ stbtt_GetGlyphShape(info, glyph, &vertices) };
	int ix0{};
	int iy0{};

	stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0, &iy0, nullptr, nullptr);

	stbtt__bitmap gbm;

	gbm.pixels = output;
	gbm.w = out_w;
	gbm.h = out_h;
	gbm.stride = out_stride;

	if (gbm.w != 0 && gbm.h != 0)
	{
		stbtt_Rasterize(&gbm, 0.35F, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1, info->userdata);
	}

	static_cast<void>(info->userdata), free(vertices);
}

inline void stbtt_MakeGlyphBitmap(const stbtt_fontinfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph)
{
	stbtt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0F,0.0F, glyph);
}

inline unsigned char* stbtt_GetCodepointBitmapSubpixel(const stbtt_fontinfo* info, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint, int* width, int* height, int* xoff, int* yoff)
{
	return stbtt_GetGlyphBitmapSubpixel(info, scale_x, scale_y,shift_x,shift_y, stbtt_FindGlyphIndex(info, codepoint), width, height, xoff, yoff);
}

void stbtt_MakeCodepointBitmapSubpixelPrefilter(const stbtt_fontinfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int prefilter_x, int prefilter_y, float *sub_x, float *sub_y, int codepoint)
{
	stbtt_MakeGlyphBitmapSubpixelPrefilter(info, output, out_w, out_h, out_stride, scale_x, scale_y, shift_x, shift_y, prefilter_x, prefilter_y, sub_x, sub_y, stbtt_FindGlyphIndex(info, codepoint));
}

inline void stbtt_MakeCodepointBitmapSubpixel(const stbtt_fontinfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint)
{
	stbtt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, shift_x, shift_y, stbtt_FindGlyphIndex(info, codepoint));
}

inline unsigned char* stbtt_GetCodepointBitmap(const stbtt_fontinfo* info, float scale_x, float scale_y, int codepoint, int* width, int* height, int* xoff, int* yoff)
{
   return stbtt_GetCodepointBitmapSubpixel(info, scale_x, scale_y, 0.0F, 0.0F, codepoint, width, height, xoff, yoff);
}

inline void stbtt_MakeCodepointBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int codepoint)
{
	stbtt_MakeCodepointBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0F, 0.0F, codepoint);
}

//////////////////////////////////////////////////////////////////////////////
//
// bitmap baking
//
// This is SUPER-CRAPPY packing to keep source code small

inline static int stbtt_BakeFontBitmap_internal(unsigned char* data, int offset, float pixel_height, unsigned char* pixels, int pw, int ph, int first_char, int num_chars, stbtt_bakedchar* chardata)
{
	int x{ 1 };
	int y{ 1 };
	int bottom_y{ 1 };

	stbtt_fontinfo f;
	f.userdata = nullptr;

	if (stbtt_InitFont(&f, data, offset) == 0)
	{
		return -1;
	}

	std::memset(pixels, 0, pw*ph); // background of 0 around pixels

	float scale{ stbtt_ScaleForPixelHeight(&f, pixel_height) };

	for (int i{}; i < num_chars; ++i)
	{
		int advance{};
		int lsb{};
		int x0{};
		int y0{};
		int x1{};
		int y1{};
		const int g{ stbtt_FindGlyphIndex(&f, first_char + i) };

		stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
		stbtt_GetGlyphBitmapBox(&f, g, scale, scale, &x0, &y0, &x1, &y1);

		const int gw{ x1 - x0 };
		const int gh{ y1 - y0 };

		if (x + gw + 1 >= pw)
		{
			y = bottom_y;
			x = 1; // advance to next row
		}

		if (y + gh + 1 >= ph) // check if it fits vertically AFTER potentially moving to next row
		{
			return -i;
		}

		assert(x + gw < pw);
		assert(y + gh < ph);
		stbtt_MakeGlyphBitmap(&f, pixels + x + y * pw, gw, gh, pw, scale, scale, g);

		chardata[i].x0 = static_cast<signed short>(x);
		chardata[i].y0 = static_cast<signed short>(y);
		chardata[i].x1 = static_cast<signed short>(x + gw);
		chardata[i].y1 = static_cast<signed short>(y + gh);
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

inline void stbtt_GetBakedQuad(const stbtt_bakedchar* chardata, int pw, int ph, int char_index, float* xpos, const float* ypos, stbtt_aligned_quad* q, int opengl_fillrule)
{
	const float d3d_bias{ (opengl_fillrule != 0) ? 0.0F : -0.5F };
	const float ipw{ 1.0F / pw };
	const float iph{ 1.0F / ph };
	const stbtt_bakedchar* b{ chardata + char_index };
	const int round_x{ static_cast<int>(std::floor((*xpos + b->xoff) + 0.5F)) };
	const int round_y{ static_cast<int>(std::floor((*ypos + b->yoff) + 0.5F)) };

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

#ifndef STB_RECT_PACK_VERSION

////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
//                                                                                //
// COMPILER WARNING ?!?!?                                                         //
//                                                                                //
//                                                                                //
// if you get a compile warning due to these symbols being defined more than      //
// once, move #include "stb_rect_pack.h" before #include "stb_truetype.h"         //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////

using stbrp_context = struct
{
	int width{};
	int height{};
	int x{};
	int y{};
	int bottom_y{};
};

using stbrp_node = struct
{
	unsigned char x{};
};

struct stbrp_rect
{
	int x{};
	int y{};
	int id{};
	int w{};
	int h{};
	int was_packed{};
};

inline static void stbrp_init_target(stbrp_context* con, int pw, int ph, stbrp_node* nodes, int num_nodes)
{
	con->width = pw;
	con->height = ph;
	con->x = 0;
	con->y = 0;
	con->bottom_y = 0;
	STBTT__NOTUSED(nodes);
	STBTT__NOTUSED(num_nodes);
}

inline static void stbrp_pack_rects(stbrp_context* con, stbrp_rect* rects, int num_rects)
{
	for (int i{}; i < num_rects; ++i)
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

	for (int i{}; i < num_rects; ++i)
	{
		rects[i].was_packed = 0;
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// bitmap baking
//
// This is SUPER-AWESOME (tm Ryan Gordon) packing using stb_rect_pack.h. If
// stb_rect_pack.h isn't available, it uses the BakeFontBitmap strategy.

inline int stbtt_PackBegin(stbtt_pack_context* spc, unsigned char* pixels, int pw, int ph, int stride_in_bytes, int padding, void* alloc_context)
{
	stbrp_context* context{ static_cast<stbrp_context*>(static_cast<void>(alloc_context), malloc(sizeof(*context))) };
	const int num_nodes{ pw - padding };
	stbrp_node* nodes{ static_cast<stbrp_node*>(static_cast<void>(alloc_context), malloc(sizeof(*nodes) * num_nodes)) };

	if (context == nullptr || nodes == nullptr)
	{
		if (context != nullptr)
		{
			static_cast<void>(alloc_context), free(context);
		}

		if (nodes != nullptr)
		{
			static_cast<void>(alloc_context), free(nodes);
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

inline void stbtt_PackSetOversampling(stbtt_pack_context* spc, unsigned int h_oversample, unsigned int v_oversample)
{
	assert(h_oversample <= STBTT_MAX_OVERSAMPLE);
	assert(v_oversample <= STBTT_MAX_OVERSAMPLE);

	if (h_oversample <= STBTT_MAX_OVERSAMPLE)
	{
		spc->h_oversample = h_oversample;
	}

	if (v_oversample <= STBTT_MAX_OVERSAMPLE)
	{
		spc->v_oversample = v_oversample;
	}
}

inline void stbtt_PackSetSkipMissingCodepoints(stbtt_pack_context* spc, int skip)
{
	spc->skip_missing = skip;
}

inline static void stbtt__h_prefilter(unsigned char* pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width)
{
	unsigned char buffer[STBTT_MAX_OVERSAMPLE];
	const int safe_w{ w - static_cast<int>(kernel_width) };

	std::memset(buffer, 0, STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze

	for (int j{}; j < h; ++j)
	{
		unsigned int total{};
		std::memset(buffer, 0, kernel_width);

		// make kernel_width a constant in common cases so compiler can optimize out the divide
		switch (kernel_width)
		{
			case 2:
			{
				for (int i{}; i <= safe_w; ++i)
				{
					total += pixels[i] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i];
					pixels[i] = static_cast<unsigned char>(total / 2);
				}
				break;
			}
			case 3:
			{
				for (int i{}; i <= safe_w; ++i)
				{
					total += pixels[i] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i];
					pixels[i] = static_cast<unsigned char>(total / 3);
				}
				break;
			}
			case 4:
			{
				for (int i{}; i <= safe_w; ++i)
				{
					total += pixels[i] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i];
					pixels[i] = static_cast<unsigned char>(total / 4);
				}
				break;
			}
			case 5:
			{
				for (int i{}; i <= safe_w; ++i)
				{
					total += pixels[i] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i];
					pixels[i] = static_cast<unsigned char>(total / 5);
				}
				break;
			}
			default:
			{
				for (int i{}; i <= safe_w; ++i)
				{
					total += pixels[i] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i];
					pixels[i] = static_cast<unsigned char>(total / kernel_width);
				}
				break;
			}
		}

		for (int i{}; i < w; ++i)
		{
			assert(pixels[i] == 0);
			total -= buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
			pixels[i] = static_cast<unsigned char>(total / kernel_width);
		}

		pixels += stride_in_bytes;
	}
}

inline static void stbtt__v_prefilter(unsigned char* pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width)
{
	unsigned char buffer[STBTT_MAX_OVERSAMPLE];
	const int safe_h{ h - static_cast<int>(kernel_width) };

	std::memset(buffer, 0, STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze

	for (int j{}; j < w; ++j)
	{
		unsigned int total{};
		std::memset(buffer, 0, kernel_width);

		// make kernel_width a constant in common cases so compiler can optimize out the divide
		switch (kernel_width)
		{
			case 2:
			{
				for (int i{}; i <= safe_h; ++i)
				{
					total += pixels[i * stride_in_bytes] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i * stride_in_bytes];
					pixels[i * stride_in_bytes] = static_cast<unsigned char>(total / 2);
				}
				break;
			}
			case 3:
			{
				for (int i{}; i <= safe_h; ++i)
				{
					total += pixels[i * stride_in_bytes] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i * stride_in_bytes];
					pixels[i * stride_in_bytes] = static_cast<unsigned char>(total / 3);
				}
				break;
			}
			case 4:
			{
				for (int i{}; i <= safe_h; ++i)
				{
					total += pixels[i * stride_in_bytes] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i * stride_in_bytes];
					pixels[i * stride_in_bytes] = static_cast<unsigned char>(total / 4);
				}
				break;
			}
			case 5:
			{
				for (int i{}; i <= safe_h; ++i) {
					total += pixels[i * stride_in_bytes] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i * stride_in_bytes];
					pixels[i * stride_in_bytes] = static_cast<unsigned char>(total / 5);
				}
				break;
			}
			default:
			{
				for (int i{}; i <= safe_h; ++i)
				{
					total += pixels[i * stride_in_bytes] - buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
					buffer[(i + kernel_width) & (STBTT_MAX_OVERSAMPLE - 1)] = pixels[i * stride_in_bytes];
					pixels[i * stride_in_bytes] = static_cast<unsigned char>(total / kernel_width);
				}
				break;
			}
		}

		for (int i{}; i < h; ++i)
		{
			assert(pixels[i*stride_in_bytes] == 0);
			total -= buffer[i & (STBTT_MAX_OVERSAMPLE - 1)];
			pixels[i*stride_in_bytes] = static_cast<unsigned char>(total / kernel_width);
		}

		pixels += 1;
	}
}

inline static float stbtt__oversample_shift(int oversample)
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
inline int stbtt_PackFontRangesGatherRects(stbtt_pack_context* spc, const stbtt_fontinfo* info, stbtt_pack_range* ranges, int num_ranges, stbrp_rect* rects)
{
	int k{};

	for (int i{}; i < num_ranges; ++i)
	{
		const float fh{ ranges[i].font_size };
		const float scale{ fh > 0.0F ? stbtt_ScaleForPixelHeight(info, fh) : stbtt_ScaleForMappingEmToPixels(info, -fh) };

		ranges[i].h_oversample = static_cast<unsigned char>(spc->h_oversample);
		ranges[i].v_oversample = static_cast<unsigned char>(spc->v_oversample);

		for (int j{}; j < ranges[i].num_chars; ++j)
		{
			int x0{};
			int y0{};
			int x1{};
			int y1{};

			const int codepoint{ ranges[i].array_of_unicode_codepoints == nullptr ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j] };
			const int glyph{ stbtt_FindGlyphIndex(info, codepoint) };

			if (glyph == 0 && spc->skip_missing != 0)
			{
				rects[k].w = 0;
				rects[k].h = 0;
			}
			else
			{
				stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale * spc->h_oversample, scale * spc->v_oversample, 0,0, &x0, &y0, &x1, &y1);
				rects[k].w = static_cast<int>(x1 - x0 + spc->padding + spc->h_oversample-1);
				rects[k].h = static_cast<int>(y1 - y0 + spc->padding + spc->v_oversample-1);
			}

			++k;
		}
	}

	return k;
}

inline void stbtt_MakeGlyphBitmapSubpixelPrefilter(const stbtt_fontinfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int prefilter_x, int prefilter_y, float* sub_x, float* sub_y, int glyph)
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
inline int stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context* spc, const stbtt_fontinfo* info, stbtt_pack_range* ranges, int num_ranges, stbrp_rect* rects)
{
	int k{};
	int return_value{ 1 };

	// save current values
	const int old_h_over{ static_cast<int>(spc->h_oversample) };
	const int old_v_over{ static_cast<int>(spc->v_oversample) };

	for (int i{}; i < num_ranges; ++i)
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

		for (int j{}; j < ranges[i].num_chars; ++j)
		{
			stbrp_rect* r{ &rects[k] };

			if (r->was_packed != 0 && r->w != 0 && r->h != 0)
			{
				stbtt_packedchar* bc{ &ranges[i].chardata_for_range[j] };
				int advance{};
				int lsb{};
				int x0{};
				int y0{};
				int x1{};
				int y1{};
				int codepoint{ ranges[i].array_of_unicode_codepoints == nullptr ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j] };
				int glyph{ stbtt_FindGlyphIndex(info, codepoint) };
				int pad{ static_cast<int>(spc->padding) };

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

				bc->x0 = static_cast<signed short>(r->x);
				bc->y0 = static_cast<signed short>(r->y);
				bc->x1 = static_cast<signed short>(r->x + r->w);
				bc->y1 = static_cast<signed short>(r->y + r->h);
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

inline void stbtt_PackFontRangesPackRects(stbtt_pack_context* spc, stbrp_rect* rects, int num_rects)
{
	stbrp_pack_rects(static_cast<stbrp_context*>(spc->pack_info), rects, num_rects);
}

inline int stbtt_PackFontRanges(stbtt_pack_context* spc, const unsigned char* fontdata, int font_index, stbtt_pack_range* ranges, int num_ranges)
{
	stbtt_fontinfo info{};
	int n{};
	int return_value{ 1 };
	//stbrp_context *context = (stbrp_context *) spc->pack_info;
	stbrp_rect* rects{};

	// flag all characters as NOT packed
	for (int i{}; i < num_ranges; ++i)
	{
		for (int j{}; j < ranges[i].num_chars; ++j)
		{
			ranges[i].chardata_for_range[j].x0 = 0;
			ranges[i].chardata_for_range[j].y0 = 0;
			ranges[i].chardata_for_range[j].x1 = 0;
			ranges[i].chardata_for_range[j].y1 = 0;
		}
	}

	for (int i{}; i < num_ranges; ++i)
	{
		n += ranges[i].num_chars;
	}

	rects = static_cast<stbrp_rect*>(static_cast<void>(spc->user_allocator_context), malloc(sizeof(*rects) * n));

	if (rects == nullptr)
	{
		return 0;
	}

	info.userdata = spc->user_allocator_context;
	stbtt_InitFont(&info, fontdata, stbtt_GetFontOffsetForIndex(fontdata,font_index));
	n = stbtt_PackFontRangesGatherRects(spc, &info, ranges, num_ranges, rects);
	stbtt_PackFontRangesPackRects(spc, rects, n);
	return_value = stbtt_PackFontRangesRenderIntoRects(spc, &info, ranges, num_ranges, rects);

	static_cast<void>(spc->user_allocator_context), free(rects);
	return return_value;
}

inline int stbtt_PackFontRange(stbtt_pack_context* spc, const unsigned char* fontdata, int font_index, float font_size, int first_unicode_codepoint_in_range, int num_chars_in_range, stbtt_packedchar* chardata_for_range)
{
	stbtt_pack_range range{};
	range.first_unicode_codepoint_in_range = first_unicode_codepoint_in_range;
	range.array_of_unicode_codepoints = nullptr;
	range.num_chars = num_chars_in_range;
	range.chardata_for_range = chardata_for_range;
	range.font_size = font_size;
	return stbtt_PackFontRanges(spc, fontdata, font_index, &range, 1);
}

inline void stbtt_GetScaledFontVMetrics(const unsigned char* fontdata, int index, float size, float* ascent, float* descent, float* lineGap)
{
	stbtt_fontinfo info{};
	stbtt_InitFont(&info, fontdata, stbtt_GetFontOffsetForIndex(fontdata, index));

	const float scale{ (size > 0) ? stbtt_ScaleForPixelHeight(&info, size) : stbtt_ScaleForMappingEmToPixels(&info, -size) };

	int i_ascent{};
	int i_descent{};
	int i_lineGap{};

	stbtt_GetFontVMetrics(&info, &i_ascent, &i_descent, &i_lineGap);
	*ascent  = static_cast<float>(i_ascent) * scale;
	*descent = static_cast<float>(i_descent) * scale;
	*lineGap = static_cast<float>(i_lineGap) * scale;
}

inline void stbtt_GetPackedQuad(const stbtt_packedchar* chardata, int pw, int ph, int char_index, float* xpos, const float* ypos, stbtt_aligned_quad* q, int align_to_integer)
{
	float ipw{ 1.0F / pw };
	float iph{ 1.0F / ph };
	const stbtt_packedchar* b{ chardata + char_index };

	if (align_to_integer != 0)
	{
		auto x{ static_cast<float>(static_cast<int>(std::floor((*xpos + b->xoff) + 0.5f))) };
		auto y{ static_cast<float>(static_cast<int>(std::floor((*ypos + b->yoff) + 0.5f))) };

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

inline static int stbtt__ray_intersect_bezier(const float orig[2], const float ray[2], const float q0[2], const float q1[2], const float q2[2], float hits[2][2])
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
	int num_s{};

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

inline static int equal(const float* a, const float* b)
{
	return (a[0] == b[0] && a[1] == b[1]); //-V550
}

inline static int stbtt__compute_crossings_x(float x, float y, int nverts, stbtt_vertex* verts)
{
	std::vector<float> orig{x, y};
	std::vector<float> ray{ 1.0F, 0.0F };
	int winding{};

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
	for (int i{}; i < nverts; ++i)
	{
		if (verts[i].type == STBTT_vline)
		{
			const int x0{ static_cast<int>(verts[i - 1].x) };
			const int y0{ static_cast<int>(verts[i - 1].y) };
			const int x1{ static_cast<int>(verts[i].x) };
			const int y1{ static_cast<int>(verts[i].y) };

			if (y > (std::min)(y0, y1) && y < (std::max)(y0, y1) && x > (std::min)(x0, x1))
			{
				const float x_inter{ (y - y0) / (y1 - y0) * (x1 - x0) + x0 };

				if (x_inter < x)
				{
					winding += (y0 < y1) ? 1 : -1;
				}
			}
		}

		if (verts[i].type == STBTT_vcurve)
		{
			int x0{ static_cast<int>(verts[i - 1].x) };
			int y0{ static_cast<int>(verts[i - 1].y) };
			int x1{ static_cast<int>(verts[i].cx) };
			int y1{ static_cast<int>(verts[i].cy) };
			int x2{ static_cast<int>(verts[i].x) };
			int y2{ static_cast<int>(verts[i].y) };
			const int ax{ (std::min)(x0, (std::min)(x1, x2)) };
			const int ay{ (std::min)(y0, (std::min)(y1, y2)) };
			const int by{ (std::max)(y0, (std::max)(y1, y2)) };

			if (y > ay && y < by && x > ax)
			{
				const float q0[2]{ static_cast<float>(x0), static_cast<float>(y0) };
				const float q1[2]{ static_cast<float>(x1), static_cast<float>(y1) };
				const float q2[2]{ static_cast<float>(x2), static_cast<float>(y2) };
				float hits[2][2]{};

				if (equal(q0, q1) != 0 || equal(q1, q2) != 0)
				{
					x0 = static_cast<int>(verts[i-1].x);
					y0 = static_cast<int>(verts[i-1].y);
					x1 = static_cast<int>(verts[i].x);
					y1 = static_cast<int>(verts[i].y);

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
					const int num_hits{ stbtt__ray_intersect_bezier(orig.data(), ray.data(), q0, q1, q2, hits) };

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

inline static float stbtt__cuberoot(float x)
{
	if (x < 0.0F)
	{
		return -std::powf(-x, 1.0F / 3.0F);
	}

	return std::powf(x, 1.0F / 3.0F);
}

// x^3 + c*x^2 + b*x + a = 0
inline static int stbtt__solve_cubic(float a, float b, float c, float* r)
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

		u = stbtt__cuberoot(u);
		v = stbtt__cuberoot(v);
		r[0] = s + u + v;

		return 1;
	}

	const float u{ std::sqrt(-p / 3.0F) };
	const float v{ std::acosf(-std::sqrtf(-27.0F / p3) * q / 2.0F) / 3.0F }; // p3 must be negative, since d is negative
	const float m{ std::cosf(v) };
	const float n{ std::cosf(v - 3.141592026F / 2.0F) * 1.732050808F };

	r[0] = s + u * 2.0F * m;
	r[1] = s - u * (m + n);
	r[2] = s - u * (m - n);

	//STBTT_assert( STBTT_fabs(((r[0]+a)*r[0]+b)*r[0]+c) < 0.05f);  // these asserts may not be safe at all scales, though they're in bezier t parameter units so maybe?
	//STBTT_assert( STBTT_fabs(((r[1]+a)*r[1]+b)*r[1]+c) < 0.05f);
	//STBTT_assert( STBTT_fabs(((r[2]+a)*r[2]+b)*r[2]+c) < 0.05f);

	return 3;
}

inline unsigned char* stbtt_GetGlyphSDF(const stbtt_fontinfo *info, float scale, int glyph, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff)
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

	int ix0{};
	int iy0{};
	int ix1{};
	int iy1{};

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

	const int w{ (ix1 - ix0) };
	const int h{ (iy1 - iy0) };

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
		const int num_verts{ stbtt_GetGlyphShape(info, glyph, &verts) };
		data = static_cast<unsigned char*>(static_cast<void>(info->userdata), malloc(w * h));
		float* precompute{ static_cast<float*>(static_cast<void>(info->userdata), malloc(num_verts * sizeof(float))) };

		for (int i{}, j = num_verts - 1; i < num_verts; j = i++)
		{
			if (verts[i].type == STBTT_vline)
			{
				const float x0{ verts[i].x * scale_x };
				const float y0{ verts[i].y * scale_y };
				const float x1{ verts[j].x * scale_x };
				const float y1{ verts[j].y * scale_y };
				const float dist{ std::sqrtf((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) };
				precompute[i] = (dist == 0.0F) ? 0.0F : 1.0F / dist; //-V550
			}
			else if (verts[i].type == STBTT_vcurve)
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

		for (int y{ iy0 }; y < iy1; ++y)
		{
			for (int x{ ix0 }; x < ix1; ++x)
			{
				float min_dist{ 999999.0F };
				const float sx{ static_cast<float>(x) + 0.5F };
				const float sy{ static_cast<float>(y) + 0.5F };
				const float x_gspace{ (sx / scale_x) };
				const float y_gspace{ (sy / scale_y) };

				const int winding{ stbtt__compute_crossings_x(x_gspace, y_gspace, num_verts, verts) }; // @OPTIMIZE: this could just be a rasterization, but needs to be line vs. non-tesselated curves so a new path

				for (int i{}; i < num_verts; ++i)
				{
					const float x0{ verts[i].x * scale_x };
					const float y0{ verts[i].y * scale_y };

					// check against every point here rather than inside line/curve primitives -- @TODO: wrong if multiple 'moves' in a row produce a garbage point, and given culling, probably more efficient to do within line/curve
					float dist2{ (x0 - sx) * (x0 - sx) + (y0 - sy) * (y0 - sy) };

					if (dist2 < min_dist * min_dist)
					{
						min_dist = std::sqrtf(dist2);
					}

					if (verts[i].type == STBTT_vline)
					{
						const float x1{ verts[i - 1].x * scale_x };
						const float y1{ verts[i - 1].y * scale_y };

						// coarse culling against bbox
						//if (sx > STBTT_min(x0,x1)-min_dist && sx < STBTT_max(x0,x1)+min_dist &&
						//    sy > STBTT_min(y0,y1)-min_dist && sy < STBTT_max(y0,y1)+min_dist)
						const float dist{ static_cast<float>(std::fabs((x1 - x0) * (y0 - sy) - (y1 - y0) * (x0 - sx))) * precompute[i] };
						assert(i != 0);

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
					else if (verts[i].type == STBTT_vcurve)
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
							int num{};
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

				float val{ onedge_value + pixel_dist_scale * min_dist };

				if (val < 0.0F)
				{
					val = 0.0F;
				}
				else if (val > 255.0F)
				{
					val = 255.0F;
				}

				data[(y - iy0) * w + (x - ix0)] = static_cast<unsigned char>(val);
			}
		}

		static_cast<void>(info->userdata), free(precompute);
		static_cast<void>(info->userdata), free(verts);
	}

	return data;
}

inline unsigned char* stbtt_GetCodepointSDF(const stbtt_fontinfo* info, float scale, int codepoint, int padding, unsigned char onedge_value, float pixel_dist_scale, int* width, int* height, int* xoff, int* yoff)
{
	return stbtt_GetGlyphSDF(info, scale, stbtt_FindGlyphIndex(info, codepoint), padding, onedge_value, pixel_dist_scale, width, height, xoff, yoff);
}

inline void stbtt_FreeSDF(unsigned char* bitmap, void* userdata)
{
	static_cast<void>(userdata), free(bitmap);
}

//////////////////////////////////////////////////////////////////////////////
//
// font name matching -- recommended not to use this
//

// check if a utf8 string contains a prefix which is the utf16 string; if so return length of matching utf8 string
inline static signed int stbtt__CompareUTF8toUTF16_bigendian_prefix(const unsigned char* s1, signed int len1, const unsigned char* s2, signed int len2)
{
	signed int i{};

	// convert utf16 to utf8 and compare the results while converting
	while (len2 != 0)
	{
		const unsigned short ch{ static_cast<unsigned short>(s2[0] * 256 + s2[1]) };

		if (ch < 0x80)
		{
			if (i >= len1)
			{
				return -1;
			}

			if (s1[i++] != ch)
			{
				return -1;
			}
		}
		else if (ch < 0x800)
		{
			if (i + 1 >= len1)
			{
				return -1;
			}

			if (s1[i++] != 0xc0 + (ch >> 6))
			{
				return -1;
			}

			if (s1[i++] != 0x80 + (ch & 0x3f))
			{
				return -1;
			}
		}
		else if (ch >= 0xd800 && ch < 0xdc00)
		{
			const unsigned short ch2{ static_cast<unsigned short>(s2[2] * 256 + s2[3]) };

			if (i + 3 >= len1)
			{
				return -1;
			}

			const unsigned int c{ static_cast<unsigned int>(((ch - 0xd800) << 10) + (ch2 - 0xdc00) + 0x10000) };

			if (s1[i++] != 0xf0 + (c >> 18))
			{
				return -1;
			}

			if (s1[i++] != 0x80 + ((c >> 12) & 0x3f))
			{
				return -1;
			}

			if (s1[i++] != 0x80 + ((c >> 6) & 0x3f))
			{
				return -1;
			}

			if (s1[i++] != 0x80 + ((c) & 0x3f))
			{
				return -1;
			}

			s2 += 2; // plus another 2 below
			len2 -= 2;
		}
		else if (ch >= 0xdc00 && ch < 0xe000)
		{
			return -1;
		}
		else
		{
			if (i + 2 >= len1)
			{
				return -1;
			}

			if (s1[i++] != 0xe0 + (ch >> 12))
			{
				return -1;
			}

			if (s1[i++] != 0x80 + ((ch >> 6) & 0x3f))
			{
				return -1;
			}

			if (s1[i++] != 0x80 + ((ch) & 0x3f))
			{
				return -1;
			}
		}

		s2 += 2;
		len2 -= 2;
	}

	return i;
}

inline static int stbtt_CompareUTF8toUTF16_bigendian_internal(char* s1, int len1, char* s2, int len2)
{
	return len1 == stbtt__CompareUTF8toUTF16_bigendian_prefix(reinterpret_cast<unsigned char*>(s1), len1, reinterpret_cast<unsigned char*>(s2), len2);
}

// returns results in whatever encoding you request... but note that 2-byte encodings
// will be BIG-ENDIAN... use stbtt_CompareUTF8toUTF16_bigendian() to compare
inline const char *stbtt_GetFontNameString(const stbtt_fontinfo* font, int* length, int platformID, int encodingID, int languageID, int nameID)
{
	unsigned char* fc{ font->data };
	const unsigned int offset{ static_cast<unsigned int>(font->fontstart) };
	const unsigned int nm{ stbtt__find_table(fc, offset, "name") };

	if (nm == 0)
	{
		return nullptr;
	}

	const signed int count{ ttUSHORT(fc + nm + 2) };
	const signed int stringOffset{ static_cast<signed int>(nm + ttUSHORT(fc + nm + 4)) };

	for (signed int i{}; i < count; ++i)
	{
		const unsigned int loc{ nm + 6 + 12 * i };

		if (platformID == ttUSHORT(fc + loc + 0) && encodingID == ttUSHORT(fc + loc + 2) && languageID == ttUSHORT(fc + loc + 4) && nameID == ttUSHORT(fc + loc + 6))
		{
			*length = ttUSHORT(fc + loc + 8);
			return reinterpret_cast<const char*>(fc + stringOffset + ttUSHORT(fc + loc + 10));
		}
	}

	return nullptr;
}

inline static int stbtt__matchpair(unsigned char* fc, unsigned int nm, unsigned char* name, signed int nlen, signed int target_id, signed int next_id)
{
	const signed int count{ ttUSHORT(fc + nm + 2) };
	const signed int stringOffset{ static_cast<signed int>(nm + ttUSHORT(fc + nm + 4)) };

	for (signed int i{}; i < count; ++i)
	{
		const unsigned int loc{ nm + 6 + 12 * i };
		const signed int id{ ttUSHORT(fc + loc + 6) };

		if (id == target_id)
		{
			// find the encoding
			const signed int platform{ ttUSHORT(fc + loc + 0) };
			const signed int encoding{ ttUSHORT(fc + loc + 2) };
			const signed int language{ ttUSHORT(fc + loc + 4) };

			// is this a Unicode encoding?
			if (platform == 0 || (platform == 3 && encoding == 1) || (platform == 3 && encoding == 10))
			{
				signed int slen{ ttUSHORT(fc + loc + 8) };
				signed int off{ ttUSHORT(fc + loc + 10) };

				// check if there's a prefix match
				signed int matchlen{ stbtt__CompareUTF8toUTF16_bigendian_prefix(name, nlen, fc + stringOffset + off, slen) };

				if (matchlen >= 0)
				{
					// check for target_id+1 immediately following, with same encoding & language
					if (i + 1 < count && ttUSHORT(fc + loc + 12 + 6) == next_id && ttUSHORT(fc+loc+12) == platform && ttUSHORT(fc + loc + 12 + 2) == encoding && ttUSHORT(fc + loc + 12 + 4) == language)
					{
						slen = ttUSHORT(fc + loc + 12 + 8);
						off = ttUSHORT(fc + loc + 12 + 10);

						if (slen == 0)
						{
							if (matchlen == nlen)
							{
								return 1;
							}
						}
						else if (matchlen < nlen && name[matchlen] == ' ')
						{
							++matchlen;

							if (stbtt_CompareUTF8toUTF16_bigendian_internal(reinterpret_cast<char*>(name + matchlen), nlen - matchlen, reinterpret_cast<char*>(fc + stringOffset + off), slen) != 0)
							{
								return 1;
							}
						}
					}
					else
					{
						// if nothing immediately following
						if (matchlen == nlen)
						{
							return 1;
						}
					}
				}
			}
			// @TODO handle other encodings
		}
	}

	return 0;
}

inline static int stbtt__matches(unsigned char* fc, unsigned int offset, unsigned char* name, signed int flags)
{
	if (stbtt__isfont(fc + offset) == 0)
	{
		return 0;
	}

	// check italics/bold/underline flags in macStyle...
	if (flags != 0)
	{
		const unsigned int hd{ stbtt__find_table(fc, offset, "head") };

		if ((ttUSHORT(fc + hd + 44) & 7) != (flags & 7))
		{
			return 0;
		}
	}

	const unsigned int nm{ stbtt__find_table(fc, offset, "name") };

	if (nm == 0)
	{
		return 0;
	}

	const auto nlen{ static_cast<signed int>(strlen(reinterpret_cast<char*>(name))) };

	if (flags != 0)
	{
		// if we checked the macStyle flags, then just check the family and ignore the subfamily
		if (stbtt__matchpair(fc, nm, name, nlen, 16, -1) != 0)
		{
			return 1;
		}

		if (stbtt__matchpair(fc, nm, name, nlen, 1, -1) != 0)
		{
			return 1;
		}

		if (stbtt__matchpair(fc, nm, name, nlen, 3, -1) != 0)
		{
			return 1;
		}

	}
	else
	{
		if (stbtt__matchpair(fc, nm, name, nlen, 16, 17) != 0)
		{
			return 1;
		}

		if (stbtt__matchpair(fc, nm, name, nlen, 1, 2) != 0)
		{
			return 1;
		}

		if (stbtt__matchpair(fc, nm, name, nlen, 3, -1) != 0)
		{
			return 1;
		}
	}

	return 0;
}

inline static int stbtt_FindMatchingFont_internal(unsigned char* font_collection, char* name_utf8, signed int flags)
{
	for (int i{};; ++i)
	{
		const signed int off{ stbtt_GetFontOffsetForIndex(font_collection, i) };

		if (off < 0)
		{
			return off;
		}

		if (stbtt__matches(reinterpret_cast<unsigned char*>(font_collection), off, reinterpret_cast<unsigned char*>(name_utf8), flags) != 0)
		{
			return off;
		}
	}
}

#if defined(__GNUC__) || defined(__clang__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

inline int stbtt_BakeFontBitmap(const unsigned char* data, int offset, float pixel_height, unsigned char* pixels, int pw, int ph, int first_char, int num_chars, stbtt_bakedchar* chardata)
{
	return stbtt_BakeFontBitmap_internal(const_cast<unsigned char*>(data), offset, pixel_height, pixels, pw, ph, first_char, num_chars, chardata);
}

inline int stbtt_GetFontOffsetForIndex(const unsigned char* data, int index)
{
	return stbtt_GetFontOffsetForIndex_internal(const_cast<unsigned char*>(data), index);
}

inline int stbtt_GetNumberOfFonts(const unsigned char* data)
{
	return stbtt_GetNumberOfFonts_internal(const_cast<unsigned char*>(data));
}

inline int stbtt_InitFont(stbtt_fontinfo* info, const unsigned char* data, int offset)
{
	return stbtt_InitFont_internal(info, const_cast<unsigned char*>(data), offset);
}

inline int stbtt_FindMatchingFont(const unsigned char* fontdata, const char* name, int flags)
{
	return stbtt_FindMatchingFont_internal(const_cast<unsigned char*>(fontdata), const_cast<char*>(name), flags);
}

inline int stbtt_CompareUTF8toUTF16_bigendian(const char* s1, int len1, const char* s2, int len2)
{
	return stbtt_CompareUTF8toUTF16_bigendian_internal(const_cast<char*>(s1), len1, const_cast<char*>(s2), len2);
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
