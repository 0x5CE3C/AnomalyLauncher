#include "main.h"
#include "UnZipper.h"
#include "miniz.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "..\math_stuff.h"

namespace fs = std::filesystem;

#ifdef _MSC_VER
#include <windows.h>
#include "../main.h"

//detect utf-8
bool is_valid_utf8(const std::string& str) {
    size_t i = 0;
    while (i < str.length()) {
        unsigned char c = str[i];
        if (c < 0x80) {
            i++; // 1 байт
        }
        else if ((c & 0xE0) == 0xC0) {
            // 2 байта
            if (i + 1 >= str.length() || (str[i + 1] & 0xC0) != 0x80) {
                return false;
            }
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0) {
            // 3 байта
            if (i + 2 >= str.length() || (str[i + 1] & 0xC0) != 0x80 || (str[i + 2] & 0xC0) != 0x80) {
                return false;
            }
            i += 3;
        }
        else if ((c & 0xF8) == 0xF0) {
            // 4 байта
            if (i + 3 >= str.length() || (str[i + 1] & 0xC0) != 0x80 || (str[i + 2] & 0xC0) != 0x80 || (str[i + 3] & 0xC0) != 0x80) {
                return false;
            }
            i += 4;
        }
        else {
            return false; // Неправильный байт
        }
    }
    return true; // Корректная строка UTF-8
}

// Функция для проверки, содержит ли строка символы, не поддерживаемые в CP1251
bool is_valid_cp1251(const std::string& str) {
    for (unsigned char c : str) {
        if (c < 0x20 || (c > 0x7E && c < 0xA0) || c > 0xFF) {
            return false; // Неподдерживаемый символ
        }
    }
    return true; // Все символы допустимы
}

// UTF-8 to wstring
std::wstring utf8_to_wstring(const std::string& s)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.size(), nullptr, 0);
    // the same as ws(len, '\0')
    std::wstring ws(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.size(), ws.data(), ws.size());
    return ws;
}

// wstring to UTF-8
std::string wstring_to_utf8(const std::wstring& ws)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.size(), nullptr, 0, nullptr, nullptr);
    // the same as s(len, '\0')
    std::string s(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.size(), s.data(), s.size(), nullptr, nullptr);
    return s;
}
#endif

class UnZipperPrivate {
public:
    explicit UnZipperPrivate() = default;
    void init(UnZipper::OpenMode mode);
    mz_zip_archive_file_stat readFileStat(int index);
    auto archiveFileIndex(const std::string& name);

private:
    friend UnZipper;
    mz_zip_archive archive_{};
    std::string zipName_{};
    UnZipper::OpenMode mode_{ UnZipper::OpenMode::None};
};

void UnZipperPrivate::init(UnZipper::OpenMode mode)
{
    switch (mode) {
        case UnZipper::OpenMode::Create: {
            if (!mz_zip_writer_init_file(&archive_, zipName_.c_str(), 0))
                throw std::runtime_error("Zip writer init failed");
        } break;
        case UnZipper::OpenMode::Read: {
            if (!mz_zip_reader_init_file(&archive_, zipName_.c_str(), 0)) {
                throw std::runtime_error("Failed opening zip archive " + zipName_);
            }
        } break;
        case UnZipper::OpenMode::None:
        default:
            break;
    }
}

mz_zip_archive_file_stat UnZipperPrivate::readFileStat(int index)
{
    mz_zip_archive_file_stat stat;
    if (!mz_zip_reader_file_stat(&archive_, index, &stat)) {
        std::cerr << "Failed read file index " << index
                  << " err: " << mz_zip_get_error_string(mz_zip_get_last_error(&archive_));
        return {};
    }
    return stat;
}

auto UnZipperPrivate::archiveFileIndex(const std::string& name)
{
    int index = mz_zip_reader_locate_file(&archive_, name.c_str(), nullptr, 0);

    if (index < 0) {
        throw std::logic_error(name + " not found");
    }
    return index;
}

UnZipper::UnZipper(const std::string& zipName, OpenMode mode) : d_ptr(std::make_unique<UnZipperPrivate>())
{
    d_ptr->zipName_ = zipName;
    d_ptr->mode_ = mode;
    try {
    d_ptr->init(mode);
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Ошибка при инициализации архива: " << e.what() << std::endl;
        // Можно выбросить исключение дальше, если это необходимо
        throw;
    }
}

UnZipper::UnZipper(const std::string& zipName) : d_ptr(std::make_unique<UnZipperPrivate>())
{
    d_ptr->zipName_ = zipName;
}

UnZipper::~UnZipper()
{
    close();
}

void UnZipper::setmode(UnZipper::OpenMode mode)
{
    d_ptr->mode_ = mode;
    d_ptr->init(mode);
}

bool UnZipper::extract(const std::string& member, const std::string& path)
{
    try {
        int index = d_ptr->archiveFileIndex(member); // Получаем индекс файла из архива

        fs::path dir(path); // Путь, куда будем извлекать
        if (!fs::exists(dir)) {
            fs::create_directories(dir); // Создание директорий, если их нет
        }

        // Создание полного имени файла для извлечения
        fs::path dstPath = dir / member; // Используем fs::path для работы с путями

        // Убедимся, что родительская директория существует
        if (!fs::exists(dstPath.parent_path())) {
            fs::create_directories(dstPath.parent_path());
        }

        // Извлечение файла
        if (!mz_zip_reader_extract_to_file(&d_ptr->archive_, index, dstPath.string().c_str(), 0)) {
            std::cerr << "Ошибка извлечения " << member
                << " из архива. Ошибка: "
                << mz_zip_get_error_string(mz_zip_get_last_error(&d_ptr->archive_)) << std::endl;
            return false;
        }

        return true; // Успешное извлечение
    }
    catch (const std::logic_error& e) {
        std::cerr << "Ошибка извлечения " << member << ": " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Общая ошибка извлечения: " << e.what() << std::endl;
        return false;
    }
}

void UnZipper::extractall(const std::string& path)
{
    try {
        fs::path dstPath(path); // Используем только std::string
        if (!fs::exists(dstPath)) {
            fs::create_directories(dstPath); // Создание директорий, если их нет
        }

        auto nums = mz_zip_reader_get_num_files(&d_ptr->archive_);
        for (unsigned int i = 0; i < nums; i++) {
            mz_zip_archive_file_stat stat{};
            if (!mz_zip_reader_file_stat(&d_ptr->archive_, i, &stat)) {
                std::cerr << "Ошибка чтения файла из архива. Ошибка: "
                    << mz_zip_get_error_string(mz_zip_get_last_error(&d_ptr->archive_)) << std::endl;
                continue; // Пропускаем файл в случае ошибки
            }

            // Создание директории, если это директория
            if (stat.m_is_directory) {
                fs::path dir = dstPath / stat.m_filename; // Прямое использование
                fs::create_directories(dir); // Создание всех необходимых директорий
            }
            else {
                // Полное имя файла для извлечения
                std::string outFilePath = (dstPath / stat.m_filename).string(); // Строим путь к файлу

                

                // Создание родительской директории, если ее нет
                if (!fs::exists(fs::path(outFilePath).parent_path())) {
                    fs::create_directories(fs::path(outFilePath).parent_path());
                }

                // Логирование извлечения
                sn->con.AddLog("Распаковка: %s -> %s", d_ptr->zipName_.c_str(), outFilePath.c_str());
                Log("[UnZipper]: Extracted %s -> %s", d_ptr->zipName_.c_str(), outFilePath.c_str());

                // Извлечение файла
                if (!mz_zip_reader_extract_to_file(&d_ptr->archive_, stat.m_file_index, outFilePath.c_str(), 0)) {
                    std::cerr << "Ошибка извлечения " << stat.m_filename << " из архива. Ошибка: "
                        << mz_zip_get_error_string(mz_zip_get_last_error(&d_ptr->archive_)) << std::endl;
                }
            }
        }
        sn->con.AddLog("Распаковка завершена");
        unpacked_status = 1;
        Log("Статус распаковки: %i", unpacked_status);

        mz_zip_reader_end(&d_ptr->archive_); // Закрываем архив
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при извлечении всех файлов: " << e.what() << std::endl;
        close(); // Обязательно закрыть архив в случае ошибки
    }
}

void UnZipper::close()
{    
    try {
    mz_zip_end(&d_ptr->archive_);
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при закрытии архива: " << e.what() << std::endl;
    }
}