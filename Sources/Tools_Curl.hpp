/*
***************************************************
*                                                 *
* Tools_Curl.hpp                                  *
*                                                 *
* (c) 2017, 2018, 2019 Stefan Kubsch              *
***************************************************
*/

#pragma once

#define CURL_STATICLIB
#define FMT_HEADER_ONLY

#include <string>
#include <cstdio>
#include <curl.h>
#include "fmt/format.h"

#include "Tools_Console.hpp"
#include "Tools_ErrorHandling.hpp"

namespace Tools_Curl
{


	void Init();
	size_t WriteData(void* Ptr, size_t Size, size_t Mem, FILE* Stream);
	bool CheckInternetConnection();
	void FetchFileFromURL(const std::string& URL, const std::string& Filename);

	//
	// Functions
	//

	inline void Init()
	{
		Tools_Console::DisplayText(BRIGHT_MAGENTA, "\nInit curl...");
		curl_global_init(CURL_GLOBAL_DEFAULT) != 0 ? Tools_ErrorHandling::DisplayError("curl init failed!") : Tools_ErrorHandling::DisplayOK();
	}

	inline size_t WriteData(void* Ptr, const size_t Size, const size_t Mem, FILE* Stream)
	{
		return fwrite(Ptr, Size, Mem, Stream);
	}

	inline bool CheckInternetConnection()
	{
		Tools_Console::DisplayText(BRIGHT_MAGENTA, "\nChecking internet connection...");

		bool Result{};

		if (CURL* CurlInstance{ curl_easy_init() })
		{
			curl_easy_setopt(CurlInstance, CURLOPT_URL, "www.google.com");
			curl_easy_setopt(CurlInstance, CURLOPT_NOBODY, 1);
			curl_easy_perform(CurlInstance) != CURLE_OK ? Tools_Console::DisplayText(BRIGHT_RED, "No internet connection found.") : (Result = true, Tools_ErrorHandling::DisplayOK());
			curl_easy_cleanup(CurlInstance);
		}

		return Result;
	}

	inline void FetchFileFromURL(const std::string& URL, const std::string& Filename)
	{
		if (CURL* CurlInstance{ curl_easy_init() }; CurlInstance != nullptr && CheckInternetConnection())
		{
			Tools_Console::DisplayText(BRIGHT_MAGENTA, fmt::format("Downloading file from URL via curl...\nSource: {}...", URL));

			FILE* File{ fopen(Filename.c_str(), "w") };
			curl_easy_setopt(CurlInstance, CURLOPT_URL, URL.c_str());
			curl_easy_setopt(CurlInstance, CURLOPT_WRITEFUNCTION, WriteData);
			curl_easy_setopt(CurlInstance, CURLOPT_WRITEDATA, File);
			curl_easy_perform(CurlInstance);
			curl_easy_cleanup(CurlInstance);

			if (File != nullptr)
			{
				fclose(File);
			}

			Tools_ErrorHandling::DisplayOK();
		}
	}


} // namespace Tools_Curl
