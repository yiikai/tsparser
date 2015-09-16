#ifndef HTTP_DOWNLOAD_H
#define HTTP_DOWNLOAD_H

#include <curl/curl.h>
#include <vector>
#include <memory>
typedef long (*write_callback)(char *ptr, size_t size, size_t nmemb, void *userdata);

class HttpDownloader
{
public:
	HttpDownloader();
	~HttpDownloader();

	bool init();
	bool startDownload(const unsigned char* url, std::shared_ptr<std::vector<unsigned char>>& getdownloaddata);
	bool setDownloadRange(char* range = NULL);
private:
	static long write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

private:
	CURL *m_curl;
	std::vector<unsigned char> m_downloadbuf;
	//std::shared_ptr<std::vector<char>> m_pcontent;
};

#endif

