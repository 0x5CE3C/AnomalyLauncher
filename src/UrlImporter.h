#pragma once
#include <string>
#include <vector>

// Подключаем файл, где реально определена структура stFileList
#include "lists.h" 

// Говорим компилятору, что эти глобальные переменные существуют где-то в другом .cpp файле
extern struct stffilelist filelist;
extern int iFileListCount;

inline const char* SubModuleTypeToString(SubModuleTypes type) {
    switch (type) {
    case SUBMODULE_TYPE_UNKNOWN: return "unknown";
    case SUBMODULE_TYPE_ANOMALY: return "anomaly";
    case SUBMODULE_TYPE_MO2: return "mo2";
    case SUBMODULE_TYPE_BE: return "be";
    case SUBMODULE_TYPE_EXTRACTOR: return "extractor";
    default: return "unknown";
    }
}

inline const char* FileTypeToString(FileTypes type) {
    switch (type) {
    case FILE_TYPE_FILE: return "file";
    case FILE_TYPE_DIR: return "dir";
    default: return "file";
    }
}

// Вспомогательная функция (если её нет в твоих утилитах)
char* my_strdup(const char* src);

class UrlImporter {
public:
    // Основной метод: открывает диалог, читает файл со ссылками, обновляет URL в памяти
    bool importAndSave(const char* savePath);

    // Вспомогательный: просто парсит файл со ссылками (без сохранения)
    std::vector<std::string> parseUrlsFromFile(const std::string& filename);

    // Сопоставляет найденные URL с текущим filelist в памяти
    bool matchUrlsToConfig(const std::vector<std::string>& urls);

private:
    std::string showFileDialog();

    bool saveGlobalFileList(const char* outPath);
};
