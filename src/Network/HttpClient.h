#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

using namespace std;

#ifdef _WIN32
typedef _off_t p_off_t;
#else
typedef off_t p_off_t;
#endif

typedef void(*progress_info_callback)(void* userdata, double downloadSpeed, double remainingTime, double progressPercentage, curl_off_t ultotal, curl_off_t ulnow);

typedef struct
{
	void* sender;
	CURL* handle;
	progress_info_callback cb;
}Progress_User_Data;

typedef enum
{
	HTTP_REQUEST_OK = CURLE_OK,

	HTTP_REQUEST_ERROR = -999,

}Http_Client_Response;

class HttpClient
{
public:

	~HttpClient();

	static HttpClient* getInstance();
	static void destroyInstance();

	int HttpGet(const std::string& requestURL, const std::string& saveTo, void* sender, progress_info_callback cb);

	bool init();

	void stopDownload();

	uint64_t GetMaxDownloadSpeedBps() const {
		return m_maxDownloadSpeedBps;
	}

	bool IsDownloading() const { 
		return isDownloading; 
	}

	string GetDownloadFileName() const {
		return filename_download;
	}

	void setBandwidthLimit(uint64_t bps);

private:
	HttpClient();

	static size_t write_callback(char* buffer, size_t size, size_t nmemb, void* userdata);
	static int progress_callback(void* userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);


private:
	static HttpClient* instance;

	static double downloadFileLength;
	static curl_off_t resumeByte;

	// Call frequency of the callback function
	static time_t lastTime;

	static uint64_t m_maxDownloadSpeedBps;

	static std::unique_ptr<HttpClient> create();

	static bool isDownloading;
	static string filename_download;

	void PStopDownload(CURL* handle);

	CURL* currentHandle;

	static std::atomic<bool> m_cancelRequested; // Глобальный флаг отмены

	static curl_off_t getRemoteFileSize(const std::string& url);
	static curl_off_t getLocalFileLength(const std::string& path);


private:
	// Анализ HTTP-кода: решает, стоит ли ретраить или это фатальная ошибка
	bool analyzeHttpCode(int httpCode, bool& shouldRetry, bool& isFatal);

	// Выполняет одну попытку скачивания (то, что раньше было телом HttpGet)
	int performSingleDownloadAttempt(
		const std::string& requestURL,
		const std::string& saveTo,
		void* sender,
		progress_info_callback cb,
		int attemptNumber);
};
#endif		// __HTTPCLIENT_H__