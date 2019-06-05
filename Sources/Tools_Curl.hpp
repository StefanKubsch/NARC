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
		lwmf::AddLogEntry("\nInit curl...");

		if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0)
		{
			lwmf::LogErrorAndThrowException("curl init failed!");
		}
	}

	inline size_t WriteData(void* Ptr, const size_t Size, const size_t Mem, FILE* Stream)
	{
		return fwrite(Ptr, Size, Mem, Stream);
	}

	inline bool CheckInternetConnection()
	{
		lwmf::AddLogEntry("\nChecking internet connection...");

		bool Result{};

		if (CURL* CurlInstance{ curl_easy_init() })
		{
			curl_easy_setopt(CurlInstance, CURLOPT_URL, "www.google.com");
			curl_easy_setopt(CurlInstance, CURLOPT_NOBODY, 1);

			if (curl_easy_perform(CurlInstance) != CURLE_OK)
			{
				lwmf::AddLogEntry("No internet connection found.");
			}

			curl_easy_cleanup(CurlInstance);
		}

		return Result;
	}

	inline void FetchFileFromURL(const std::string& URL, const std::string& Filename)
	{
		if (CURL* CurlInstance{ curl_easy_init() }; CurlInstance != nullptr && CheckInternetConnection())
		{
			lwmf::AddLogEntry(fmt::format("Downloading file from URL via curl...\nSource: {}...", URL));

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
		}
	}


} // namespace Tools_Curl
