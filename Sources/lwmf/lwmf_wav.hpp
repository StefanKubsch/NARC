/*
****************************************************
*                                                  *
* lwmf_wav - lightweight media framework           *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <Windows.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include <string>
#include <vector>
#include <fstream>

#pragma comment(lib, "Winmm.lib")

#include "lwm_logging.hpp"

namespace lwmf
{


	class Wav
	{
	public:
		Wav(const std::string& Filename);
		~Wav();
		void Play(bool Async = true);

	private:
		std::vector<char> Buffer;
	};

	inline Wav::Wav(const std::string& Filename)
	{
		std::ifstream File(Filename, std::ios::in | std::ios::binary);

		if (File.fail())
		{
			LWMFSystemLog.AddEntry(LogLevel::Error, __FILENAME__, "Error loading " + Filename + "!");
		}
		else
		{
			File.seekg(0, std::ios::end);

			const std::streamsize Length{ File.tellg() };
			Buffer.resize(Length);

			File.seekg(0, std::ios::beg);
			File.read(Buffer.data(), Length);
		}
	}

	inline Wav::~Wav()
	{
		PlaySound(nullptr, nullptr, 0);
	}

	inline void Wav::Play(const bool Async)
	{
		Async ?	PlaySound(Buffer.data(), WindowInstance, SND_MEMORY | SND_ASYNC) : PlaySound(Buffer.data(), WindowInstance, SND_MEMORY);
	}


} // namespace lwmf