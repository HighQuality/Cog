#include "pch.h"
#include "HttpClient.h"
#include <curl/curl.h>

struct LibCurlInitializer
{
	LibCurlInitializer()
	{
		auto error = curl_global_init(CURL_GLOBAL_ALL);

		if (error == CURLE_OK)
			Println(L"Successfully initialized libcurl");
		else
			Println(L"libcurl initialization failed (error code {0})", error);
	}

};

static LibCurlInitializer _libcurlinitializer;

HttpDownload::HttpDownload()
{
	myHandle = curl_easy_init();

}


HttpDownload::~HttpDownload()
{
	curl_easy_cleanup(myHandle);
}

void HttpDownload::SetUrl(const StringView& aUrl)
{
	curl_easy_setopt(myHandle, CURLOPT_URL, aUrl.ToStdString().c_str());
}
