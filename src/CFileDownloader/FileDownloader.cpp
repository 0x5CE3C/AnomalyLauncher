#include "main.h"
#include "FileDownloader.h"


using namespace std;

namespace fs = std::filesystem;

double progress_ = 0.0f;
double lastReportedProgress = -1.0;
std::atomic<Task*> g_activeDownloadTask{ nullptr };

string download_info = "N\\A";


HCHANNEL menu_file_download;
static HSAMPLE menu_file_download_sample;
char	filename_s[512];


void progress_callback(void* userdata, double download_speed, double remaining_time, double progress_percentage,
    curl_off_t dltotal, curl_off_t dlnow)
{
    // Статическое состояние именно для этой функции: последний отправленный прогресс
    //static double lastReportedProgress = -1.0;

    // Сразу обновляем прогресс задачи для UI без задержек!
    progress_ = progress_percentage;
    if (Task* activeTask = g_activeDownloadTask.load()) {
        activeTask->progress = static_cast<float>(progress_percentage);
    }


    // Порог для обновления (чтобы не спамить каждые 0.01%)
    const double progressThreshold = 0.5;

    bool shouldReport = false;

    if (progress_percentage >= 99.9) {
        // Для 100% сообщаем только если раньше не сообщали 100%
        if (lastReportedProgress < 99.0) {
            shouldReport = true;
            lastReportedProgress = 100.0; // фиксируем, что 100% уже сообщили
        }
    }
    else {
        // Для промежуточных значений — порог 0.5%
        if (std::abs(progress_percentage - lastReportedProgress) >= progressThreshold) {
            shouldReport = true;
            lastReportedProgress = progress_percentage;
        }
    }

    // Если не нужно обновлять UI/лог — выходим раньше (экономим sprintf/логирование)
    if (!shouldReport) {
        return;
    }

    int hours = 0, minutes = 0, seconds = 0;

    if (download_speed > 0 && remaining_time > 0)
    {
        hours = static_cast<int>(remaining_time / 3600);
        minutes = static_cast<int>((remaining_time - hours * 3600) / 60);
        seconds = static_cast<int>(remaining_time - hours * 3600 - minutes * 60);
    }
    else
    {
        remaining_time = 0;
    }

    const char* unit = "B\\s";
    double speed_val = download_speed;

    if (speed_val > 1024 * 1024 * 1024)
    {
        unit = "Gb\\s";
        speed_val /= (1024.0 * 1024.0 * 1024.0);
    }
    else if (speed_val > 1024 * 1024)
    {
        unit = "Mb\\s";
        speed_val /= (1024.0 * 1024.0);
    }
    else if (speed_val > 1024)
    {
        unit = "Kb\\s";
        speed_val /= 1024.0;
    }

    double downloaded_bytes = static_cast<double>(dlnow);
    // Раскомментируй, если resumeByte доступен глобально/в классе:
    // downloaded_bytes += static_cast<double>(resumeByte);

    double total_bytes = static_cast<double>(dltotal);

    auto formatSize = [](double bytes) -> std::pair<double, const char*> {
        if (bytes >= 1024 * 1024 * 1024)
            return { bytes / (1024.0 * 1024.0 * 1024.0), "Gb" };
        if (bytes >= 1024 * 1024)
            return { bytes / (1024.0 * 1024.0), "Mb" };
        if (bytes >= 1024)
            return { bytes / 1024.0, "Kb" };
        return { bytes, "B" };
        };

    auto [downloaded_val, downloaded_unit] = formatSize(downloaded_bytes);
    auto [total_val, total_unit] = formatSize(total_bytes);

    char speedStr[32] = { 0 };
    char timeStr[32] = { 0 };
    char sizeStr[64] = { 0 };

#ifdef _WIN32
    sprintf_s(speedStr, sizeof(speedStr), "%.2f %s", speed_val, unit);
    sprintf_s(timeStr, sizeof(timeStr), u8"%02d ч %02d м %02d с", hours, minutes, seconds);
    sprintf_s(sizeStr, sizeof(sizeStr), "%.2f %s \\ %.2f %s", downloaded_val, downloaded_unit, total_val, total_unit);
#else
    snprintf(speedStr, sizeof(speedStr), "%.2f %s", speed_val, unit);
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, seconds);
    snprintf(sizeStr, sizeof(sizeStr), "%.2f %s \\ %.2f %s", downloaded_val, downloaded_unit, total_val, total_unit);
#endif

    // Лог (теперь он вызывается только при реальном изменении прогресса)
    // sn->con.AddLog("Скорость: %s | Осталось: %s | Скачано: %s", speedStr, timeStr, sizeStr);

    progress_ = progress_percentage;

    ProgressBarInfo(u8"Скорость: %s | Осталось: %s | Скачано: %s", speedStr, timeStr, sizeStr);
}

void    ProgressBarInfo(const char* info, ...)IM_FMTARGS(2)
{
    char buf[0x1000];
    static int c = 0;
    va_list args;
    va_start(args, info);
    vsnprintf(buf, IM_ARRAYSIZE(buf), info, args);
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);
    download_info = buf;



    /// <summary>
    ///  ЗВУК ЗАГРУЗКИ
    /// </summary>    
        // Управление звуком загрузки
    static bool bmenu_file_download_a = false;
    if (sn->bmenu_file_download.enabled)
    {
        //BASS_Init(-1, 44100, BASS_DEVICE_CPSPEAKERS, GetActiveWindow(), NULL);
        BASS_Init(-1, 44100, 0, 0, NULL);
        if (!bmenu_file_download_a)
        {
            sn->con.AddLog("[info]: menu file download sound Enabled");
            bmenu_file_download_a = 1;
        }

        _snprintf(filename_s, sizeof(filename_s), "%s\\%s\\%s", GetExePath().c_str(), PROJECT_FOLDER, BE_MENU_FILEDOWNLOADING);
        menu_file_download_sample = BASS_SampleLoad(false, filename_s, 0, 0, 1, NULL);
        menu_file_download = BASS_SampleGetChannel(menu_file_download_sample, FALSE);
        BASS_ChannelPlay(menu_file_download, FALSE);
        QWORD len = BASS_ChannelGetLength(menu_file_download, BASS_POS_BYTE);
        DWORD time = BASS_ChannelBytes2Seconds(menu_file_download, len);

        if (BASS_ChannelIsActive(menu_file_download) == BASS_ACTIVE_PLAYING)
        {
            BASS_ChannelSetAttribute(menu_file_download, BASS_ATTRIB_VOL, (float)sn->menu_file_download_volume / 100);
        }
    }
    else
    {
        if (bmenu_file_download_a)
        {
            sn->con.AddLog("[info]:  menu file download sound Disabled");
            bmenu_file_download_a = 0;
        }

        if (bmenu_file_download_a == 0)
        {
            if (BASS_ChannelIsActive(menu_file_download) == BASS_ACTIVE_PLAYING)
            {
                BASS_SampleFree(menu_file_download_sample);
                BASS_ChannelPause(menu_file_download);

                //sn->con.AddLog("BASS_ChannelStop");
            }
        }
    }
};

string link, fname;
string sfname;

string GetFileNamebyServer(string input_url)
{
    string filename = input_url;
    filename = filename.substr(filename.find_last_of("/") + 1);
    filename = filename.erase(filename.find_first_of('?'));

    return filename;
}

/*// Глобальный или член класса флаг результата последней загрузки
std::atomic<bool> isDownloadWorkerDone{ false };
std::atomic<bool> downloadFinishedOk{ false };*/

void StartDownloading()
{
    // Гарантируем, что при ЛЮБОМ return (пустая ссылка, ошибка имени, конец загрузки)
    // флаг isDownloadWorkerDone станет true, и цикл в AddFileDownload не зависнет!
    struct DoneGuard {
        ~DoneGuard() { isDownloadWorkerDone = true; }
    } guard;

    if (link.empty()) {
        sn->con.AddLog("[FileDownloader]Ошибка: Пустая ссылка для загрузки");
        Log("[FileDownloader]Ошибка: Пустая ссылка для загрузки");
        return;
    }

    string url = link;

    if (url.find("https://disk.yandex.ru") == 0) {
        url = GetConvertedLinkbyYandex(url);
        string yandex_fname = GetFileNamebyYandex(url); // Получаем имя файла от Yandex

        if (yandex_fname.empty()) {
            sn->con.AddLog("[Yandex]Ошибка: Неверное имя файла, полученное из ссылки");
            Log("[Yandex]Ошибка: Неверное имя файла, полученное из ссылки");
            return;
        }
        else
        {
            fname = yandex_fname; // Используем значение имени файла от Yandex
        }

        //Log("[Yandex]Ссылка: %s", url.c_str());
        //Log("[Yandex]Имя файла: %s", fname.c_str());        
    }
    else if (url.find("https://s3.firstvds.ru") == 0) {
        if (sfname.empty())
        {
            fname = GetFileNamebyServer(url); // Получаем имя файла от сервера
        }
        else
        {
            fname = sfname;
        }


        if (fname.empty()) {
            sn->con.AddLog("[Server]Ошибка: Неверное имя файла, полученное из ссылки");
            Log("[Server]Ошибка: Неверное имя файла, полученное из ссылки");
            return;
        }

        //Log("[Server]Ссылка: %s", url.c_str());
        //Log("[Server]Имя файла: %s", fname.c_str());        
    }
    else if (url.find("https://disk.yandex.ru") != 0 && url.find("https://s3.firstvds.ru") != 0)
    {
        fname = sfname;
        if (fname.empty()) {
            sn->con.AddLog("[FileDownloader]Ошибка: Неверное имя файла, полученное из прямой ссылки");
            Log("[FileDownloader]Ошибка: Неверное имя файла, полученное из прямой ссылки");
            return;
        }
        //Log("[Server]Ссылка: %s", url.c_str());
        //Log("[Server]Имя файла: %s", fname.c_str());        
    }            
       
    //Поддержка github внутри
    //HttpClient::getInstance()->HttpGet(url, fname, NULL, progress_callback);
    // 
    // HttpGet сам дождётся окончания загрузки и вернёт CURLE_OK при успехе
    int res = HttpClient::getInstance()->HttpGet(url, fname, NULL, progress_callback);
    downloadFinishedOk = (res == CURLE_OK);
}

void filedownload(string url, string staticfilename)
{
    isDownloadWorkerDone = false;
    downloadFinishedOk = false;

    //Создает тупик еюаный в Менеджере задач
/*    if (url.empty()) {
        sn->con.AddLog("Error: Empty download link");
        Log("Error: Empty download link");

        return;// nullptr;
    }*/

    // Сначала останавливаем скачивание
/*auto httpClient = HttpClient::getInstance();
if (httpClient) {
    httpClient->stopDownload();
}*/

    if (url.empty()) {
        sn->con.AddLog("Error: Empty download link");
        Log("Error: Empty download link");
        isDownloadWorkerDone = true; // Сразу сигнализируем, что поток не запускался и работа окончена
        return;
    }

    link = url;
    sfname = staticfilename;

    StartDownloading();

    /*auto thw = ThreadWrapper::create();
    thw->beginAsync(StartDownloading);
    thw->detachAll();*/
}

string GetConvertedLinkbyYandex(string input_url)
{
    cnp::init();

    // Загрузка страницы
    std::string result_text = cnp::download_page("https://cloud-api.yandex.net/v1/disk/public/resources/download?public_key=" + input_url);

    // Проверка пустого ответа
    if (result_text.empty()) {
        sn->con.AddLog("Error: Empty response from Yandex");
        Log("Error: Empty response from Yandex");

        return "";
    }

    // Поиск href в ответе
    size_t href_pos = result_text.find("\"href\":\"");
    if (href_pos == std::string::npos) {
        sn->con.AddLog("Error: Download link not found in response");
        Log("Error: Download link not found in response");
        return "";
    }

    // Извлечение ссылки
    string converted = result_text.substr(href_pos + 8);
    size_t end_quote = converted.find_first_of("\"");

    if (end_quote == std::string::npos) {
        sn->con.AddLog("Error: Malformed download link");
        Log("Error: Malformed download link");
        return "";
    }

    converted = converted.substr(0, end_quote);

    cnp::cleanup();

    return converted;
}



string GetFileNamebyYandex(string input_url)
{
    string filename;
    filename = input_url;
    filename = filename.substr(filename.find("filename=") + 9);
    //Log("converted fname: %s", fname);
    filename = filename.erase(filename.find_first_of("&"));
    //Log("converted fname: %s", fname);
    //Log("%s", cp1251_to_utf8(result_text.c_str()).c_str());
    //fname = filename;
    //Log("%s", cp1251_to_utf8(converted.c_str()).c_str());
    //   
    return filename;
};