#include "main.h"


int iSubModuleListCount = 0;
int iFileListCount = 0;
int iPatchListCount = 0;
int iTaskListCount = 0;
struct stfsubmodulelist submodulelist;
struct stffilelist filelist;
struct stftasklist tasklist;
struct stfpatchlist patchlist;

static int parse_int(const char* str)
{
	return strtol(str, NULL, 0);
}

// Функция для определения типа секции
int getType(const char* section) {
	if (strcmp(section, "filelist") == 0) {		
		memset(&filelist, 0, sizeof(filelist));
		return LIST_TYPE_FILELIST;
	}
	/*else if (strcmp(section, "modulelist") == 0) {
		memset(&submodulelist, 0, sizeof(submodulelist));
		return LIST_TYPE_SUBMODULELIST;
	}*/
	// Добавить другие типы по необходимости
	return -1; // Ошибка
}

// Функция для получения типа модуля по строке
SubModuleTypes getSubModuleType(const std::string& typeStr) {
	if (typeStr == "anomaly") return SUBMODULE_TYPE_ANOMALY;
	if (typeStr == "mo2") return SUBMODULE_TYPE_MO2;

	if (sn->umode.universal_mode.enabled)
	{
		if (typeStr == "mp") return SUBMODULE_TYPE_BE;
	}
	else
	{
		if (typeStr == "pz") return SUBMODULE_TYPE_BE;
	}

	if (typeStr == "extractor") return SUBMODULE_TYPE_EXTRACTOR;

	return static_cast<SubModuleTypes>(SUBMODULE_TYPE_UNKNOWN); // Неизвестный тип
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


std::string concatenateValues(const std::vector<VarValue>& values, size_t maxCount) {
    std::string concatenated;
    size_t count = min(values.size(), maxCount);

    for (size_t i = 0; i < count; ++i) {
        std::string value = normalizeSpaces(values[i].AsString());
        Log("value parse: %s", value.c_str());
        concatenated += value;
        if (i < count - 1) {
            concatenated += " ";
        }
    }
    return concatenated;
}

void list_parse(const std::string& filename) {
	traceLastFunc("void list_parse(const std::string& filename);");

	// Инициализация с использованием std::unique_ptr
	auto list = listparser::init();
	if (!list) {
		Log(" Failed to init listparser");
		return;
	}

	try {
		list->load(filename); // Загрузка файла конфигурации
	}
	catch (const std::runtime_error& e) {
		Log(" Ошибка загрузки файла '%s': %s", filename.c_str(), e.what());
		return;
	}

	std::vector<std::string> sections = list->getAllSections();
	if (sections.empty()) {
		Log(" Нет секций в файле '%s'", filename.c_str());
		return;
	}

	for (const std::string& section : sections) {
		Log(" Обработка секции: %s", section.c_str());

		int type = getType(section.c_str());
		if (type == -1) {
			Log(" Неизвестный тип секции: %s", section.c_str());
			continue;
		}

		for (int idx = 1; idx < MAX_ID; idx++) {
			try {
				std::vector<VarValue> values = list->getValuesByID(section.c_str(), idx);

				// Пропускаем пустые ID
				if (values.empty()) {
					continue;
				}

				// Логируем все поля корректно: по индексам 0..N
				/*Log("=== ID %d ===", idx);
				for (size_t i = 0; i < values.size(); ++i) {
					Log("  [%d] = '%s'", i, utf8_to_cp1251(values[i].AsString()).c_str());
				}*/

				switch (type) {
				case LIST_TYPE_FILELIST: {
					if (values.size() < 5) continue;

					int id = parse_int(values[0].AsString().c_str());
					if (id != idx || id >= MAX_ID || iFileListCount >= MAX_ID) continue;

					stFileList* f = &filelist.fileList[iFileListCount];
					f->id = id;

					// === Собираем name — если начинается с ", ищем закрывающую " ===
					std::string name;
					int pos = 1;
					if (!values[1].AsString().empty() && values[1].AsString()[0] == '"') {
						bool closed = false;
						for (int i = 1; i < values.size(); ++i) {
							std::string part = utf8_to_cp1251(values[i].AsString());
							// Убираем кавычки в начале и конце
							if (part[0] == '"') part = part.substr(1);
							if (!part.empty() && part.back() == '"') {
								part.pop_back();
								closed = true;
							}
							name += part + " ";
							pos = i + 1;
							if (closed) break;
						}
						if (!name.empty()) name.pop_back(); // убираем последний пробел
					}
					else {
						name = values[1].AsString();
						pos = 2;
					}

					f->name = my_strdup(name.c_str());

					// === Остальные поля — начиная с pos ===
					if (pos + 4 >= values.size()) {
						Log(" Мало полей для ID %d", id);
						continue;
					}

					f->dir = my_strdup(values[pos].AsString().c_str());
					f->m_type = getSubModuleType(values[pos + 1].AsString());
					f->f_type = (_stricmp(values[pos + 2].AsString().c_str(), "dir") == 0) ? FILE_TYPE_DIR : FILE_TYPE_FILE;

					std::string hash = values[pos + 3].AsString();
					if (hash == "\"\"" || hash == "''" || hash.empty()) hash = "";
					f->hash = my_strdup(hash.c_str());

					std::string dl_name = values[pos + 4].AsString();
					if (dl_name == "\"\"" || dl_name == "''" || dl_name.empty()) dl_name = "";
					f->downloadfilename = my_strdup(dl_name.c_str());

					if (pos + 5 < values.size()) {
						std::string url = values[pos + 5].AsString();
						if (url == "\"\"" || url == "''" || url.empty()) url = "";
						f->url = my_strdup(url.c_str());
					}
					else {
						f->url = my_strdup("");
					}

					// === dl_type по расширению ===
					//f->dl_type = DL_SUB_MODULE_TYPE_UNKNOWN;
					if (!std::string(f->downloadfilename).empty()) {
						std::string dl = f->downloadfilename;
						if (dl.find(".zip") != std::string::npos) {
							f->dl_type = DL_SUB_MODULE_TYPE_ZIP;
						}
						else if (dl.find(".exe") != std::string::npos) {
							f->dl_type = DL_SUB_MODULE_TYPE_EXE;
						}
					}

					iFileListCount++;
					break;
				}

									   // Пример заглушки для других типов (можно расширить)
									   // case LIST_TYPE_SUBMODULELIST: { ... }

				default:
					Log(" Необработанный тип секции: %s", section.c_str());
					break;
				}
			}
			catch (const std::exception& e) {
				Log(" Исключение при обработке ID %d в секции %s: %s", idx, section.c_str(), e.what());
				continue;
			}
			catch (...) {
				Log(" Неизвестная ошибка при обработке ID %d", idx);
				continue;
			}
		}
	}

	// Финальный лог — сколько записей загружено
	Log(" Успешно загружено %d записей в filelist", iFileListCount);
}

void str_split_free(struct str_split* split)
{
	if (split != NULL)
	{
		if (split->str != NULL)
			free(split->str);
		if (split->argv != NULL)
			free(split->argv);
		free(split);
	}
}

struct str_split* str_split(const char* str, const char* ch)
{
	struct str_split* split;
	char* prev, * next;
	char* dest;
	void* tmp;

	/* left trim */
	while (*str && strchr(ch, *str) != NULL)
		str++;

	split = (struct str_split*)calloc(1, sizeof(struct str_split));
	if (split == NULL)
		return NULL;

	split->str = _strdup(str);
	if (split->str == NULL)
	{
		free(split);
		return NULL;
	}

	for (prev = split->str;; prev = next)
	{
		if (*prev == '"')
		{
			/* find the ending " */
			for (dest = next = ++prev; *next; next++)
			{
				if (*next == '"')
				{
					next++;
					break;
				}
				else if (*next == '\\' && next[1] != 0)
					*dest++ = *++next;
				else
					*dest++ = *next;
			}

			*dest = 0;
		}
		else
		{
			next = prev;
		}

		/* find next value */
		for (; *next && strchr(ch, *next) == NULL; next++);
		next = *next ? next : NULL;

		if ((tmp = realloc(split->argv, (split->argc + 1) * sizeof(char*))) == NULL)
			goto out;
		split->argv = (char**)tmp;

		split->argv[split->argc] = prev;
		split->argc++;

		if (next == NULL)
			break;

		for (*next++ = 0; *next && strchr(ch, *next) != NULL; next++);

		if (*next == 0)
			break;
	}

	tmp = realloc(split->argv, (split->argc + 1) * sizeof(char*));
	if (tmp == NULL)
		goto out;
	split->argv = (char**)tmp;

	split->argv[split->argc] = NULL;

	return split;

out:;
	str_split_free(split);
	return NULL;
}


size_t strlcat(char* dst, const char* src, size_t size)
{
	size_t	dlen = strlen(dst);
	size_t	slen = strlen(src);

	if (size == 0)
		return dlen + slen;

	if (dlen + slen >= size)
	{
		size -= dlen - 1;
		memcpy(dst + dlen, src, size);
		dst[dlen + size] = 0;
	}
	else if (size > 0)
	{
		strcpy(dst + dlen, src);
	}

	return dlen + slen;
}

size_t strlcpy(char* dst, const char* src, size_t size)
{
	size_t	len = strlen(src);

	if (size == 0)
		return len;

	if (len >= size)
	{
		size--;
		memcpy(dst, src, size);
		dst[size] = 0;
	}
	else if (size > 0)
	{
		strcpy(dst, src);
	}

	return len;
}