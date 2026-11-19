#include "main.h"
#include "UrlImporter.h"
#include <commdlg.h>      // Для OPENFILENAME и GetOpenFileName
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <unordered_map>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

// -----------------------------------------------------------------------------
// Вспомогательная функция: диалог выбора файла (Windows)
// -----------------------------------------------------------------------------
std::string UrlImporter::showFileDialog() {
    OPENFILENAME ofn = { 0 };
    char szFile[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Link Files (*.ini;*.txt)\0*.ini;*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        return std::string(szFile);
    }
    return "";
}

// -----------------------------------------------------------------------------
// Парсер файла со ссылками
// Ищет строки вида: http://... или https://...
// Поддерживает кавычки и пробелы вокруг ссылки.
// -----------------------------------------------------------------------------
std::vector<std::string> UrlImporter::parseUrlsFromFile(const std::string& filename) {
    std::vector<std::string> urls;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "[UrlImporter] ERROR: Не удалось открыть файл ссылок: " << filename << std::endl;
        return urls;
    }

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        lineNum++;
        // Убираем пробелы и табы по краям
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue; // пустая строка

        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);

        // Пропускаем комментарии
        if (line[0] == '#' || line[0] == ';') continue;

        // Ищем начало URL
        size_t httpPos = line.find("http://");
        size_t httpsPos = line.find("https://");

        size_t urlStart = std::string::npos;
        if (httpPos != std::string::npos && (httpsPos == std::string::npos || httpPos < httpsPos)) {
            urlStart = httpPos;
        }
        else if (httpsPos != std::string::npos) {
            urlStart = httpsPos;
        }

        if (urlStart != std::string::npos) {
            // Ищем конец URL (пробел или конец строки)
            size_t urlEnd = line.find(' ', urlStart);
            if (urlEnd == std::string::npos) urlEnd = line.length();

            std::string url = line.substr(urlStart, urlEnd - urlStart);

            // Убираем возможные кавычки, если файл был в формате "url"
            if (!url.empty() && url.front() == '"') url.erase(0, 1);
            if (!url.empty() && url.back() == '"') url.pop_back();

            if (!url.empty()) {
                urls.push_back(url);
                // Можно раскомментировать для отладки:
                // std::cout << "[PARSER] Строка " << lineNum << ": найден URL: " << url << std::endl;
            }
        }
    }
    file.close();

    std::cout << "[UrlImporter] Найдено URL для обработки: " << urls.size() << std::endl;
    return urls;
}

// -----------------------------------------------------------------------------
// Логика сопоставления: URL -> filelist
// Сравнивает имя ZIP-файла (без параметров) с полем downloadfilename в filelist.
// Обновляет поле url в структуре filelist.
// -----------------------------------------------------------------------------
bool UrlImporter::matchUrlsToConfig(const std::vector<std::string>& urls) {
    if (urls.empty()) return false;

    // Создаем карту: имя_файла.zip -> URL
    // Приводим всё к нижнему регистру для надежного сравнения
    std::unordered_map<std::string, std::string> urlMap;

    for (const auto& url : urls) {
        size_t lastSlash = url.find_last_of('/');
        if (lastSlash == std::string::npos) continue;

        std::string zipName = url.substr(lastSlash + 1);

        // Удаляем параметры (?v=1, &id=2 и т.д.)
        size_t qPos = zipName.find('?');
        if (qPos != std::string::npos) zipName.erase(qPos);

        // Очистка от лишних пробелов
        zipName.erase(0, zipName.find_first_not_of(" \t\r\n"));
        zipName.erase(zipName.find_last_not_of(" \t\r\n") + 1);

        if (!zipName.empty()) {
            std::string lowerName = zipName;
            for (char& c : lowerName) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            urlMap[lowerName] = url;
        }
    }

    int matchesFound = 0;

    // Проходим по глобальному списку filelist
    for (int i = 0; i < iFileListCount; ++i) {
        const char* rawDlName = filelist.fileList[i].downloadfilename;

        // Защита от nullptr
        if (!rawDlName || std::string(rawDlName).empty()) continue;

        std::string currentZipName(rawDlName);
        for (char& c : currentZipName) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        auto it = urlMap.find(currentZipName);
        if (it != urlMap.end()) {
            const char* currentUrlRaw = filelist.fileList[i].url;
            const std::string& newUrlStr = it->second;

            bool needUpdate = false;

            // Если URL пустой или отличается - обновляем
            if (!currentUrlRaw || strlen(currentUrlRaw) == 0) {
                needUpdate = true;
            }
            else if (strcmp(currentUrlRaw, newUrlStr.c_str()) != 0) {
                needUpdate = true;
            }

            if (needUpdate) {
                std::cout << "[UrlImporter] Match ID=" << filelist.fileList[i].id
                    << " | File: " << rawDlName
                    << " | New URL: " << newUrlStr << std::endl;

                // Освобождаем старую память, если она была выделена через my_strdup/malloc
                if (currentUrlRaw != nullptr) {
                    free(const_cast<char*>(currentUrlRaw));
                }

                // Выделяем новую память под новый URL
                char* newUrlPtr = my_strdup(newUrlStr.c_str());
                filelist.fileList[i].url = newUrlPtr;
                matchesFound++;
            }
        }
    }

    std::cout << "[UrlImporter] Всего обновлено записей: " << matchesFound << std::endl;
    return matchesFound > 0;
}

// -----------------------------------------------------------------------------
// ГЛАВНЫЙ МЕТОД: Импорт + Сохранение
// -----------------------------------------------------------------------------
bool UrlImporter::importAndSave(const char* savePath) {
    // 1. Выбираем файл со ссылками
    std::string filePath = showFileDialog();
    if (filePath.empty()) {
        std::cout << "[UrlImporter] Отменено пользователем или ошибка диалога." << std::endl;
        return false;
    }

    // 2. Парсим URL из файла
    std::vector<std::string> urls = parseUrlsFromFile(filePath);
    if (urls.empty()) {
        std::cout << "[UrlImporter] В файле не найдено валидных URL." << std::endl;
        return false;
    }

    // 3. Сопоставляем и обновляем память (глобальный filelist)
    if (!matchUrlsToConfig(urls)) {
        std::cout << "[UrlImporter] Совпадений не найдено. Ничего не обновлено." << std::endl;
        return false;
    }

    // 4. Сохраняем обновленный глобальный список в файл
    // ВАЖНО: Эта функция должна быть объявлена у тебя (как в предыдущем ответе)
    // Она пишет в том же формате, что и FileListGenerator::generate
    //saveGlobalFileList(const char* outPath);

    if (saveGlobalFileList(savePath)) {
        std::cout << "[UrlImporter] УСПЕХ! Файл сохранен: " << savePath << std::endl;
        std::cout << "Теперь лаунчер будет качать моды с новых зеркал." << std::endl;
        return true;
    }
    else {
        std::cerr << "[UrlImporter] ОШИБКА: Не удалось сохранить файл." << std::endl;
        return false;
    }
}









/////////////////============================= ОБНОВЛЕНИЕ ДАННЫХ

// Вспомогательная функция: экранирует кавычки и обратные слеши в строке
static void escapeString(const char* src, std::string& dst) {
    dst.clear();
    if (!src) {
        dst = "\"\"";
        return;
    }

    dst += '"';
    for (size_t i = 0; src[i] != '\0'; ++i) {
        char c = src[i];
        if (c == '"') {
            dst += "\\\"";
        }
        else if (c == '\\') {
            dst += "\\\\";
        }
        else {
            dst += c;
        }
    }
    dst += '"';
}

static const char* getModuleTypeStr(SubModuleTypes t) {
    switch (t) {
    case SUBMODULE_TYPE_ANOMALY: return "anomaly";
    case SUBMODULE_TYPE_MO2: return "mo2";
    case SUBMODULE_TYPE_BE:{
        if (sn->umode.project_mode == 0) { return "pz"; }
        else if (sn->umode.project_mode == 1) { return "mp"; }
    } 
    case SUBMODULE_TYPE_EXTRACTOR: return "extractor";
    default: return "unknown";
    }
}

static const char* getFileTypeStr(FileTypes t) {
    return (t == FILE_TYPE_DIR) ? "dir" : "file";
}


// Правильная версия: только оборачиваем в кавычки.
// Никаких замен слэшей внутри!
std::string makeQuotedPath(const std::string& src) {
    if (src.empty()) {
        return "\"\""; // Возвращаем пустые кавычки, если путь пустой
    }
    // Просто добавляем кавычки по краям. 
    // std::string умеет работать с \ внутри строки корректно.
    return "\"" + src + "\"";
}


bool UrlImporter::saveGlobalFileList(const char* outPath) {
    if (!outPath || !strlen(outPath)) {
        Log("[ERROR] Invalid path for filelist generation.");
        return false;
    }

    FILE* f = fopen(outPath, "wb");
    if (!f) {
        Log("[ERROR] Cannot open file for writing.");
        return false;
    }

    fprintf(f, "# Format: ID Filename Dir ModuleType FileType Hash DownloadFilename DownloadUrl\n");
    fprintf(f, "[filelist]\n\n");

    int validCount = 0;

    for (int i = 0; i < iFileListCount; ++i) {
        const stFileList& item = filelist.fileList[i];

        if (item.id == 0 || !item.name || strlen(item.name) == 0) {
            continue;
        }

        std::string nameSrc = (item.name ? std::string(item.name) : "");
        std::string dirSrc = (item.dir ? std::string(item.dir) : "");
        std::string hashSrc = (item.hash ? std::string(item.hash) : "");
        std::string dlNameSrc = (item.downloadfilename ? utf8_to_cp1251(std::string(item.downloadfilename)) : "");
        std::string urlSrc = (item.url ? std::string(item.url) : "");

        std::string m_type = getModuleTypeStr(item.m_type);

        if (dirSrc == "\"\"") dirSrc.clear();;

        // Логирование (оставляем как было)
        Log("Filelist > ID: %d, NAME: %s, DIR (raw): %s, m_type: %s", item.id, nameSrc.c_str(), dirSrc.c_str(), m_type.c_str());

        // Конвертация кодировки (MBCS -> UTF-8)
        std::string nameUtf8 = cp1251_to_utf8(nameSrc);
        std::string dirUtf8 = cp1251_to_utf8(dirSrc);
        std::string hashUtf8 = cp1251_to_utf8(hashSrc);
        std::string dlNameUtf8 = cp1251_to_utf8(dlNameSrc);
        std::string urlUtf8 = cp1251_to_utf8(urlSrc);

        // ВАЖНО: Теперь мы НЕ делаем qName, qDir и т.д. с кавычками внутри.
        // Мы передаём чистые строки, а кавычки добавит fprintf.

        fprintf(f, "%d \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n",
            (int)item.id,
            nameUtf8.c_str(),      // Если пусто -> ""
            dirUtf8.c_str(),       // Если пусто -> "" (именно это тебе и нужно)
            getModuleTypeStr(item.m_type),
            getFileTypeStr(item.f_type),
            hashUtf8.c_str(),
            dlNameUtf8.c_str(),
            urlUtf8.c_str()
        );
        validCount++;
    }

    fclose(f);
    if (ferror(f)) {
        Log("[ERROR] Write failed.");
        return false;
    }

    Log("[UI] Генерация завершена успешно. Записано записей: %d", validCount);
    return true;
}
