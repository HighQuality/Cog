#pragma once

typedef void CURL;

class HttpDownload
{
public:
	HttpDownload();
	~HttpDownload();

	bool IsEnabled() const { return myHandle != nullptr; }

	void SetUrl(const StringView& aUrl);

private:
	CURL* myHandle;
};
