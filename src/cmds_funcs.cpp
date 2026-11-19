#include "main.h"

void cmd_exit(const char* args)
{
	//abort();
    Log("[AnomalyLauncher]: Успешное закрытие.");
    ExitProcess(0);
};
void cmd_help(const char* args)
{
	sn->con.AddLog("Список Команд:");
	for (int i = 0; i < sn->con.Commands.Size; i++)
	{
		sn->con.AddLog("--> %s - %s", sn->con.Commands[i]->name, sn->con.Commands[i]->description);
	}
};

static bool stopCleaning = false;

void execHidden(const std::string& command) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.wShowWindow = SW_HIDE;  // Скрыть окно
    si.dwFlags = STARTF_USESHOWWINDOW;

    ZeroMemory(&pi, sizeof(pi));

    std::string cmd = "cmd.exe /C " + command;

    if (!CreateProcess(NULL, const_cast<LPSTR>(cmd.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "CreateProcess failed (" << GetLastError() << "). Command: " << cmd << std::endl;
        return;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void clearEventLogs() {
    std::string output = exec("wevtutil.exe el");
    std::istringstream iss(output);
    std::string event;

    while (std::getline(iss, event)) {
        if (!event.empty()) {
            if (stopCleaning) {
                sn->con.AddLog("Stopping cleaning process.");
                return;
            }

            std::string clearCommand = "wevtutil.exe cl \"" + event + "\"";
            execHidden(clearCommand); // Используем скрытую функцию

            // Логируем результат
            sn->con.AddLog("Cleared: %s", event.c_str());
        }
    }    
    sn->con.AddLog("All event logs cleared successfully.");
}

void cmd_cleanjc(const char* args)
{    
    stopCleaning = false;
    if (args != nullptr) {
        char command[16];
        sscanf(args, "%s", command);
        if (strcmp(command, "0") == 0) {
            stopCleaning = true;          
            return;
        }
    }

    try {
        auto thw = ThreadWrapper::create();
        thw->beginAsync(clearEventLogs);        
        thw->detachAll();        
    }
    catch (const std::exception& e) {
        sn->con.AddLog("Error: %s", e.what());
    }
};

void cmd_cls(const char* args) 
{
    sn->con.ClearLog();
    sn->con.AddLog("{FF0000}Консоль Очищена!");///Красный цвет
};

void cmd_getfilelist(const char* args)
{
    GetListFiles(GetExePath().c_str());
};

void cmd_odir(const char* args)
{    
    const char* edit = GetExePath().c_str();
    ShellExecuteA(NULL, "explore", edit, NULL, NULL, SW_SHOWNORMAL);
};

void cmd_getexename(const char* args) {
    sn->con.AddLog(u8"%s", getFileNameWithoutExtension(GetExeName().c_str()).c_str());
}

void cmd_getexenameext(const char* args) 
{
    sn->con.AddLog(u8"%s", getFileNameWithExtension(GetExeName().c_str()).c_str());
}


std::string exec(const char* cmd) {
    // Устанавливаем кодировку вывода консоли на CP1251 (Windows-1251) для кириллицы
    //SetConsoleOutputCP(1251);
    //SetConsoleCP(1251);  // Устанавливаем кодовую страницу ввода тоже

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hReadPipe, hWritePipe;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        throw std::runtime_error("CreatePipe failed");
    }
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    // Подготавливаем команду в изменяемом буфере
    std::string cmdStr = cmd;
    std::vector<char> cmdLine(cmdStr.c_str(), cmdStr.c_str() + cmdStr.size() + 1);

    if (!CreateProcessA(
        NULL,
        cmdLine.data(),
        NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi
    )) {
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        throw std::runtime_error("CreateProcess failed");
    }

    CloseHandle(hWritePipe);

    std::string result;
    char buffer[512];
    DWORD bytesRead;

    while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result.append(buffer, bytesRead);
        std::string cp1251_str = cp866_to_cp1251(buffer);
        sn->con.AddLog("%s", cp1251_str.c_str());
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hReadPipe);

    return result;
}


void runDiagnostics() {
    // === Внешняя функция логирования в файл ===
    extern void Log(const char* format, ...);

    // === Простое логирование: и в файл, и в консоль ===
    auto logBoth = [&](const char* fmt, ...) {
        // Буфер для строки
        char buf[2048];

        // Собираем строку один раз
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        // 1. Пишем в файл (как есть)
        Log("%s", buf);

        // 2. Пишем в ImGui-консоль (как есть)
        sn->con.AddLog("%s", buf);
        };

    // === Диагностика (осталась без изменений) ===
    bool isAdmin = []() -> bool {
        BOOL fIsRunAsAdmin = FALSE;
        HANDLE hToken = nullptr;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            TOKEN_ELEVATION elevation;
            DWORD dwSize;
            if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
                fIsRunAsAdmin = elevation.TokenIsElevated;
            }
        }
        if (hToken) CloseHandle(hToken);
        return fIsRunAsAdmin;
        }();

    bool inVirtualStore = []() -> bool {
        char path[MAX_PATH];
        if (GetModuleFileNameA(nullptr, path, MAX_PATH)) {
            std::string exePath(path);
            return exePath.find("VirtualStore") != std::string::npos;
        }
        return false;
        }();

    bool canWrite = []() -> bool {
        const std::string dir = "temp";
        if (!std::filesystem::exists(dir)) {
            std::error_code ec;
            std::filesystem::create_directories(dir, ec);
            if (ec) return false;
        }

        std::string testFile = dir + "\\__diag_test__.tmp";
        std::error_code ec;

        std::ofstream file(testFile, std::ios::binary);
        if (!file) return false;
        file << "test";
        file.close();

        if (!std::filesystem::exists(testFile)) return false;

        bool removed = std::filesystem::remove(testFile, ec);
        return removed && !ec;
        }();

    int staleConfigFiles = []() -> int {

        //CleanupPartFiles(GetBaseDir());

        // Сначала пробуем явно указать путь к AnomalyLauncher (можно вынести в конфиг)
        fs::path baseDir = GetBaseDir();

        // Вариант 1: если мы уже в папке AnomalyLauncher — оставляем baseDir как есть
        // Вариант 2: если нужно искать подпапку AnomalyLauncher внутри текущей директории
        fs::path candidate = baseDir / "AnomalyLauncher";
        if (fs::exists(candidate) && fs::is_directory(candidate)) {
            baseDir = candidate;
        }

        if (!fs::exists(baseDir) || !fs::is_directory(baseDir)) {
            return -1; // каталог не найден
        }

        int count = 0;
        try {
            for (const auto& entry : fs::directory_iterator(baseDir)) {
                if (!entry.is_regular_file()) continue;

                auto ext = entry.path().extension().string();
                if (ext == ".part") {
                    // ВАЖНО: удаляем по полному пути, а не только по имени файла
                    std::error_code ec;
                    fs::remove(entry.path(), ec);
                    if (!ec) {
                        ++count;
                    }
                    else {
                        // Тут можно добавить логирование (ZLOG и т.п.)
                        Log("Не удалось удалить: %s, причина: %s", entry.path(), ec.message());
                    }
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            Log("Ошибка при сканировании каталога: %s", e.what());
            return -1;
        }
        catch (...) {
            return -1;
        }

        return count;
        }();

    int staleNearlauncherFiles = []() -> int {

        //CleanupPartFiles(GetBaseDir());

        // Сначала пробуем явно указать путь к AnomalyLauncher (можно вынести в конфиг)
        fs::path baseDir = GetBaseDir();

        // Вариант 1: если мы уже в папке AnomalyLauncher — оставляем baseDir как есть
        // Вариант 2: если нужно искать подпапку AnomalyLauncher внутри текущей директории
       /* fs::path candidate = baseDir / "AnomalyLauncher";
        if (fs::exists(candidate) && fs::is_directory(candidate)) {
            baseDir = candidate;
        }*/

        if (!fs::exists(baseDir) || !fs::is_directory(baseDir)) {
            return -1; // каталог не найден
        }

        int countlauncher = 0;
        try {
            for (const auto& entry : fs::directory_iterator(baseDir)) {
                if (!entry.is_regular_file()) continue;

                auto ext = entry.path().extension().string();
                if (ext == ".part") {
                    // ВАЖНО: удаляем по полному пути, а не только по имени файла
                    std::error_code ec;
                    fs::remove(entry.path(), ec);
                    if (!ec) {
                        ++countlauncher;
                    }
                    else {
                        // Тут можно добавить логирование (ZLOG и т.п.)
                        Log("Не удалось удалить: %s, причина: %s", entry.path(), ec.message());
                    }
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            Log("Ошибка при сканировании каталога: %s", e.what());
            return -1;
        }
        catch (...) {
            return -1;
        }

        return countlauncher;
        }();    

    bool dnsOk = []() -> bool {
        struct addrinfo* result = nullptr;
        int status = getaddrinfo("firsts3.ru", nullptr, nullptr, &result);
        if (result) freeaddrinfo(result);
        return status == 0;
        }();

    bool serverOk = false;
    long httpCode = 0;

    if (dnsOk) {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) == 0) {
            CURL* curl = curl_easy_init();
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, "https://firsts3.ru");
                curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
                //curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
                curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
                curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36");

                CURLcode res = curl_easy_perform(curl);
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
                curl_easy_cleanup(curl);

                serverOk = (res == CURLE_OK) && (httpCode == 200 || httpCode == 204);
            }
            curl_global_cleanup();
        }
    }

    char exePath[MAX_PATH] = { 0 };
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);

    // === Вывод отчёта — дважды: в Log и в sn->con.AddLog ===
    logBoth("=== ДИАГНОСТИКА СИСТЕМЫ ===");
    logBoth("Запущен от админа: %s", isAdmin ? "Да" : "Нет");
    logBoth("В VirtualStore: %s", inVirtualStore ? "Да (ПРОБЛЕМА)" : "Нет");
    logBoth("Доступ к temp(Файловая система): %s", canWrite ? "ОК" : "ЗАПРЕЩЁН");
    logBoth("Висячие .part: %d шт.(Каталог с лаунчером)", staleNearlauncherFiles);
    logBoth("Висячие .part: %d шт.(Каталог с конфигурацией)", staleConfigFiles);
    logBoth("DNS: %s", dnsOk ? "Разрешён" : "Не разрешён");
    logBoth("Сервер доступен: %s", serverOk ? "Да" : "Нет");
    logBoth("HTTP статус: %ld", httpCode ? httpCode : -1L);
    logBoth("Путь к лаунчеру: %s", exePath);
    logBoth("Папка temp: temp");
    logBoth("========================");

    if (inVirtualStore) {
        logBoth(" Переместите лаунчер в папку без контроля UAC (например, D:\\Launcher)");
    }
    if (!canWrite) {
        logBoth(" Проверьте права на папку temp или отключите антивирус на время");
    }
    if (!dnsOk) {
        logBoth(" Проверьте интернет или смените DNS (например, на 1.1.1.1)");
    }
    if (dnsOk && !serverOk) {
        logBoth(" Сервер недоступен. Возможна блокировка провайдером или фаерволом");
    }

    if (staleConfigFiles > 0)
    {
        logBoth(" Обнаружены висячие *.part файлы загрузчика в каталоге конфигурации. Очищено автоматически!");
    }

    if (staleNearlauncherFiles > 0)
    {
        logBoth(" Обнаружены висячие *.part файлы загрузчика в каталоге лаунчера. Очищено автоматически!");
    }

    if (staleConfigFiles == 0)
    {
        logBoth(" Висячие *.part файлы загрузчика в каталоге конфигурации не обнаружены. Очистка отменена!");
    }

    if (staleNearlauncherFiles == 0)
    {
        logBoth(" Висячие *.part файлы загрузчика в каталоге лаунчера не обнаружены. Очистка отменена!");
    }

    logBoth(" Диагностика завершена.");
}


void cmd_diag(const char* args)
{
    try {
        auto thw = ThreadWrapper::create();
        thw->beginAsync(runDiagnostics);
        thw->detachAll();        
    }
    catch (const std::exception& e) {
        sn->con.AddLog("Error: %s", e.what());
    }
};

void cmd_getbasedir(const char* args) 
{
    string basedir = GetBaseDir();
    sn->con.AddLog("%s", basedir.c_str());
}

void clearparts()
{
    // === Внешняя функция логирования в файл ===
    extern void Log(const char* format, ...);

    // === Простое логирование: и в файл, и в консоль ===
    auto logBoth = [&](const char* fmt, ...) {
        // Буфер для строки
        char buf[2048];

        // Собираем строку один раз
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        // 1. Пишем в файл (как есть)
        Log("%s", buf);

        // 2. Пишем в ImGui-консоль (как есть)
        sn->con.AddLog("%s", buf);
        };

    int staleConfigFiles = []() -> int {

        //CleanupPartFiles(GetBaseDir());

        // Сначала пробуем явно указать путь к AnomalyLauncher (можно вынести в конфиг)
        fs::path baseDir = GetBaseDir();

        // Вариант 1: если мы уже в папке AnomalyLauncher — оставляем baseDir как есть
        // Вариант 2: если нужно искать подпапку AnomalyLauncher внутри текущей директории
        fs::path candidate = baseDir / "AnomalyLauncher";
        if (fs::exists(candidate) && fs::is_directory(candidate)) {
            baseDir = candidate;
        }

        if (!fs::exists(baseDir) || !fs::is_directory(baseDir)) {
            return -1; // каталог не найден
        }

        int count = 0;
        try {
            for (const auto& entry : fs::directory_iterator(baseDir)) {
                if (!entry.is_regular_file()) continue;

                auto ext = entry.path().extension().string();
                if (ext == ".part") {
                    // ВАЖНО: удаляем по полному пути, а не только по имени файла
                    std::error_code ec;
                    fs::remove(entry.path(), ec);
                    if (!ec) {
                        ++count;
                    }
                    else {
                        // Тут можно добавить логирование (ZLOG и т.п.)
                        Log("Не удалось удалить: %s, причина: %s", entry.path(), ec.message());
                    }
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            Log("Ошибка при сканировании каталога: %s", e.what());
            return -1;
        }
        catch (...) {
            return -1;
        }

        return count;
        }();

    int staleNearlauncherFiles = []() -> int {

        //CleanupPartFiles(GetBaseDir());

        // Сначала пробуем явно указать путь к AnomalyLauncher (можно вынести в конфиг)
        fs::path baseDir = GetBaseDir();

        // Вариант 1: если мы уже в папке AnomalyLauncher — оставляем baseDir как есть
        // Вариант 2: если нужно искать подпапку AnomalyLauncher внутри текущей директории
       /* fs::path candidate = baseDir / "AnomalyLauncher";
        if (fs::exists(candidate) && fs::is_directory(candidate)) {
            baseDir = candidate;
        }*/

        if (!fs::exists(baseDir) || !fs::is_directory(baseDir)) {
            return -1; // каталог не найден
        }

        int countlauncher = 0;
        try {
            for (const auto& entry : fs::directory_iterator(baseDir)) {
                if (!entry.is_regular_file()) continue;

                auto ext = entry.path().extension().string();
                if (ext == ".part") {
                    // ВАЖНО: удаляем по полному пути, а не только по имени файла
                    std::error_code ec;
                    fs::remove(entry.path(), ec);
                    if (!ec) {
                        ++countlauncher;
                    }
                    else {
                        // Тут можно добавить логирование (ZLOG и т.п.)
                        Log("Не удалось удалить: %s, причина: %s", entry.path(), ec.message());
                    }
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            Log("Ошибка при сканировании каталога: %s", e.what());
            return -1;
        }
        catch (...) {
            return -1;
        }

        return countlauncher;
        }();

    if (staleConfigFiles > 0)
    {
        logBoth(" Обнаружены висячие *.part файлы загрузчика в каталоге конфигурации. Очищено автоматически!");
    }

    if (staleNearlauncherFiles > 0)
    {
        logBoth(" Обнаружены висячие *.part файлы загрузчика в каталоге лаунчера. Очищено автоматически!");
    }

    if (staleConfigFiles == 0)
    {
        logBoth(" Висячие *.part файлы загрузчика в каталоге конфигурации не обнаружены. Очистка отменена!");
    }    

    if (staleNearlauncherFiles == 0)
    {
        logBoth(" Висячие *.part файлы загрузчика в каталоге лаунчера не обнаружены. Очистка отменена!");
    }

    logBoth(" == Очистка висячих *.part файлов завершена! ==");
}

void cmd_clearparts(const char* args) 
{
    try {        
        auto thw = ThreadWrapper::create();
        thw->beginAsync(clearparts);
        thw->detachAll();
    }
    catch (const std::exception& e) {
        sn->con.AddLog("Error: %s", e.what());
    }
}