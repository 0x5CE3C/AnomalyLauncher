#include "main.h"
#include "HttpClient.h"
#include <sys/stat.h>
#include <mutex>

HttpClient* HttpClient::instance = NULL;

double HttpClient::downloadFileLength = -1;
curl_off_t HttpClient::resumeByte = -1;
time_t HttpClient::lastTime = 0;

uint64_t HttpClient::m_maxDownloadSpeedBps = 10ULL * 1024ULL * 1024ULL; // 10 Mb/s default

bool HttpClient::isDownloading = false;
string HttpClient::filename_download = "N\\A";

std::mutex downloadMutex; // Мютекс для защиты доступа к isDownloading

std::atomic<bool> HttpClient::m_cancelRequested(false);


HttpClient::HttpClient()
{
}


HttpClient::~HttpClient()
{
}

HttpClient* HttpClient::getInstance()
{
	if (instance == NULL)
	{
		instance = new HttpClient();
		instance->init();
	}

	return instance;
}

void HttpClient::destroyInstance()
{
	if (instance != NULL)
	{		

		curl_global_cleanup(); // will crash on Qt windows

		delete instance;
		instance = NULL;
	}
}

bool HttpClient::init()
{
	if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
		return false;

	return true;
}


int HttpClient::HttpGet(const std::string& requestURL, const std::string& saveTo, void* sender, progress_info_callback cb)
{
	std::lock_guard<std::mutex> lock(downloadMutex);
	if (isDownloading) {
		Log("[HttpClient]: Ошибка: загрузка уже выполняется");
		return HTTP_REQUEST_ERROR;
	}

	isDownloading = true;

	const int maxRetries = 5;
	int lastError = HTTP_REQUEST_ERROR;
	std::string finalPartPath = saveTo + ".part";

	// =========================================================
	// ПРОВЕРКА ПЕРЕД ПОПЫТКАМИ: только для случая, когда .part БОЛЬШЕ сервера
	// =========================================================
	curl_off_t localSize = HttpClient::getLocalFileLength(finalPartPath);
	curl_off_t remoteSize = HttpClient::getRemoteFileSize(requestURL);

	if (localSize > 0 && remoteSize > 0) {
		if (localSize > remoteSize) {
			Log("[HttpClient]: Локальный .part (%lld) > размер на сервере (%lld). Удаляем для чистого старта.",
				(long long)localSize, (long long)remoteSize);
			remove(finalPartPath.c_str());
		}
	}
	else if (localSize > 0 && remoteSize == -1) {
		// Если размер сервера неизвестен (S3/подписи) — безопаснее удалить .part и качать с нуля.
		// Это предотвращает ошибку 416 и лишние ретраи.
		Log("[HttpClient]: Размер на сервере неизвестен. Удаляем .part для безопасного старта.");
		remove(finalPartPath.c_str());
	}

	for (int attempt = 1; attempt <= maxRetries; ++attempt)
	{
		Log("[HttpClient]: Попытка скачивания файла %d из %d", attempt, maxRetries);
		sn->con.AddLog("[HttpClient]: Попытка скачивания файла %d из %d", attempt, maxRetries);

		int ret = performSingleDownloadAttempt(requestURL, saveTo, sender, cb, attempt);

		// === ГЛАВНОЕ: если пользователь отменил — сразу выходим, без ретраев ===
		if (ret == CURLE_ABORTED_BY_CALLBACK) {
			Log("[HttpClient]: Загрузка отменена пользователем. Ретраи не выполняются.");
			sn->con.AddLog("[HttpClient]: Загрузка отменена. Ретраи пропущены.");
			isDownloading = false;
			progress_ = 100.0f; // или оставь текущий прогресс, если хочешь
			return ret;
		}

		if (ret == CURLE_OK) {
			// Удаляем старый файл (если он был), переименовываем .part
			remove(saveTo.c_str());

			if (rename(finalPartPath.c_str(), saveTo.c_str()) == 0) {
				filename_download = saveTo;
				isDownloading = false;
				progress_ = 100.0f;
				Log("[HttpClient]: Файл %s скачан (попытка №%d)", filename_download.c_str(), attempt);
				sn->con.AddLog("[HttpClient]: File %s is downloaded", filename_download.c_str());
				return CURLE_OK;
			}
			else {
				Log("[HttpClient]: Не удалось переименовать файл. Повторяем...");
				lastError = HTTP_REQUEST_ERROR;
			}
		}
		else {
			lastError = ret;
		}

		if (attempt == maxRetries) {
			Log("[HttpClient]: Достигнуто максимальное число попыток. Пропуск файла...");
			sn->con.AddLog("[HttpClient]: Достигнуто максимальное число попыток. Пропуск файла...");
			break;
		}

		sleep(2000);
	}

	isDownloading = false;
	progress_ = 100.0f;

	if (lastError != CURLE_OK) {
		Log("[HttpClient]: Загрузка не удалась после %d попыток. Файл: %s", maxRetries, filename_download.c_str());
		sn->con.AddLog("[HttpClient]: Загрузка не удалась после %d попыток. Файл: %s", maxRetries, filename_download.c_str());
	}

	return lastError;
}



curl_off_t HttpClient::getRemoteFileSize(const std::string& url)
{
	CURL* easy_handle = curl_easy_init();
	if (!easy_handle) return -1;

	curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(easy_handle, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(easy_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36");
	curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 10L);

	curl_off_t contentLength = -1;
	int hasContentLength = curl_easy_getinfo(easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);

	curl_easy_cleanup(easy_handle);

	if (hasContentLength != CURLE_OK || contentLength < 0) {
		return -1;
	}
	return contentLength;
}



size_t HttpClient::write_callback(char* buffer, size_t size, size_t nmemb, void* userdata)
{
	FILE* fp = static_cast<FILE*>(userdata);
	size_t length = fwrite(buffer, size, nmemb, fp);
	if (length != nmemb)
	{
		return length;
	}

	return size * nmemb;
}



int HttpClient::progress_callback(void* userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	(void)ultotal;
	(void)ulnow;


	time_t now = time(NULL);
	// Ограничиваем частоту вызова логики (не чаще 1 раза в секунду)
	if (now - lastTime < 1)
	{
		return 0;
	}
	lastTime = now;

	Progress_User_Data* data = static_cast<Progress_User_Data*>(userdata);
	CURL* easy_handle = data->handle;


	// ГЛАВНОЕ: если пользователь нажал «Стоп», возвращаем CURLE_ABORT
	if (m_cancelRequested.load(std::memory_order_relaxed)) {
		return CURLE_ABORTED_BY_CALLBACK;
	}


	double speed = 0.0;
	curl_easy_getinfo(easy_handle, CURLINFO_SPEED_DOWNLOAD, &speed);

	double leftTime = 0.0;
	double progress = 0.0;

	// Корректный учёт уже скачанного до начала этой сессии (докачка)
	double current_downloaded_bytes = static_cast<double>(dlnow) + static_cast<double>(resumeByte);

	if (dltotal > 0)
	{
		progress = (current_downloaded_bytes / static_cast<double>(dltotal)) * 100.0;
		progress_ = progress;

		// Защита от деления на ноль и отрицательных значений
		if (speed > 1.0) // скорость хотя бы 1 байт/сек
		{
			double remaining_bytes = static_cast<double>(dltotal) - dlnow;
			leftTime = remaining_bytes / speed;
		}
		else
		{
			leftTime = 0.0; // если скорость почти 0 — не показываем «бесконечность»
		}

		// Логирование (оставляем для отладки, но не слишком часто из-за lastTime)
		//Log("progress: %.2f%% | speed: %.2f B/s | resumeByte: %lld | total: %lld | downloaded: %lld", \
			progress, speed, resumeByte, dltotal, dlnow);
	}
	else
	{
		// Если сервер не прислал Content-Length — показываем прогресс условно по скачанным байтам
		// и не пытаемся считать «осталось времени»
		progress_ = 0.0; // или можно считать как-то иначе, если нужно
		leftTime = 0.0;
	}

	// Вызов внешнего коллбэка (UI, прогресс-бар и т.п.)
	if (data->cb != nullptr)
	{
		data->cb(data->sender, speed, leftTime, progress, dltotal, dlnow);
	}

	return 0;
}

// Get the local file size, return -1 if failed
curl_off_t HttpClient::getLocalFileLength(const std::string& path) {
#ifdef _WIN32
	struct _stat64 st;
	if (_stat64(path.c_str(), &st) == 0) {
		return static_cast<curl_off_t>(st.st_size);
	}
#else
	struct stat st;
	if (stat(path.c_str(), &st) == 0) {
		return static_cast<curl_off_t>(st.st_size);
	}
#endif
	return -1; // Файл не найден или ошибка
}

size_t nousecb(char* buffer, size_t x, size_t y, void* userdata)
{
	(void)buffer;
	(void)userdata;
	return x * y;
}


void HttpClient::stopDownload() {
	if (!currentHandle) return;

	m_cancelRequested.store(true, std::memory_order_release);
	Log("[HttpClient]: Загрузка остановлена пользователем (флаг установлен)");

	// Опционально: можно всё-таки сделать паузу, чтобы не качать лишнее
	curl_easy_pause(currentHandle, CURLPAUSE_RECV);
}

bool HttpClient::analyzeHttpCode(int httpCode, bool& shouldRetry, bool& isFatal)
{
	shouldRetry = false;
	isFatal = false;

	Log("[HttpClient]: Анализ кода входящего HTTP-ответа: %d", httpCode);
	sn->con.AddLog("[HttpClient]: Анализ кода входящего HTTP-ответа: %d", httpCode);

	switch (httpCode)
	{
	case 200:
	case 206:
		// Успех: файл есть или сервер отдаёт диапазон
		shouldRetry = false;
		isFatal = false;
		return true; // Можно считать успехом

		// --- Клиентские ошибки: почти все фатальны, ретраи не помогут ---
	case 400: // Bad Request
		shouldRetry = false;
		isFatal = true;
		Log("[HttpClient]: Критическая ошибка: неверный запрос. Отмена повтора.");
		sn->con.AddLog("[HttpClient]: Критическая ошибка: неверный запрос. Отмена повтора.");
		return false;

	case 401: // Unauthorized
		shouldRetry = false;
		isFatal = true;
		Log("[HttpClient]: Критическая ошибка: требуется авторизация. Отмена повтора.");
		sn->con.AddLog("[HttpClient]: Критическая ошибка: требуется авторизация. Отмена повтора.");
		return false;

	case 403: // Forbidden
		shouldRetry = false;
		isFatal = true;
		Log("[HttpClient]: Критическая ошибка: сервер запрещает доступ. IP-адрес в бане или Файрвол блокирует лаунчер. Отмена повтора.");
		sn->con.AddLog("[HttpClient]: Критическая ошибка: сервер запрещает доступ. IP-адрес в бане или Файрвол блокирует лаунчер. Отмена повтора.");
		return false;

	case 404:
		// Файла нет — ретраи не помогут
		shouldRetry = false;
		isFatal = true;
		Log("[HttpClient]: Критическая ошибка: Файл не найден. Отмена повтора.");
		sn->con.AddLog("[HttpClient]: Критическая ошибка: Файл не найден. Отмена повтора.");
		return false;

	case 410: // Gone
		shouldRetry = false;
		isFatal = true;
		Log("[HttpClient]: Критическая ошибка: файл удалён с сервера. Нужна обновлённая версия списка файлов. Отмена повтора.");
		sn->con.AddLog("[HttpClient]: Критическая ошибка: файл удалён с сервера. Нужна обновлённая версия списка файлов. Отмена повтора.");
		return false;

	case 413: // Payload Too Large
		shouldRetry = false;
		isFatal = true;
		Log("[HttpClient]: Критическая ошибка: запрос отклонён из-за размера. Проверьте заголовки и параметры. Отмена повтора.");
		sn->con.AddLog("[HttpClient]: Критическая ошибка: запрос отклонён из-за размера. Проверьте заголовки и параметры. Отмена повтора.");
		return false;

	case 415: // Unsupported Media Type
		shouldRetry = false;
		isFatal = true;
		Log("[HttpClient]: Критическая ошибка: сервер не принимает такой тип запроса. Отмена повтора.");
		sn->con.AddLog("[HttpClient]: Критическая ошибка: сервер не принимает такой тип запроса. Отмена повтора.");
		return false;


	case 416:
		// Неверный диапазон (локальный .part больше файла на сервере)
		// Это не «сервер упал», это нужно сбросить докачку и начать заново
		shouldRetry = true;  // Да, пробуем снова, но с очисткой .part
		isFatal = false;
		Log("[HttpClient]: Ошибка: Неверный диапазон (локальный .part больше файла на сервере). Повторная попытка после очистки .part");
		sn->con.AddLog("[HttpClient]: Ошибка: Неверный диапазон (локальный .part больше файла на сервере). Повторная попытка после очистки .part");			
		return false;

	case 429:
		// Слишком много запросов — ждём и пробуем снова
		shouldRetry = true;
		isFatal = false;
		Log("[HttpClient]: Ошибка: Слишком много запросов. Ждём 2 секунды перед повторной попыткой.");
		sn->con.AddLog("[HttpClient]: Ошибка: Слишком много запросов. Ждём 2 секунды перед повторной попыткой.");
		return false;

	case 451: // Unavailable For Legal Reasons
		shouldRetry = false;
		isFatal = true;
		Log("[HttpClient]: Критическая ошибка: доступ к файлу заблокирован по требованию правообладателей. Отмена повтора.");
		sn->con.AddLog("[HttpClient]: Критическая ошибка: доступ к файлу заблокирован по требованию правообладателей. Отмена повтора.");
		return false;

		// --- Ошибки сервера: почти всегда стоит попробовать ещё раз ---
	case 500: // Internal Server Error
		shouldRetry = true;
		isFatal = false;
		Log("[HttpClient]: Внутренняя ошибка сервера. Будет повторная попытка.");
		sn->con.AddLog("[HttpClient]: Внутренняя ошибка сервера. Будет повторная попытка.");
		return false;

	case 502: // Bad Gateway
		shouldRetry = true;
		isFatal = false;
		Log("[HttpClient]: Сервер временно недоступен. Попробуем снова через паузу.");
		sn->con.AddLog("[HttpClient]: Сервер временно недоступен. Попробуем снова через паузу.");
		return false;

	case 503: // Service Unavailable
		shouldRetry = true;
		isFatal = false;
		Log("[HttpClient]: Сервис временно недоступен. Будет повтор с задержкой.");
		sn->con.AddLog("[HttpClient]: Сервис временно недоступен. Будет повтор с задержкой.");
		return false;

	case 504: // Gateway Timeout
		shouldRetry = true;
		isFatal = false;
		Log("[HttpClient]: Таймаут шлюза. Попробуем ещё раз.");
		sn->con.AddLog("[HttpClient]: Таймаут шлюза. Попробуем ещё раз.");
		return false;

	default:
		// Любой другой код (редкие 3xx, неизвестные и т.?п.)
		// Для безопасности считаем, что стоит попробовать ещё
		shouldRetry = true;
		isFatal = false;
		Log("[HttpClient]: Неизвестный код входящего HTTP-ответа %d. Повторяем попытку.", httpCode);
		sn->con.AddLog("[HttpClient]: Неизвестный код входящего HTTP-ответа %d. Повторяем попытку.", httpCode);
		return false;
	}
}


int HttpClient::performSingleDownloadAttempt(
	const std::string& requestURL,
	const std::string& saveTo,
	void* sender,
	progress_info_callback cb,
	int attemptNumber)
{
	std::string partPath = saveTo + ".part";
	CURL* easy_handle = nullptr;
	FILE* fp = nullptr;

	Log("[HttpClient]: START ATTEMPT #%d | URL=%s | saveTo=%s", attemptNumber, requestURL.c_str(), saveTo.c_str());

	// Получаем локальный размер ДО открытия файла (для логики докачки)
	curl_off_t localSize = getLocalFileLength(partPath);
	bool useResume = (localSize > 0);

	filename_download = partPath;

	easy_handle = curl_easy_init();
	if (!easy_handle) {
		Log("[HttpClient]: Попытка %d: не удалось инициализировать cURL", attemptNumber);
		sn->con.AddLog("[HttpClient]: Попытка %d: не удалось инициализировать cURL", attemptNumber);
		return HTTP_REQUEST_ERROR;
	}

	currentHandle = easy_handle;

#ifdef _WIN32
	if (fopen_s(&fp, partPath.c_str(), "ab+") != 0)
#else
	fp = fopen(partPath.c_str(), "ab+");
	if (!fp)
#endif
	{
		Log("[HttpClient]: Попытка %d: не удалось открыть файл %s", attemptNumber, partPath.c_str());
		sn->con.AddLog("[HttpClient]: Попытка %d: не удалось открыть файл %s", attemptNumber, partPath.c_str());

		currentHandle = nullptr;
		curl_easy_cleanup(easy_handle);
		return HTTP_REQUEST_ERROR;
	}

	curl_easy_setopt(easy_handle, CURLOPT_URL, requestURL.c_str());
	curl_easy_setopt(easy_handle, CURLOPT_HEADER, 0L);
	curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(easy_handle, CURLOPT_ENCODING, "");
	curl_easy_setopt(easy_handle, CURLOPT_MAXREDIRS, 10L);
	curl_easy_setopt(easy_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36");
	curl_easy_setopt(easy_handle, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(easy_handle, CURLOPT_SSL_VERIFYHOST, 0L);

	curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &HttpClient::write_callback);
	curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);

	Progress_User_Data data = { sender, easy_handle, cb };
	curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION, &HttpClient::progress_callback);
	curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA, &data);

	curl_easy_setopt(easy_handle, CURLOPT_FAILONERROR, 0L);
	curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT, 15L);
	curl_easy_setopt(easy_handle, CURLOPT_LOW_SPEED_LIMIT, 512L);
	curl_easy_setopt(easy_handle, CURLOPT_LOW_SPEED_TIME, 120L);

	if (m_maxDownloadSpeedBps > 0) {
		curl_easy_setopt(easy_handle, CURLOPT_MAX_RECV_SPEED_LARGE,
			static_cast<curl_off_t>(m_maxDownloadSpeedBps));
	}

	if (useResume) {
		// Передаём смещение для докачки
		curl_easy_setopt(easy_handle, CURLOPT_RESUME_FROM_LARGE, localSize);
	}

	int result = HTTP_REQUEST_ERROR;

	CURLcode ret = curl_easy_perform(easy_handle);

	long httpCode = 0;
	curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &httpCode);

	curl_off_t contentLength = -1;
	int hasContentLength = curl_easy_getinfo(easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);
	if (hasContentLength != CURLE_OK || contentLength < 0) {
		contentLength = -1;
	}

	curl_off_t dlNow = 0;
	curl_easy_getinfo(easy_handle, CURLINFO_SIZE_DOWNLOAD, &dlNow);

	// === ВАЖНО: объявляем totalDownloaded ЗДЕСЬ, чтобы он был виден во всём коде ===
	curl_off_t totalDownloaded = localSize + dlNow;

	// Очистка ресурсов (ДО возврата из функции!)
	if (fp) {
		fclose(fp);
		fp = nullptr;
	}

	if (currentHandle == easy_handle) {
		currentHandle = nullptr;
	}

	if (easy_handle) {
		curl_easy_cleanup(easy_handle);
		easy_handle = nullptr;
	}

	bool userCancelled = (ret == CURLE_ABORTED_BY_CALLBACK);

	// Если пользователь отменил — сразу ставим результат и выходим
	if (userCancelled) {
		result = CURLE_ABORTED_BY_CALLBACK;
		goto cleanup_and_return;
	}

	// Обработка ошибок cURL (сеть, таймаут и т.п.)
	if (ret != CURLE_OK) {
		if (ret == CURLE_OPERATION_TIMEDOUT ||
			ret == CURLE_COULDNT_CONNECT ||
			ret == CURLE_HTTP_RETURNED_ERROR) {
			remove(partPath.c_str());
			Log("[HttpClient]: Файл .part удалён из-за сетевой ошибки.");
			sn->con.AddLog("[HttpClient]: Файл .part удалён из-за сетевой ошибки.");
		}
		result = HTTP_REQUEST_ERROR;
		goto cleanup_and_return;
	}

	// Анализ HTTP-кода
	bool shouldRetry = false;
	bool isFatal = false;
	if (!analyzeHttpCode(httpCode, shouldRetry, isFatal)) {
		if (isFatal) {
			Log("[HttpClient]: Попытка %d критическая HTTP-ошибка %ld.", attemptNumber, httpCode);
			sn->con.AddLog("[HttpClient]: Попытка %d критическая HTTP-ошибка %ld.", attemptNumber, httpCode);
			result = HTTP_REQUEST_ERROR;
			goto cleanup_and_return;
		}
		else if (shouldRetry) {
			if (httpCode == 416) {
				remove(partPath.c_str());
				Log("[HttpClient]: Ошибка 416. Удаляем .part и начинаем заново.");
				sn->con.AddLog("[HttpClient]: Ошибка 416.");
			}
			result = HTTP_REQUEST_ERROR;
			goto cleanup_and_return;
		}
	}

	// Проверка поддержки диапазонов (сервер должен ответить 206 при докачке)
	if (useResume && httpCode != 206) {
		remove(partPath.c_str());
		Log("[HttpClient]: Сервер не поддерживает диапазоны (HTTP %ld). Удаляем .part.", httpCode);
		sn->con.AddLog("[HttpClient]: Сервер не поддерживает диапазоны.");
		result = HTTP_REQUEST_ERROR;
		goto cleanup_and_return;
	}

	// === ПРАВИЛЬНАЯ ВАЛИДАЦИЯ РАЗМЕРА (без ошибки totalDownloaded) ===
	bool isSizeValid = true;

	if (userCancelled) {
		// Отмена пользователем: валидно, если скачано хоть что-то
		if (totalDownloaded == 0) {
			isSizeValid = false;
			Log("[HttpClient]: Отмена, но скачано 0 байт. Считаем файл битым.");
		}
		else {
			Log("[HttpClient]: Загрузка прервана пользователем. Файл валиден для докачки (%lld байт).",
				(long long)totalDownloaded);
			isSizeValid = true;
		}
	}
	else if (useResume) {
		// Докачка (не отменена): если скачано хоть что-то — считаем валидным.
		// НЕ проверяем 85%, потому что dlNow — это только текущий кусок.
		if (dlNow > 0) {
			isSizeValid = true;
			Log("[HttpClient]: Докачка завершена успешно. Скачано дополнительно: %lld байт.",
				(long long)dlNow);
		}
		else {
			// Скачано 0 байт в этой попытке, но локальный был. Это странно — считаем битым.
			isSizeValid = false;
			Log("[HttpClient]: Докачка не дала прогресса (0 байт скачано). Удаляем.");
		}
	}
	else {
		// Полная загрузка с нуля (не докачка, не отмена)
		if (contentLength == -1) {
			// Размер неизвестен: удаляем только если 0 байт
			if (totalDownloaded == 0) isSizeValid = false;
		}
		else {
			// Известен размер: проверяем 85% только для полной загрузки
			if (totalDownloaded < (contentLength * 85 / 100)) {
				isSizeValid = false;
				Log("[HttpClient]: Файл слишком маленький: скачано %lld, ожидается %lld.",
					(long long)totalDownloaded, (long long)contentLength);
			}
		}
	}

	if (!isSizeValid) {
		remove(partPath.c_str());
		Log("[HttpClient]: Удаляем битый .part файл.");
		sn->con.AddLog("[HttpClient]: Удаляем битый .part файл.");
		result = HTTP_REQUEST_ERROR;
	}
	else {
		result = CURLE_OK;
	}

cleanup_and_return:
	// Сброс флага отмены перед выходом
	m_cancelRequested.store(false, std::memory_order_release);

	return result;
}





void HttpClient::setBandwidthLimit(uint64_t bps) {
	m_maxDownloadSpeedBps = bps; // поле в классе
}