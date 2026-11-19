#ifndef INI_PARSER_HPP
#define INI_PARSER_HPP

#include <string>     // for std::string
#include <vector>     // for std::vector
#include <map>        // for std::map
#include <sstream>    // for std::stringstream
#include <algorithm>  // for std::transform
#include <cctype>     // for std::isspace()
#include <fstream>    // for std::fstream
#include <stdexcept>  // for std::runtime_error
#include <iostream>   // for std::cout, std::cerr
#include <set>        // for std::set
#include <variant>

class VarValue {
public:
    using VarType = std::variant<int, float, bool, char, std::string>;

    VarValue(const std::string& value) : value(value) {}

    int AsInt() const noexcept {
        return std::stoi(value);
    }

    float AsFloat() const noexcept {
        return std::stof(value);
    }

    std::string AsString() const noexcept {
        return value;
    }

    char AsChar() const noexcept {
        if (value.length() == 1) {
            return value[0];
        }
        throw std::runtime_error("Невозможно преобразовать в char: значение не является одиночным символом.");
    }

    bool AsBool() const noexcept {
        if (value == "true") return true;
        if (value == "false") return false;
        throw std::runtime_error("Невозможно преобразовать в bool: значение не 'true' или 'false'.");
    }

private:
    std::string value;

    bool isInteger(const std::string& str) const noexcept {
        return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
    }

    bool isFloat(const std::string& str) const noexcept {
        std::istringstream iss(str);
        float f;
        iss >> f;
        return iss.eof() && !iss.fail();
    }
};

class listparser {
private:
    static bool isNumeric(const std::string& str) {
        std::istringstream iss(str);
        double dummy;

        iss >> std::noskipws >> dummy; // Избегаем пропуска пробелов

        if (iss.eof() && !iss.fail()) {
            return true; // Успех: строка является числом
        }

        char c;
        if (iss >> c) {
            return false; // Если есть дополнительные символы после числа
        }
        return true; // Только числовая строка
    }

public:
    static std::unique_ptr<listparser> init() {
        return std::unique_ptr<listparser>(new listparser());
    }

    void clrdata() noexcept {
        data.clear();
    }

    bool is_open(const std::string& filename) const {
        std::ifstream file(filename);
        return file.is_open(); // Возвращает true, если файл успешно открыт
    }

    bool file_exists(const std::string& filename) const {
        std::ifstream file(filename);
        return file.good(); // Возвращает true, если файл существует и доступен
    }

    bool hasSection(const std::string& section) const {
        return data.find(section) != data.end();
    }

    void load(const std::string& filename) {
        if (!file_exists(filename)) {
            throw std::runtime_error("Файл не найден: " + filename);
        }
        if (!is_open(filename)) {
            throw std::runtime_error("Не удалось открыть файл для чтения: " + filename);
        }
        else
        {        
            //Log("Файл открыт: %s", filename);
            std::ifstream file(filename);
            std::string line, section;

            while (std::getline(file, line)) {
                line = removeComments(line);
                //line = trim(line);
                trim(line);
            
                if (line.empty()) continue; // Игнорируем пустые строки
            

                if (line[0] == '[') {
                    section = line.substr(1, line.find(']') - 1);
                    if (data.find(section) != data.end()) {
                        throw std::runtime_error("Секция уже существует: " + section);
                    }
                    data[section]; // Инициализация секции
                    Log("Добавлена секция: %s", section.c_str());
                }
                else {
                    processKeyValueLine(line, section);
                }
            }
            file.close();
            //Log("Файл закрыт: %s", filename);
        }
    }

    std::vector<VarValue> getValuesByID(const std::string& section, int id) const {
        std::string key = std::to_string(id);
        auto secIt = data.find(section);
        if (secIt == data.end()) return {};

        auto keyIt = secIt->second.find(key);
        if (keyIt == secIt->second.end()) return {};

        std::vector<VarValue> result;
        for (const std::string& val : keyIt->second) {
            result.emplace_back(val);
        }
        return result;
    }    

    std::vector<std::string> getAllSections() const {
        std::vector<std::string> sections;
        for (const auto& pair : data) {
            sections.push_back(pair.first);
        }
        return sections;
    }    

private:
    std::map<std::string, std::map<std::string, std::vector<std::string>>> data;

    /* NEW VER*/
    std::string parseQuotedString(std::istringstream& ss) const {
        std::string result;
        char quoteChar = ss.peek();

        if (quoteChar == '"' || quoteChar == '\'') {
            ss.get(); // Убираем кавычку

            std::getline(ss, result, quoteChar); // Считываем всё до закрывающей кавычки

            // Если это URL, удаляем пробелы вокруг знаков равно
            if (result.find("http://") == 0 || result.find("https://") == 0) {
                // Убираем пробелы вокруг знаков "="
                size_t pos = 0;
                while ((pos = result.find('=', pos)) != std::string::npos) {
                    // Удаляем пробелы перед и после знака '='
                    size_t start = result.find_last_not_of(' ', pos - 1); // Находим последний непробельный символ перед '='
                    size_t end = result.find_first_not_of(' ', pos + 1); // Находим первый непробельный символ после '='

                    if (start != std::string::npos && end != std::string::npos) {
                        result.erase(start + 1, pos - (start + 1)); // Убираем пробелы перед '='
                        result.erase(pos + 1, end - (pos + 1)); // Убираем пробелы после '='
                    }
                    pos++; // Сдвигаем позицию
                }
            }

            return result.empty() ? "\"\"" : result; // Если строка пустая, возвращаем пустую строку
        }
        else {
            ss >> result; // Считываем следующее слово
        }

        return result.empty() ? "\"\"" : result;
    }

    std::string normalizeSpaces(const std::string& input) {
        std::istringstream iss(input);
        std::ostringstream oss;
        std::string word;
        bool first = true;
        while (iss >> word) {
            if (!first) oss << ' ';
            oss << word;
            first = false;
        }
        return oss.str();
    }

    void processKeyValueLine(const std::string& line, const std::string& section) {
        std::istringstream ss(line);
        std::string key;

        if (!(ss >> key) || !isInteger(key)) {
            throw std::runtime_error("Первое значение должно быть целым числом: " + key);
        }

        if (data[section].find(key) != data[section].end()) {
            throw std::runtime_error("Ключ уже существует: " + key + " в секции: " + section);
        }

        std::vector<std::string> values;
        values.push_back(key);

        std::string token;
        while (ss) {
            token = parseQuotedString(ss);
            if (!token.empty()) {
                trim(token);
                token = normalizeSpaces(token);
                values.push_back(std::move(token));
            }
            ss >> std::ws;
            if (ss.eof()) break;
        }

        data[section][key] = std::move(values);       
    }

    void trim(std::string& str) const {
        const char* whitespace = " \t\n\r";
        str.erase(0, str.find_first_not_of(whitespace));
        str.erase(str.find_last_not_of(whitespace) + 1);
    }

    bool isInteger(const std::string& str) const {
        return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
    }

    string removeComments(const string& line) {
        string res;
        bool m_cmt = false; // Флаг многострочного комментария

        for (size_t i = 0; i < line.length(); ++i) {
            // Начало многострочного комментария
            //if (!m_cmt && line[i] == '/' && i + 1 < line.length() && line[i + 1] == '*') {
            //    m_cmt = true;
            //    return ""; // Полностью удаляем строку, если начался многострочный комментарий
            //}

            // Конец многострочного комментария
            //if (m_cmt && line[i] == '*' && i + 1 < line.length() && line[i + 1] == '/') {
            //    m_cmt = false;
            //    return ""; // Полностью удаляем строку после закрытия комментария
            //}

            // Пропуск символов внутри многострочного комментария
            //if (m_cmt) continue;

            // Начало однострочного комментария
            if (line[i] == '#' && i + 1 < line.length() /*&& line[i + 1] == '/'*/) {
                break; // Пропускаем остаток строки
            }

            // Добавляем символ, если это не комментарий
            res += line[i];
        }

        // Удаляем пробелы справа
        while (!res.empty() && isspace(res.back())) {
            res.pop_back();
        }

        return res;
    }

   

};

#endif // LIST_PARSER_HPP
