/*
***************************************************
*                                                 *
* Game_GlobalDefinitions.hpp                      *
*                                                 *
* (c) 2017, 2018, 2019 Stefan Kubsch              *
***************************************************
*/

#pragma once

#include <cstdint>
#include <random>

// Setting planes/viewport for raycaster
inline lwmf::FloatPointStruct Plane{};
inline lwmf::FloatPointStruct PlaneStartValue{};

// Settings for looking up/down
inline float VerticalLookCamera{};
inline float VerticalLookUpLimit{};
inline float VerticalLookDownLimit{};
inline float VerticalLookStep{};
inline float FogOfWarDistance{};
inline std::int_fast32_t VerticalLook{};

// Current Level + Number of Levels
inline std::int_fast32_t SelectedLevel{};
inline std::int_fast32_t NumberOfLevels{};

// Options for Renderer
inline bool VSync{};

// Size of textures (width and height)
inline std::int_fast32_t TextureSize{};
inline std::int_fast32_t EntitySize{};

// Factor for bitwise texture operations is calculated correctly dependent on TextureSize in "Game_Config.hpp"
inline std::int_fast32_t TextureSizeBitwiseAnd{};

// Set factor for bitshifting from TextureSize
// 7 for 128x128, 8 for 256x256, 9 for 512x512, 10 for 1024x1024
// is calculated in "Game_Config.hpp" dependent on given TextureSize
inline std::int_fast32_t TextureSizeShiftFactor{};

// Variables for fixed timestep gameloop
inline std::uint_fast32_t LengthOfFrame{};
inline std::uint_fast32_t FrameLock{};
inline bool GamePausedFlag{};
inline bool GameControllerFlag{};
inline bool QuitGameFlag{};
inline bool HUDEnabled{ true };

// Random Number Generator
// C++ 11 Mersenne-Twister-Engine
inline std::random_device Seed;
inline std::mt19937 RNG(Seed());