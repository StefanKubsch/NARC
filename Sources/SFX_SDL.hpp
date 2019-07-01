/*
******************************************************************
*                                                                *
* SFX_SDL.hpp                                                    *
*                                                                *
* (c) 2017, 2018, 2019 Stefan Kubsch                             *
******************************************************************
*/

#pragma once

#include <string>
#include <SDL_mixer.h>

#include "Tools_ErrorHandling.hpp"

namespace SFX_SDL
{


	void Init();
	Mix_Chunk* LoadAudioFile(const std::string& FileName);
	void StopAudio();

	//
	// Functions
	//

	inline void Init()
	{
		if (const std::string INIFile{ "./DATA/GameConfig/AudioConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			const std::int_fast32_t Samplerate{ lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "Samplerate") };
			const std::int_fast32_t StereoChannels{ lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "StereoChannels") };
			const std::int_fast32_t ChunkSize{ lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "ChunkSize") };
			const std::int_fast32_t MaxChannels{ lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "GENERAL", "MaxChannels") };

			NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, "Initialising SDL Mixer system...");

			if (constexpr std::int_fast32_t Flags{ MIX_INIT_OGG }; (Mix_Init(Flags) & Flags) != Flags)
			{
				NARCLog.AddEntry(lwmf::LogLevel::Critical, __FILENAME__, "SDL audio support init failed: " + std::string(SDL_GetError()));
			}

			if (Mix_OpenAudio(Samplerate, MIX_DEFAULT_FORMAT, StereoChannels, ChunkSize) != 0)
			{
				NARCLog.AddEntry(lwmf::LogLevel::Critical, __FILENAME__, "SDL mixer init failed: " + std::string(SDL_GetError()));
			}
			else
			{
				Mix_AllocateChannels(MaxChannels);
				Mix_Volume(-1, MIX_MAX_VOLUME);
			}
		}
	}

	inline Mix_Chunk* LoadAudioFile(const std::string& FileName)
	{
		Mix_Chunk* TempAudio{};

		if (Tools_ErrorHandling::CheckFileExistence(FileName, StopOnError))
		{
			TempAudio = Mix_LoadWAV(FileName.c_str());
		}

		return TempAudio;
	}

	inline void StopAudio()
	{
		Mix_HaltChannel(-1);
	}


} // namespace SFX_SDL
