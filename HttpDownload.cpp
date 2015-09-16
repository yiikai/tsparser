#include "HttpDownload.h"
#include <iostream>
HttpDownloader::HttpDownloader() :m_curl(NULL)
{
	//m_pcontent = std::make_shared<std::vector<char>>();
}

HttpDownloader::~HttpDownloader()
{
	curl_easy_cleanup(m_curl);
}

bool HttpDownloader::init()
{
	char error[1024] = { 0 };
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
	if (code != CURLE_OK)
	{
		std::cout << "failed to global all init" << std::endl;
		return false;
	}
	m_curl = curl_easy_init();
	code = curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, error);
	if (code != CURLE_OK)
	{
		printf("Failed to set error buffer [%d]\n", code);
		return false;
	}
	curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
	
	code = curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1);
	if (code != CURLE_OK)
	{
		printf("Failed to set redirect option [%s]\n", error);
		return false;
	}
	code = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_callback);
	if (code != CURLE_OK)
	{
		printf("Failed to set writer [%s]\n", error);
		return false;
	}
	code = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_downloadbuf);
	if (code != CURLE_OK)
	{
		printf("Failed to set write data [%s]\n", error);
		return false;
	}
	return true;
}

long HttpDownloader::write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	long sizes = size * nmemb;
	((std::vector<unsigned char>*)userdata)->insert(((std::vector<unsigned char>*)userdata)->end(),ptr,ptr+sizes);
	return sizes;
}

bool HttpDownloader::setDownloadRange(char* range)
{
	char error[1024] = { 0 };
	CURLcode code;
	code = curl_easy_setopt(m_curl, CURLOPT_RANGE, range);
	if (code != CURLE_OK)
	{
		return false;
	}
	return true;
}

bool HttpDownloader::startDownload(const unsigned char* url, std::shared_ptr<std::vector<unsigned char>>& getdownloaddata)
{
	char error[1024] = { 0 };
	CURLcode code;
	code = curl_easy_setopt(m_curl, CURLOPT_URL, url);
	if (code != CURLE_OK)
	{
		printf("Failed to set URL [%s]\n", error);
		return false;
	}
	code = curl_easy_perform(m_curl);
	if (code != CURLE_OK)
	{
		std::cout << "perform error" << std::endl;
		return false;
	}
	long retcode = 0;
	code = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &retcode);
	if (code == CURLE_OK && (retcode == 200 || retcode == 206))
	{
		getdownloaddata = std::make_shared<std::vector<unsigned char>>(m_downloadbuf);
	}
	return true;
}