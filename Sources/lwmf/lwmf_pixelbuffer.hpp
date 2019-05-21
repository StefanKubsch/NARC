/*
****************************************************
*                                                  *
* lwmf_pixelbuffer - lightweight media framework   *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <GL/gl.h>

#include "lwmf_general.hpp"
#include "lwmf_openglloader.hpp"

namespace lwmf
{


	void RenderPixelBuffer();
	void ClearPixelBuffer(std::int_fast32_t Color);

	//
	// Variables and constants
	//

	inline std::vector<std::int_fast32_t> PixelBuffer;

	//
	// Functions
	//

	inline void RenderPixelBuffer()
	{
		switch (FullscreenFlag)
		{
			case 1:
			{
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ViewportWidth, ViewportHeight, GL_RGBA, GL_UNSIGNED_BYTE, PixelBuffer.data());
				break;
			}
			case 0:
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ViewportWidth, ViewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, PixelBuffer.data());
				break;
			}
			default: {};
		}

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	inline void ClearPixelBuffer(const std::int_fast32_t Color)
	{
		std::fill(PixelBuffer.begin(), PixelBuffer.end(), Color);
	}


} // namespace lwmf