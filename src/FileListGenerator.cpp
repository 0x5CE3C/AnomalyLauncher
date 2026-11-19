#include "main.h"
#include "FileListGenerator.h"
#include "LauncherConfig.h"
#include "ConfigManager.h"

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

// ЗАГЛУШКА MD5. Вставь сюда свою реальную реализацию calcMD5 из проекта.
std::string calcMD5(const std::string& path) {
    return "";
}

std::string extractVersion(const std::string& filename) {
    // Ищем последовательность цифр и точек, которая выглядит как версия
    // \d+ - одна или более цифр
    // (\.\d+)* - ноль или более повторений ".цифра"
    std::regex versionRegex(R"(\d+(?:\.\d+)*)");

    std::smatch match;
    if (std::regex_search(filename, match, versionRegex)) {
        return match.str();
    }
    return ""; // Если версии нет, возвращаем пустоту
}

bool isPathExcluded(const std::string& fullPath, const std::string& ruleFolder, ModuleType currentModType, ModuleType ruleType) {
    if (ruleType != ModuleType::All && ruleType != currentModType)
        return false;

    std::string entryName = "";
    size_t lastSlash = fullPath.find_last_of("\\");

    if (lastSlash != std::string::npos && lastSlash < fullPath.length() - 1) {
        entryName = fullPath.substr(lastSlash + 1);
    }
    else if (lastSlash == fullPath.length() - 1 && lastSlash > 0) {
        size_t prevSlash = fullPath.find_last_of("\\", lastSlash - 1);
        if (prevSlash != std::string::npos) entryName = fullPath.substr(prevSlash + 1);
    }

    if (entryName == ruleFolder) return true;

    std::string needle = "\\" + ruleFolder + "\\";
    if (fullPath.find(needle) != std::string::npos) return true;

    std::string rootCheck = ruleFolder + "\\";
    if (fullPath.length() >= rootCheck.length()) {
        if (fullPath.substr(0, rootCheck.length()) == rootCheck) return true;
    }

    return false;
}


// Вспомогательная функция: заменить одиночные '\' на '\\' для вывода в кавычках
// НОВАЯ ФУНКЦИЯ: превращает одиночные \ в двойные \\ для формата filelist.ini
static std::string escapePath(const std::string& path) {
    std::string result;
    result.reserve(path.size() * 2);
    for (char c : path) {
        if (c == '\\') {
            result += "\\\\";  // В итоговом файле будет ровно "\\", что и нужно
        }
        else {
            result += c;
        }
    }
    return result;
}

// Получить относительный путь от корня игры (exeRoot)
static std::string getRelativePath(const fs::path& root, const fs::path& file) {
    fs::path rel = fs::relative(file, root);
    std::string relStr = rel.string();
    // Заменяем одиночные слэши на двойные для формата filelist
    return escapePath(relStr);
}

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

bool FileListGenerator::generate(const char* outPath) {
    int currentId = 0;

    // 1. Открываем файл ТОЛЬКО в бинарном режиме.
    // Никаких "ccs=UTF-8" в fopen — это ломает работу с fprintf!
    FILE* f = fopen(outPath, "wb");
    if (!f) {
        std::cerr << "[ERROR] Не удалось открыть файл для записи: " << outPath << std::endl;
        return false;
    }

    // 2. Отключаем автоконвертацию перевода строки (CR/LF).
    // Это обязательно, иначе Windows добавит лишний байт \r, и парсер лаунчера сломается.
#ifdef _WIN32
    _setmode(_fileno(f), _O_BINARY);
#endif

    // Заголовок файла (ASCII, проблем не будет)
    fprintf(f, "# Format: ID Filename Dir ModuleType FileType Hash DownloadFilename DownloadUrl\n");
    fprintf(f, "[filelist]\n\n");

    auto& cfg = ConfigManager::getInstance();
    const auto& paths = LauncherConfig::getInstance().getPaths();
    const auto& blacklistRules = ConfigManager::getInstance().getBlackListRules();
    const fs::path& globalRoot = paths.rootPath;

    auto processModule = [&](const fs::path& moduleRoot, ModuleType modType, bool useBinFilter, const std::string& typeName) {
        if (!fs::exists(moduleRoot)) {
            std::cerr << "[WARN] Путь не найден: " << moduleRoot.string() << " (тип: " << typeName << ")\n";
            return;
        }

        try {
            // ОПРЕДЕЛЯЕМ, НУЖЕН ЛИ НАМ ТОЛЬКО ПЕРВЫЙ УРОВЕНЬ
            bool isMO2ModsFolder = (typeName == "pz" && moduleRoot.filename().string() == "mods" || typeName == "mp" && moduleRoot.filename().string() == "mods" || typeName == "mo2" && moduleRoot.filename().string() == "MO2");

            if (isMO2ModsFolder) {
                // --- ВАРИАНТ 1: ТОЛЬКО ПЕРВЫЙ УРОВЕНЬ (для MO2\mods) ---
                // Используем обычный directory_iterator — он НЕ заходит внутрь папок
                for (const auto& entry : fs::directory_iterator(moduleRoot)) {
                    const auto& path = entry.path();
                    std::string fullPath = path.string();

                    // 1. Исключения лаунчера
                    if (entry.is_regular_file()) {
                        if (path.filename().string() == "AnomalyLauncher.exe" ||
                            path.filename().string() == "bass.dll" ||
                            path.filename().string() == "7za.exe") {
                            continue;
                        }
                        // Если нужно игнорировать meta.ini прямо в папке mods, раскомментируй:
                        /*
                        if (path.filename().string() == "meta.ini") {
                            continue;
                        }
                        */
                    }

                    // 2. Blacklist rules
                    bool excluded = false;
                    for (const auto& rule : blacklistRules) {
                        if (isPathExcluded(fullPath, rule.folderName, modType, rule.type)) {
                            excluded = true;
                            break;
                        }
                    }
                    if (excluded) continue;

                    // 3. Фильтр bin (если включен)
                    bool allowFile = true;
                    if (useBinFilter && entry.is_regular_file()) {
                        bool found = false;
                        std::string relativePath = fs::relative(path, moduleRoot).string();
                        for (char& c : relativePath) if (c == '/') c = '\\';

                        for (const auto& allowed : cfg.getBinFiles()) {
                            std::string allowedPath = allowed.path;
                            if (!allowedPath.empty() && allowedPath[0] != '\\') {
                                allowedPath = "\\" + allowedPath;
                            }

                            if ((relativePath.find(allowedPath) != std::string::npos) || (relativePath == allowedPath)) {
                                if (allowed.type == modType || allowed.type == ModuleType::All) {
                                    found = true;
                                    break;
                                }
                            }
                        }
                        if (!found) allowFile = false;
                    }

                    if (!allowFile) continue;

                    // --- ЗАПОЛНЕНИЕ И ЗАПИСЬ (одинаково для обоих случаев) ---
                    FileListItem item;
                    item.id = ++currentId;
                    if (item.id > 65535) break;

                    item.filename = path.filename().string();
                    item.moduleType = typeName;
                    item.fileType = entry.is_directory() ? "dir" : "file";

                    fs::path parentDir = path.parent_path();
                    fs::path relPath = fs::relative(parentDir, globalRoot);
                    std::string relStr = relPath.string();

                    for (char& c : relStr) if (c == '/') c = '\\';
                    if (relStr == ".") {
                        relStr = "";
                    }
                    //item.dir = relStr;

                     // Экранирование одиночных "\" в "\\"
                    item.dir = escapePath(relStr);

                    //Log("item.dir: %s", item.dir.c_str());

                    // Игнор каталогов
                    if (item.moduleType == "anomaly" && entry.is_directory()) continue;                    

                    if (entry.is_directory() && relStr == "") continue;

                    if (entry.is_regular_file()) {
                        if (sn->scanhashes.enabled == true)
                        {
                        item.hash = md5file(path.string().c_str());//calcMD5(path.string());
                        }
                        else
                        {
                            item.hash.clear();
                        }
                        item.downloadFilename = item.filename + ".zip";

                        std::string urlPath = item.dir.empty() ? "" : (item.dir + "/");
                        size_t pos = 0;
                        while ((pos = urlPath.find('\\', pos)) != std::string::npos) {
                            urlPath.replace(pos, 1, "/");
                            pos += 1;
                        }
                        item.downloadUrl = "";
                    }
                    else {
                        //item.hash.clear();
                        if (sn->scanhashes_dirs.enabled == true)
                        {
                            item.hash = md5_files_in_directory(path);
                        }
                        else
                        {
                            item.hash.clear();
                        }
                        
                        //item.downloadFilename.clear();
                        //item.downloadFilename = item.filename + ".zip";

                        // mp сторонняя сборка, созданная другим разрабом
                        if (item.moduleType == "pz" || item.moduleType == "mp")
                        {
                            // Где ты формируешь item.downloadFilename:
                            std::string baseName = extractVersion(item.filename); // например, "1.0.1"
                            if (!baseName.empty()) {
                                item.downloadFilename = baseName + ".zip";     // будет "1.0.1.zip"
                            }
                            else {
                                // Фолбэк, если версия не найдена (на всякий случай)
                                item.downloadFilename = item.filename + ".zip";
                            }
                        }   

                        if (item.moduleType == "mo2")
                        {
                            item.downloadFilename = item.filename + ".zip";
                        }

                        item.downloadUrl.clear();
                    }

                    /*fprintf(f, "%d \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n",
                        item.id,
                        item.filename.c_str(),
                        item.dir.c_str(),
                        item.moduleType.c_str(),
                        item.fileType.c_str(),
                        item.hash.c_str(),
                        item.downloadFilename.c_str(),
                        item.downloadUrl.c_str());*/

                        // Конвертируем все строки из CP1251 (как они у тебя сейчас) в UTF-8
                    std::string fnameUtf8 = cp1251_to_utf8(item.filename);
                    std::string dirUtf8 = cp1251_to_utf8(item.dir);
                    std::string hashUtf8 = cp1251_to_utf8(item.hash);
                    std::string dlNameUtf8 = cp1251_to_utf8(item.downloadFilename);
                    std::string urlUtf8 = cp1251_to_utf8(item.downloadUrl);

                    fprintf(f, "%d \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n",
                        item.id,
                        fnameUtf8.c_str(),
                        dirUtf8.c_str(),
                        item.moduleType.c_str(),      // Это ASCII ("extractor", "anomaly"), конвертировать не надо
                        item.fileType.c_str(),        // Это ASCII ("file", "dir"), конвертировать не надо
                        hashUtf8.c_str(),
                        dlNameUtf8.c_str(),
                        urlUtf8.c_str());

                    sn->con.AddLog("%d \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n",
                        item.id,
                        item.filename.c_str(),
                        item.dir.c_str(),
                        item.moduleType.c_str(),
                        item.fileType.c_str(),
                        item.hash.c_str(),
                        item.downloadFilename.c_str(),
                        item.downloadUrl.c_str());
                }
            }
            else {
                // --- ВАРИАНТ 2: РЕКУРСИВНЫЙ ОБХОД (для всех остальных папок) ---
                for (const auto& entry : fs::recursive_directory_iterator(moduleRoot, fs::directory_options::skip_permission_denied)) {
                    const auto& path = entry.path();
                    std::string fullPath = path.string();

                    // (Здесь тот же код проверок и заполнения, что и выше)
                    if (entry.is_regular_file()) {
                        if (path.filename().string() == "AnomalyLauncher.exe" ||
                            path.filename().string() == "bass.dll" ||
                            path.filename().string() == "7za.exe") {
                            continue;
                        }
                    }

                    bool excluded = false;
                    for (const auto& rule : blacklistRules) {
                        if (isPathExcluded(fullPath, rule.folderName, modType, rule.type)) {
                            excluded = true;
                            break;
                        }
                    }
                    if (excluded) continue;

                    bool allowFile = true;
                    if (useBinFilter && entry.is_regular_file()) {
                        bool found = false;
                        std::string relativePath = fs::relative(path, moduleRoot).string();
                        for (char& c : relativePath) if (c == '/') c = '\\';

                        for (const auto& allowed : cfg.getBinFiles()) {
                            std::string allowedPath = allowed.path;
                            if (!allowedPath.empty() && allowedPath[0] != '\\') {
                                allowedPath = "\\" + allowedPath;
                            }

                            if ((relativePath.find(allowedPath) != std::string::npos) || (relativePath == allowedPath)) {
                                if (allowed.type == modType || allowed.type == ModuleType::All) {
                                    found = true;
                                    break;
                                }
                            }
                        }
                        if (!found) allowFile = false;
                    }

                    if (!allowFile) continue;

                    FileListItem item;
                    item.id = ++currentId;
                    if (item.id > 65535) break;

                    item.filename = path.filename().string();
                    item.moduleType = typeName;
                    item.fileType = entry.is_directory() ? "dir" : "file";

                    fs::path parentDir = path.parent_path();
                    fs::path relPath = fs::relative(parentDir, globalRoot);
                    std::string relStr = relPath.string();

                    for (char& c : relStr) if (c == '/') c = '\\';
                    if (relStr == ".") {
                        relStr = "";
                    }
                    //item.dir = relStr;

                     // Экранирование одиночных "\" в "\\"
                    item.dir = escapePath(relStr);

                    //Log("item.dir: %s", item.dir.c_str());

                     // Игнор каталогов
                    if (item.moduleType == "anomaly" && entry.is_directory()) continue;                    

                    if (entry.is_directory() && relStr == "") continue;

                    if (entry.is_regular_file()) {
                        if (sn->scanhashes.enabled == true)
                        {
                            item.hash = md5file(path.string().c_str());//calcMD5(path.string());
                        }
                        else
                        {
                            item.hash.clear();
                        }

                        item.downloadFilename = item.filename + ".zip";

                        std::string urlPath = item.dir.empty() ? "" : (item.dir + "/");
                        size_t pos = 0;
                        while ((pos = urlPath.find('\\', pos)) != std::string::npos) {
                            urlPath.replace(pos, 1, "/");
                            pos += 1;
                        }
                        item.downloadUrl = "";
                    }
                    else {
                        if (sn->scanhashes_dirs.enabled == true)
                        {
                            item.hash = md5_files_in_directory(path);
                        }
                        else
                        {
                            item.hash.clear();
                        }
                        //item.hash = md5_files_in_directory(path);
                        //item.downloadFilename.clear();
                        item.downloadFilename = item.filename + ".zip";
                        item.downloadUrl.clear();
                    }

                    // Конвертируем все строки из CP1251 (как они у тебя сейчас) в UTF-8
                    std::string fnameUtf8 = cp1251_to_utf8(item.filename);
                    std::string dirUtf8 = cp1251_to_utf8(item.dir);
                    std::string hashUtf8 = cp1251_to_utf8(item.hash);
                    std::string dlNameUtf8 = cp1251_to_utf8(item.downloadFilename);
                    std::string urlUtf8 = cp1251_to_utf8(item.downloadUrl);

                    /*fprintf(f, "%d \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n",
                        item.id,
                        item.filename.c_str(),
                        item.dir.c_str(),
                        item.moduleType.c_str(),
                        item.fileType.c_str(),
                        item.hash.c_str(),
                        item.downloadFilename.c_str(),
                        item.downloadUrl.c_str());*/

                    fprintf(f, "%d \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n",
                        item.id,
                        fnameUtf8.c_str(),
                        dirUtf8.c_str(),
                        item.moduleType.c_str(),      // Это ASCII ("extractor", "anomaly"), конвертировать не надо
                        item.fileType.c_str(),        // Это ASCII ("file", "dir"), конвертировать не надо
                        hashUtf8.c_str(),
                        dlNameUtf8.c_str(),
                        urlUtf8.c_str());

                    sn->con.AddLog("%d \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n",
                        item.id,
                        item.filename.c_str(),
                        item.dir.c_str(),
                        item.moduleType.c_str(),
                        item.fileType.c_str(),
                        item.hash.c_str(),
                        item.downloadFilename.c_str(),
                        item.downloadUrl.c_str());
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "[ERROR] Ошибка ФС при сканировании " << typeName << ": " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "[ERROR] Неожиданная ошибка при сканировании модуля " << typeName << std::endl;
        }
        };


    processModule(paths.pathExtractor, ModuleType::Extractor, sn->filter_only_extractor.enabled, "extractor");
    processModule(paths.pathAnomaly, ModuleType::Anomaly, sn->filter_only_anomaly.enabled, "anomaly");
    processModule(paths.pathMO2, ModuleType::MO2, sn->filter_only_mo2.enabled, "mo2");

    switch (sn->umode.project_mode) {
    case PROJECTMODES::PM_Anomaly_PZ:
        processModule(paths.pathPZ, ModuleType::PZ, sn->filter_only_mp.enabled, "pz");
        break;
    case PROJECTMODES::PM_Anomaly_MP:
        processModule(paths.pathPZ, ModuleType::PZ, sn->filter_only_mp.enabled, "mp");
        break;
    default:
        break;
    }

    

    fclose(f);
    std::cout << "[OK] filelist.ini создан. Всего записей: " << currentId << std::endl;
    return true;
}





int RunCmdNoWindow(const std::string& cmdAnsi) {
    // 1. Конвертируем ANSI (CP1251) в UTF?16 (wchar_t)
    int len = MultiByteToWideChar(CP_ACP, 0, cmdAnsi.c_str(), -1, nullptr, 0);
    if (len <= 0) {
        Log("[ERROR] MultiByteToWideChar failed, WinError: %lu", GetLastError());
        return -1;
    }

    std::wstring cmdWide(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, cmdAnsi.c_str(), -1, cmdWide.data(), len);

    // ВАЖНО: CreateProcessW модифицирует строку, поэтому берём data() и не делаем const
    LPWSTR cmdToRun = cmdWide.data();

    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;  // Надёжнее явно скрыть окно

    PROCESS_INFORMATION pi;

    DWORD flags = CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT;

    // Явно вызываем W?версию: CreateProcessW
    if (!CreateProcessW(nullptr, cmdToRun, nullptr, nullptr, FALSE, flags, nullptr, nullptr, &si, &pi)) {
        Log("[ERROR] CreateProcessW failed, WinError: %lu", GetLastError());
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return static_cast<int>(exitCode);
}



// Вспомогательная: заменить все \ на /
std::string normalizePath(const std::string& input) {
    std::string out = input;
    for (char& c : out) {
        if (c == '\\') c = '/';
    }
    return out;
}


// Основной метод: создать архив, если его нет
bool EnsureModArchiveExists(const char* modFolderPath, const char* sevenZaPath, bool allowOverwrite, const std::string& archiveName) {
    if (!modFolderPath || !sevenZaPath) {
        Log("[ERROR] Invalid paths.");
        return false;
    }

    std::string srcPathStr = modFolderPath;
    // Убираем завершающий слэш, чтобы проверить существование папки
    while (!srcPathStr.empty() && (srcPathStr.back() == '/' || srcPathStr.back() == '\\')) {
        srcPathStr.pop_back();
    }

    if (!PathFileExistsA(srcPathStr.c_str())) {
        Log("[ERROR] Mod folder not found: %s", srcPathStr.c_str());
        return false;
    }

    // Вычисляем outDir (родительская папка для архива)
    std::string outDir = srcPathStr;
    size_t lastSlash = outDir.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        outDir = outDir.substr(0, lastSlash);
    }
    else {
        outDir = ".";
    }

    std::string outPath = outDir;
    if (!outPath.empty() && outPath.back() != '/' && outPath.back() != '\\') {
        outPath += '/';
    }
    outPath += archiveName;

    // Если архив уже есть и перезапись не нужна — считаем успехом
    if (fsm->FileExist(outPath.c_str()) && !allowOverwrite) {
        return true;
    }

    // Нормализуем путь к 7za (заменяем \ на /)
    std::string sevenZaNorm = sevenZaPath;
    for (char& c : sevenZaNorm) {
        if (c == '\\') c = '/';
    }

    // ПРАВИЛЬНАЯ команда для 7za:
    // a -tzip -mx1 -r "output.zip" "source_folder"
    std::ostringstream cmd;
    cmd << "\"" << sevenZaNorm << "\" "
        << "a -tzip -mx1 -r -y "
        << "\"" << outPath << "\" "
        << "\"" << srcPathStr << "\"";

    Log("[ARCHIVE] Running: %s", cmd.str().c_str());

    int result = RunCmdNoWindow(cmd.str());

    // Проверяем факт существования файла архива
    if (fsm->FileExist(outPath.c_str())) {
        Log("[ARCHIVE] Success: %s created", outPath.c_str());
        return true;
    }
    else {
        Log("[ERROR] 7za failed or file not created. Exit code: %d", result);
        Log("[DEBUG] Expected file at: %s", outPath.c_str());
        if (result == 0) {
            Log("[HINT] Process exited with 0, but archive is missing. Check:");
            Log("       - Antivirus blocking 7za.exe");
            Log("       - Write permissions on the target folder");
            Log("       - Whether 7za actually ran (not a fake exe)");
        }
        return false;
    }
}




void GenerateModsToZip()
{
    if (!fsm->FileExist(zipPath.c_str()))
        return;

    const char* pathTo7za = zipPath.c_str();

    for (int i = 0; i < iFileListCount; ++i) {
        const stFileList& item = filelist.fileList[i];

        if (item.id == 0 || !item.name || strlen(item.name) == 0)
            continue;

        if (item.m_type == SUBMODULE_TYPE_BE && item.f_type == FILE_TYPE_DIR)
        {
            std::string basePath = GetExePath();
            std::string modDir = item.dir;          // "MO2/mods"
            std::string modName = item.name;        // "1.0 Основной мод..."

            // Формируем полный путь к папке мода
            std::string fullPath = basePath + "/" + modDir + "/" + modName;

            // Нормализация слэшей
            for (char& c : fullPath) { if (c == '\\') c = '/'; }

            // Удаление двойных слэшей
            for (size_t pos = 0; (pos = fullPath.find("//", pos)) != std::string::npos; ) {
                fullPath.replace(pos, 2, "/");
                pos += 1;
            }

            if (!fullPath.empty() && fullPath.back() != '/') {
                fullPath += '/';
            }
            else if (fullPath.empty()) {
                fullPath = "./";
            }

            Log("[Extractor] > %s", fullPath.c_str());

            // --- ГЛАВНОЕ ИЗМЕНЕНИЕ: передаём готовое имя архива ---
            // item.downloadFilename уже содержит "1.0.zip" или "1.0.1.zip"
            EnsureModArchiveExists(fullPath.c_str(), pathTo7za, true, item.downloadfilename);
        }
    }
}