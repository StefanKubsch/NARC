/*
******************************************************
*                                                    *
* lwmf_general - lightweight media framework         *
*                                                    *
* (C) 2019 - present by Stefan Kubsch                *
*                                                    *
******************************************************
*/

#pragma once

#include "Windows.h"
#include <cstdint>
#include <vector>

namespace lwmf
{


	//
	// Variables and constants
	//

	struct TextureStruct final
	{
		std::vector<std::int_fast32_t> Texture;
		std::int_fast32_t Width{};
		std::int_fast32_t Height{};
	};

	struct IntPointStruct final
	{
		std::int_fast32_t X{};
		std::int_fast32_t Y{};
	};

	struct FloatPointStruct final
	{
		float X{};
		float Y{};
	};

	struct IntRectStruct final
	{
		std::int_fast32_t X{};
		std::int_fast32_t Y{};
		std::int_fast32_t Width{};
		std::int_fast32_t Height{};
	};

	inline std::int_fast32_t FullscreenFlag{};

	inline std::int_fast32_t ViewportWidth{};
	inline std::int_fast32_t ViewportHeight{};
	inline std::int_fast32_t ViewportWidthMid{};
	inline std::int_fast32_t ViewportHeightMid{};

	inline HDC WindowHandle;
	inline HWND MainWindow;


} // namespace lwmf