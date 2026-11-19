#include "main.h"
#include "L_ModPack_Helper.h"

void Request_CheckAndCleanInvalidHashMods() {
	static bool showConfirm = false;

	if (pMenu->AddStaticButton(u8"Проверить и очистить невалидные моды")) {
		showConfirm = true;
	}

	if (showConfirm) {

		ImGui::Begin(u8"Подтверждение", &showConfirm, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
		
		ImGui::TextWithColors(u8"Подтверждение");

		ImGui::Separator();

		if (ImGui::Button(u8"Сканировать", ImVec2(200, 30))) {
			auto thw = ThreadWrapper::create();
			thw->beginAsync(Debug_CheckAndCleanInvalidHashMods);
			showConfirm = false;
		}

		ImGui::SameLine();
		if (ImGui::Button(u8"Отмена", ImVec2(100, 30))) {
			showConfirm = false;
		}

		ImGui::End();
	}
};

//Work
void Debug_CheckAndCleanInvalidHashMods()
{
	sn->con.AddLog("Диагностика: проверка хэшей валидных модов %s", (sn->umode.universal_mode.enabled == true ? cp1251_to_utf8(sn->umode.modpack_name).c_str() : u8"Purge of Zone"));

	bool foundInvalid = false;
	int totalChecked = 0;

	for (int i = 0; i < iFileListCount; i++) {
		if (filelist.fileList[i].m_type != SUBMODULE_TYPE_BE) {
			continue;
		}

		const char* modName = filelist.fileList[i].name;
		std::string pathToCheck = GetDirectoryPath(i);

		if (!fsm->DirectoryExists(pathToCheck.c_str())) {
			sn->con.AddLog(" Пропущен: папка не найдена — %s", modName);
			continue;
		}

		totalChecked++;
		

		std::string calculatedHash = md5_files_in_directory(pathToCheck.c_str());
		const char* expectedHash = filelist.fileList[i].hash;

		if (strcmp(expectedHash, calculatedHash.c_str()) == 0) {
			sn->con.AddLog(" Хэш совпал: {00BB00}%s", cp1251_to_utf8(pathToCheck.c_str()).c_str());
		}
		else {
			sn->con.AddLog(" Хэш НЕ совпал: {FFFF00}%s", cp1251_to_utf8(pathToCheck.c_str()).c_str());
			if (RemoveModFromDisk(pathToCheck)) {
				sn->con.AddLog("   Успешно удалён.");
				foundInvalid = true;
			}
			else {
				sn->con.AddLog("   Ошибка удаления!");
			}
		}
	}

	if (totalChecked == 0) {
		sn->con.AddLog("Не найдено ни одного мода для проверки.");
	}
	else if (!foundInvalid) {
		sn->con.AddLog("Все моды имеют корректный хэш — очистка не требуется.");
	}
	else {
		sn->con.AddLog("Очистка завершена: повреждённые моды удалены.");
	}

	sn->con.AddLog("Диагностика завершена.");
}


//Work
void Debug_ListExternalModsV2()
{
	std::string baseDirectory = GetMO2Folder() + "\\mods";
	sn->con.AddLog("{FFFF00}Диагностика: анализ папки mods: %s", baseDirectory.c_str());

	if (!fsm->FolderExist(baseDirectory.c_str())) {
		sn->con.AddLog("{AAFFFF}Папка mods не найдена: %s", baseDirectory.c_str());
		return;
	}

	sn->con.AddLog("{FFFF00}Папка mods существует. Начинаю чтение...");

	std::vector<std::string> allFolders;
	try {
		//sn->con.AddLog(" Открываю directory_iterator...");

		for (const auto& entry : std::filesystem::directory_iterator(
			baseDirectory,
			std::filesystem::directory_options::skip_permission_denied))
		{
			std::string name = entry.path().filename().string();
			//sn->con.AddLog(" Найден мод: %s", name.c_str());

			if (entry.is_directory()) {
				allFolders.push_back(name);
				//sn->con.AddLog(" Найден мод: %s", name.c_str());
			}
		}

		sn->con.AddLog("{FFFF00} Сканирование завершено. Найдено папок: %zu", allFolders.size());
	}
	catch (const std::filesystem::filesystem_error& e) {
		sn->con.AddLog(" Ошибка filesystem: %s", e.what());
	}
	catch (...) {
		//sn->con.AddLog(" Неизвестная ошибка при чтении папки mods");
	}

	// Показываем, какие моды разрешены
	std::set<std::string> allowedMods;
	sn->con.AddLog("{FFFF00} Анализ: проверка белого списка из filelist...");
	for (int i = 0; i < iFileListCount; i++) {
		if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE) {
			allowedMods.insert(filelist.fileList[i].name);
			sn->con.AddLog("{00FF00} Валидный мод: {FFFFFF}%s", filelist.fileList[i].name);
		}
	}
	sn->con.AddLog("{FFFF00} Всего валидных модов: {FFFFFF}%zu", allowedMods.size());

	// Показываем сторонние	
	bool foundExternal = false;
	for (const std::string& folder : allFolders) {
		if (allowedMods.find(folder) == allowedMods.end()) {
			sn->con.AddLog("{AAFFFF} Сторонний мод: %s", folder.c_str());
			foundExternal = true;
		}
	}
	if (!foundExternal) {
		sn->con.AddLog("{FFFF00} Нет сторонних модов — всё чисто!");
	}

	sn->con.AddLog("{00FF00} Диагностика завершена.");
}


int getNextId() {
	int maxId = 0;
	for (int i = 0; i < iFileListCount; i++) {
		if (filelist.fileList[i].id > maxId) {
			maxId = filelist.fileList[i].id;
		}
	}
	return maxId + 1;
}


void Debug_ExportValidModsHashes()
{
	sn->con.AddLog("Экспорт хэшей: сбор списка хэшей для валидных модов %s", (sn->umode.universal_mode.enabled == true ? cp1251_to_utf8(sn->umode.modpack_name).c_str() : u8"Purge of Zone"));

	bool foundAny = false;
	static int ids_size = getNextId();

	for (int i = 0; i < iFileListCount; i++) {
		if (filelist.fileList[i].m_type != SUBMODULE_TYPE_BE) {
			continue;
		}

		const char* modName = filelist.fileList[i].name;
		std::string pathToCheck = GetDirectoryPath(i);

		if (!fsm->DirectoryExists(pathToCheck.c_str())) {
			sn->con.AddLog(" Пропущен: папка не найдена — %s", modName);
			continue;
		}

		std::string hash = md5_files_in_directory(pathToCheck.c_str());

		sn->con.AddLog("%i \"%s\" \"MO2\\\\mods\" \"pz\" \"dir\" \"%s\" \"\" \"\"", ids_size, modName, hash.c_str());		

		foundAny = true;
		ids_size++;
	}

	ids_size = getNextId();

	if (!foundAny) {
		sn->con.AddLog("Не найдено ни одного валидного мода для экспорта.");
	}
	else {
		sn->con.AddLog("Экспорт хэшей завершён. Готово для копирования.");
	}
}

void Debug_ConvertExternalModsToFilelist()
{
	std::string modsPath = GetMO2Folder() + "\\mods";
	sn->con.AddLog("{FFFF00}Конвертер: поиск сторонних модов в папке %s", modsPath.c_str());

	if (!fsm->FolderExist(modsPath.c_str())) {
		sn->con.AddLog("{AAFFFF}Папка mods не найдена!");
		return;
	}

	// Собираем имена всех валидных модов из filelist
	std::set<std::string> validModNames;
	for (int i = 0; i < iFileListCount; i++) {
		if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE) {
			validModNames.insert(filelist.fileList[i].name);
		}
	}
	sn->con.AddLog("Загружено %zu валидных модов из filelist", validModNames.size());

	// Сканируем папку mods
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile((modsPath + "\\*").c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		sn->con.AddLog("Не удалось открыть папку mods.");
		return;
	}

	bool foundAny = false;
	static int ids_size = getNextId();

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			std::string folderName = ffd.cFileName;

			// Пропускаем . и ..
			if (folderName == "." || folderName == "..") {
				continue;
			}

			// Проверяем, есть ли такой мод в filelist
			if (validModNames.find(folderName) != validModNames.end()) {
				continue; // Это валидный мод — пропускаем
			}

			// Это сторонний мод
			std::string fullPath = modsPath + "\\" + folderName;

			//std::string hash = md5_files_in_directory(fullPath.c_str());

			sn->con.AddLog("%i \"%s\" \"MO2\\\\mods\" \"pz\" \"dir\" \"\" \"\" \"\"", ids_size, folderName.c_str());
			foundAny = true;
			ids_size++;

		}
	} while (FindNextFile(hFind, &ffd) != 0);

	ids_size = getNextId();

	FindClose(hFind);

	if (!foundAny) {
		sn->con.AddLog("{FFFF00}Не найдено ни одного стороннего мода.");
	}
	else {
		sn->con.AddLog("{00FF00}Конвертация сторонних модов завершена. Готово для копирования.");
	}
}

bool RemoveModFromDisk(const std::string& modPath)
{
	std::string modName = std::filesystem::path(modPath).filename().string();

	// Проверяем — существует ли папка
	if (!fsm->FolderExist(modPath.c_str())) {
		sn->con.AddLog("Мод уже отсутствует: %s", modName.c_str());
		return true; // можно считать "успешно"
	}

	// Запоминаем состояние
	bool existedBefore = true;
	sn->con.AddLog("Удаляю мод: %s", modName.c_str());

	// Вызываем твой void-метод
	fsm->DeleteFolderWithFiles(modPath.c_str());

	// Проверяем — исчезла ли папка
	if (!fsm->FolderExist(modPath.c_str())) {
		sn->con.AddLog(" Мод удалён: %s", modName.c_str());
		return true;
	}
	else {
		sn->con.AddLog(" Не удалось удалить мод: %s", modName.c_str());
		return false;
	}
}


std::string GetFileNameFromPath(const std::string& path) {
	size_t pos = path.find_last_of("/\\");
	return (pos != std::string::npos) ? path.substr(pos + 1) : path;
}


void Debug_CheckAndCleanAnomalyItems()
{
	sn->con.AddLog("Диагностика: проверка файлов и папок ANOMALY (1)");

	bool foundInvalid = false;
	int totalChecked = 0;

	for (int i = 0; i < iFileListCount; i++) {
		if (filelist.fileList[i].m_type != SUBMODULE_TYPE_ANOMALY) {
			continue;
		}

		const char* itemName = filelist.fileList[i].name;
		std::string pathToCheck;
		bool isDirectory = (filelist.fileList[i].dir[0] != '"');

		if (isDirectory) {
			pathToCheck = GetDirectoryPath(i);
		}
		else {
			pathToCheck = GetFilePath(i);
		}

		bool exists = isDirectory ?
			fsm->DirectoryExists(pathToCheck.c_str()) :
			fsm->FileExists(pathToCheck.c_str());

		if (!exists) {
			sn->con.AddLog(" Пропущен: не найден — %s", itemName);
			continue;
		}

		// ?? Игнорируем папку MO2
		if (isDirectory) {
			std::string folderName = GetFileNameFromPath(pathToCheck);
			if (folderName == "MO2") {
				sn->con.AddLog(" Пропущен: сторонний менеджер — %s", itemName);
				continue;
			}
		}

		totalChecked++;

		bool shouldCheck = isDirectory ? sn->scanhashes_dirs.enabled : sn->scanhashes.enabled;
		if (!shouldCheck) {
			sn->con.AddLog(" Проверка отключена — пропуск: %s", itemName);
			continue;
		}

		std::string calculatedHash;
		if (isDirectory) {
			calculatedHash = md5_files_in_directory(pathToCheck.c_str());
		}
		else {
			calculatedHash = md5file(pathToCheck.c_str());
		}

		if (strcmp(filelist.fileList[i].hash, calculatedHash.c_str()) == 0) {
			sn->con.AddLog(" Хэш совпал: {00BB00}%s", cp1251_to_utf8(pathToCheck.c_str()).c_str());
		}
		else {
			sn->con.AddLog(" Хэш НЕ совпал: {FFFF00}%s", cp1251_to_utf8(pathToCheck.c_str()).c_str());
			if (RemoveModFromDisk(pathToCheck)) {
				sn->con.AddLog("   Успешно удалён.");
				foundInvalid = true;
			}
			else {
				sn->con.AddLog("   Ошибка удаления!");
			}
		}
	}

	if (totalChecked == 0) {
		sn->con.AddLog("Нет элементов ANOMALY для проверки.");
	}
	else if (!foundInvalid) {
		sn->con.AddLog("Все элементы ANOMALY имеют корректный хэш.");
	}
	else {
		sn->con.AddLog("Очистка повреждённых элементов ANOMALY завершена.");
	}
}

void Debug_CheckAndCleanMO2Items()
{
	sn->con.AddLog("Диагностика: проверка файлов и папок MO2 (2)");

	bool foundInvalid = false;
	int totalChecked = 0;

	for (int i = 0; i < iFileListCount; i++) {
		if (filelist.fileList[i].m_type != SUBMODULE_TYPE_MO2) {
			continue;
		}

		const char* itemName = filelist.fileList[i].name;
		std::string pathToCheck;
		bool isDirectory = (filelist.fileList[i].dir[0] != '"');

		if (isDirectory) {
			pathToCheck = GetDirectoryPath(i);
		}
		else {
			pathToCheck = GetFilePath(i);
		}

		bool exists = isDirectory ?
			fsm->DirectoryExists(pathToCheck.c_str()) :
			fsm->FileExists(pathToCheck.c_str());

		if (!exists) {
			sn->con.AddLog(" Пропущен: не найден — %s", itemName);
			continue;
		}

		// ?? Игнорируем папку mods
		if (isDirectory) {
			std::string folderName = GetFileNameFromPath(pathToCheck);
			if (folderName == "mods") {
				sn->con.AddLog(" Пропущен: контейнер модов — %s", itemName);
				continue;
			}
		}

		totalChecked++;

		bool shouldCheck = isDirectory ? sn->scanhashes_dirs.enabled : sn->scanhashes.enabled;
		if (!shouldCheck) {
			sn->con.AddLog(" Проверка отключена — пропуск: %s", itemName);
			continue;
		}

		std::string calculatedHash;
		if (isDirectory) {
			calculatedHash = md5_files_in_directory(pathToCheck.c_str());
		}
		else {
			calculatedHash = md5file(pathToCheck.c_str());
		}

		if (strcmp(filelist.fileList[i].hash, calculatedHash.c_str()) == 0) {
			sn->con.AddLog(" Хэш совпал: {00BB00}%s", cp1251_to_utf8(pathToCheck.c_str()).c_str());
		}
		else {
			sn->con.AddLog(" Хэш НЕ совпал: {FFFF00}%s", cp1251_to_utf8(pathToCheck.c_str()).c_str());
			if (RemoveModFromDisk(pathToCheck)) {
				sn->con.AddLog("   Успешно удалён.");
				foundInvalid = true;
			}
			else {
				sn->con.AddLog("   Ошибка удаления!");
			}
		}
	}

	if (totalChecked == 0) {
		sn->con.AddLog("Нет элементов MO2 для проверки.");
	}
	else if (!foundInvalid) {
		sn->con.AddLog("Все элементы MO2 имеют корректный хэш.");
	}
	else {
		sn->con.AddLog("Очистка повреждённых элементов MO2 завершена.");
	}
}

bool IsLikelyFile(const char* name) {
	const char* ext = strrchr(name, '.');
	if (!ext) return false;
	ext++; // пропускаем точку

	const char* extensions[] = {
		"exe", "dll", "db0", "db1", "db2", "db3", "db4",
		"mhr", "ini", "ltx", "txt", "cfg", "so", "dylib", "bin"
	};

	for (int i = 0; i < 13; i++) {
		if (_stricmp(ext, extensions[i]) == 0) {
			return true;
		}
	}
	return false;
}

void Debug_ExportAnomalyHashes()
{
	sn->con.AddLog("Экспорт хэшей: ANOMALY (1) — начало");

	bool foundAny = false;

	for (int i = 0; i < iFileListCount; i++) {
		if (filelist.fileList[i].m_type != SUBMODULE_TYPE_ANOMALY) {
			continue;
		}

		const char* itemName = filelist.fileList[i].name;
		std::string pathToCheck;
		bool isDirectory = (filelist.fileList[i].dir[0] != '"');
		bool isActuallyFile = false;

		// ?? Коррекция: если есть расширение — это файл, даже если dir без кавычек
		if (isDirectory && IsLikelyFile(itemName)) {
			isDirectory = false;
			isActuallyFile = true;
		}

		if (isDirectory) {
			pathToCheck = GetDirectoryPath(i);
			sn->con.AddLog(" [Папка] %s -> путь: %s", itemName, cp1251_to_utf8(pathToCheck.c_str()).c_str());

			if (!fsm->DirectoryExists(pathToCheck.c_str())) {
				sn->con.AddLog("   Пропущена: папка не существует");
				continue;
			}

			std::string folderName = GetFileNameFromPath(pathToCheck);
			if (folderName == "MO2") {
				sn->con.AddLog("   Пропущена: игнор (MO2)");
				continue;
			}

			std::string hash = md5_files_in_directory(pathToCheck.c_str());
			if (hash.empty()) {
				sn->con.AddLog("   Ошибка: не удалось посчитать хэш папки");
				continue;
			}

			sn->con.AddLog("%s = %s", itemName, hash.c_str());
		}
		else {
			pathToCheck = GetFilePath(i);
			sn->con.AddLog(" [%s] %s -> путь: %s",
				isActuallyFile ? "Файл*" : "Файл",
				itemName, cp1251_to_utf8(pathToCheck.c_str()).c_str());

			if (!fsm->FileExists(pathToCheck.c_str())) {
				sn->con.AddLog("   Пропущен: файл не существует");
				continue;
			}

			std::string hash = md5file(pathToCheck.c_str());
			if (hash.empty()) {
				sn->con.AddLog("   Ошибка: не удалось посчитать хэш файла");
				continue;
			}

			sn->con.AddLog("%s = %s", itemName, hash.c_str());
		}

		foundAny = true;
	}

	if (!foundAny) {
		sn->con.AddLog("Нет активных элементов ANOMALY для экспорта.");
	}
	else {
		sn->con.AddLog("Экспорт хэшей ANOMALY завершён.");
	}
}

bool IsLikelyFileMO2(const char* name) {
	const char* ext = strrchr(name, '.');
	if (!ext) return false;
	ext++;

	const char* extensions[] = {
		"exe", "dll", "db0", "db1", "db2", "db3", "db4",
		"mhr", "ini", "ltx", "txt", "cfg", "so", "dylib", "bin", "txt", "log", "xml", "json",
		"dll", "pdb", "exp", "lib", "ilk", "tlb", "tli",
		"zip", "rar", "7z", "pak", "arc",
		"mp3", "wav", "ogg", "flac",
		"dds", "tga", "png", "jpg", "bmp",
		"vdf", "script", "script2", "script3"
	};

	int count = sizeof(extensions) / sizeof(extensions[0]);
	for (int i = 0; i < count; i++) {
		if (_stricmp(ext, extensions[i]) == 0) {
			return true;
		}
	}
	return false;
}

void Debug_ExportMO2Hashes()
{
	sn->con.AddLog("Экспорт хэшей: MO2 (2) — начало");

	bool foundAny = false;

	for (int i = 0; i < iFileListCount; i++) {
		if (filelist.fileList[i].m_type != SUBMODULE_TYPE_MO2) {
			continue;
		}

		const char* itemName = filelist.fileList[i].name;
		std::string pathToCheck;
		bool isDirectory = (filelist.fileList[i].dir[0] != '"');
		bool isActuallyFile = false;

		// ?? Коррекция: если есть расширение — это файл, даже если dir без кавычек
		if (isDirectory && IsLikelyFileMO2(itemName)) {
			isDirectory = false;
			isActuallyFile = true;
		}

		if (isDirectory) {
			pathToCheck = GetDirectoryPath(i);
			sn->con.AddLog(" [Папка] %s -> путь: %s", itemName, cp1251_to_utf8(pathToCheck.c_str()).c_str());

			if (!fsm->DirectoryExists(pathToCheck.c_str())) {
				sn->con.AddLog("   Пропущена: папка не существует");
				continue;
			}

			// ?? Игнорируем папку mods
			std::string folderName = GetFileNameFromPath(pathToCheck);
			if (folderName == "mods") {
				sn->con.AddLog("   Пропущена: игнор (mods)");
				continue;
			}

			std::string hash = md5_files_in_directory(pathToCheck.c_str());
			if (hash.empty()) {
				sn->con.AddLog("   Ошибка: не удалось посчитать хэш папки");
				continue;
			}

			sn->con.AddLog("%s = %s", itemName, hash.c_str());
		}
		else {
			pathToCheck = GetFilePath(i);
			sn->con.AddLog(" [%s] %s -> путь: %s",
				isActuallyFile ? "Файл*" : "Файл",
				itemName, cp1251_to_utf8(pathToCheck.c_str()).c_str());

			if (!fsm->FileExists(pathToCheck.c_str())) {
				sn->con.AddLog("   Пропущен: файл не существует");
				continue;
			}

			std::string hash = md5file(pathToCheck.c_str());
			if (hash.empty()) {
				sn->con.AddLog("   Ошибка: не удалось посчитать хэш файла");
				continue;
			}

			sn->con.AddLog("%s = %s", itemName, hash.c_str());
		}

		foundAny = true;
	}

	if (!foundAny) {
		sn->con.AddLog("Нет активных элементов MO2 для экспорта.");
	}
	else {
		sn->con.AddLog("Экспорт хэшей MO2 завершён.");
	}
}




//Простая функция сканирования и удаление невалидных модов.
void Debug_CheckAndCleanInvalidHashModsV2(float* outProgress = nullptr) {
    if (outProgress) *outProgress = 0.0f;

    sn->con.AddLog("Диагностика: проверка хэшей валидных модов %s",
        (sn->umode.universal_mode.enabled ? cp1251_to_utf8(sn->umode.modpack_name).c_str() : u8"Purge of Zone"));

    bool foundInvalid = false;
    int totalChecked = 0;
    int totalMods = 0;

    for (int i = 0; i < iFileListCount; ++i) {
        if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE) {
            totalMods++;
        }
    }

    if (totalMods == 0) {
        sn->con.AddLog("Не найдено ни одного мода для проверки.");
        if (outProgress) *outProgress = 100.0f;
        return;
    }

    for (int i = 0; i < iFileListCount; ++i) {
        if (filelist.fileList[i].m_type != SUBMODULE_TYPE_BE) continue;

        const char* modName = filelist.fileList[i].name;
        std::string pathToCheck = GetDirectoryPath(i);

        if (!fsm->DirectoryExists(pathToCheck.c_str())) {
            sn->con.AddLog(" Пропущен: папка не найдена — %s", modName);
            totalChecked++;
            if (outProgress) *outProgress = (float)(totalChecked * 100) / totalMods;
            continue;
        }

        std::string calculatedHash = md5_files_in_directory(pathToCheck.c_str());
        const char* expectedHash = filelist.fileList[i].hash;

        if (strcmp(expectedHash, calculatedHash.c_str()) == 0) {
            sn->con.AddLog(" Хэш совпал: {00BB00}%s", cp1251_to_utf8(pathToCheck.c_str()).c_str());
        }
        else {
            sn->con.AddLog(" Хэш НЕ совпал: {FFFF00}%s", cp1251_to_utf8(pathToCheck.c_str()).c_str());
            if (RemoveModFromDisk(pathToCheck)) {
                sn->con.AddLog("   Успешно удалён.");
                foundInvalid = true;
            }
            else {
                sn->con.AddLog("   Ошибка удаления!");
            }
        }

        totalChecked++;
        if (outProgress) {
            *outProgress = (float)(totalChecked * 100) / totalMods;
        }
    }

    if (!foundInvalid) {
        sn->con.AddLog("Все моды имеют корректный хэш — очистка не требуется.");
    }
    else {
        sn->con.AddLog("Очистка завершена: повреждённые моды удалены.");
    }

    sn->con.AddLog("Диагностика завершена.");
    if (outProgress) *outProgress = 100.0f;
}

//Очень мощный сканер 0_O
enum class ExternalModsAction {
	None,
	ConfirmDelete,
	AutoBackup
};

ExternalModsAction pendingAction = ExternalModsAction::None;
std::vector<std::string> externalModsList;
bool scanComplete = false;
std::string baseModsPath;
std::string backupTimestampDir;

void ScanExternalMods() {
	externalModsList.clear();
	scanComplete = false;

	baseModsPath = GetMO2Folder() + "\\mods";
	std::set<std::string> allowedMods;
	std::set<std::string> protectedNames = {
		"profiles", "downloads", "config", "backup", "overwrite", "root"
	};

	// Собираем разрешённые моды
	for (int i = 0; i < iFileListCount; ++i) {
		if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE) {
			allowedMods.insert(filelist.fileList[i].name);
		}
	}

	try {
		for (const auto& entry : std::filesystem::directory_iterator(
			baseModsPath,
			std::filesystem::directory_options::skip_permission_denied)) {
			if (entry.is_directory()) {
				std::string name = entry.path().filename().string();
				if (protectedNames.find(name) != protectedNames.end()) continue;
				if (allowedMods.find(name) == allowedMods.end()) {
					externalModsList.push_back(name);
				}
			}
		}
	}
	catch (...) {
		sn->con.AddLog("Ошибка при сканировании папки mods.");
	}

	scanComplete = true;
}

void PerformAutoBackup() {
	auto now = std::chrono::system_clock::now();
	std::time_t time_t = std::chrono::system_clock::to_time_t(now);
	std::tm tm = {};
#ifdef _WIN32
	localtime_s(&tm, &time_t);
#else
	tm = *std::localtime(&time_t);
#endif
	std::stringstream ss;
	ss << std::put_time(&tm, "%Y-%m-%d_%H-%M");
	std::string timestamp = ss.str();
	std::string backupDir = GetMO2Folder() + "\\backup\\external_mods\\" + timestamp;

	fsm->CreatefolderRecursive(backupDir.c_str());
	sn->con.AddLog("Создана папка бэкапа: %s", backupDir.c_str());

	for (auto& mod : externalModsList) {
		std::string src = baseModsPath + "\\" + mod;
		std::string dst = backupDir + "\\" + mod;

		try {
			std::filesystem::rename(src, dst);
			sn->con.AddLog("Перемещён в бэкап: %s", mod.c_str());
		}
		catch (const std::exception& e) {
			sn->con.AddLog("Ошибка перемещения %s: %s", mod.c_str(), e.what());
		}
	}

	sn->con.AddLog("Бэкап сторонних модов завершён.");
	pendingAction = ExternalModsAction::None;
	scanComplete = false;
}

//========================================================================//
//HASH_CHECK_MODS, // Проверка модов на валидность, удаление всех невалидных. - обновление

// Глобальный флаг прерывания — достаточно атомарного типа для потокобезопасности
std::atomic<bool> g_debugTaskAborted{ false };

std::atomic<bool> g_hashcheck{ false };

void AbortCurrentDebugTask() {
	g_debugTaskAborted = true;
}

bool isRunningCheckHash()
{
	return g_hashcheck;
}


void Debug_CheckAndCleanInvalidHashMods_ExternalModsDelete(float* outProgress) {

	// Сброс флага прерывания перед началом работы
	g_debugTaskAborted.store(false, std::memory_order_relaxed);
	g_hashcheck.store(false, std::memory_order_relaxed);

	if (outProgress) *outProgress = 0.0f;

	sn->con.AddLog("{AAFFBB}Диагностика: проверка хэшей и сторонних модов %s",
		(sn->umode.universal_mode.enabled ? cp1251_to_utf8(sn->umode.modpack_name).c_str() : u8"Purge of Zone"));

	g_hashcheck = true;

	bool foundInvalid = false;
	bool foundExternal = false;
	int totalChecked = 0;
	int totalMods = 0;

	// === ШАГ 1: Подсчёт валидных модов из filelist ===
	for (int i = 0; i < iFileListCount; ++i) {
		if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE) {
			totalMods++;
		}
	}

	if (totalMods == 0) {
		sn->con.AddLog("Не найдено ни одного мода для проверки.");
	}

	// === ШАГ 2: Проверка хэшей валидных модов ===
	for (int i = 0; i < iFileListCount; ++i) {
		// Проверка флага прерывания перед каждой итерацией
		if (g_debugTaskAborted.load(std::memory_order_relaxed)) {
			sn->con.AddLog("{AAFFBB}Диагностика прервана пользователем.");
			if (outProgress) *outProgress = -1.0f;
			g_hashcheck = false;
			return;
		}


		if (filelist.fileList[i].m_type != SUBMODULE_TYPE_BE) continue;

		const char* modName = filelist.fileList[i].name;
		std::string pathToCheck = GetDirectoryPath(i);

		if (!fsm->DirectoryExists(pathToCheck.c_str())) {
			sn->con.AddLog(" Пропущен: папка не найдена — %s", modName);
			totalChecked++;
			if (outProgress) *outProgress = (float)(totalChecked * 100) / (totalMods + 1); // +1 для плавности
			continue;
		}

		std::string calculatedHash = md5_files_in_directory(pathToCheck.c_str());
		const char* expectedHash = filelist.fileList[i].hash;

		if (strcmp(expectedHash, calculatedHash.c_str()) == 0) {
			sn->con.AddLog(" Хэш совпал: {00BB00}%s", pathToCheck.c_str());
		}
		else {
			sn->con.AddLog(" Хэш НЕ совпал: {FFFF00}%s", pathToCheck.c_str());
			if (RemoveModFromDisk(pathToCheck)) {
				sn->con.AddLog("   Успешно удалён.");
				foundInvalid = true;
			}
			else {
				sn->con.AddLog("   Ошибка удаления!");
			}
		}

		totalChecked++;
		if (outProgress) {
			*outProgress = (float)(totalChecked * 100) / (totalMods + 1);
		}
	}

	// Проверка флага перед следующим шагом
	if (g_debugTaskAborted.load(std::memory_order_relaxed)) {
		sn->con.AddLog("{AAFFBB}Диагностика прервана пользователем перед проверкой сторонних модов.");
		if (outProgress) *outProgress = -1.0f;
		g_hashcheck = false;
		return;
	}



	// === ШАГ 3: Поиск и удаление сторонних модов ===
	std::string baseDirectory = GetMO2Folder() + "\\mods";
	sn->con.AddLog("Начинаю проверку сторонних модов в папке: %s", baseDirectory.c_str());

	if (!fsm->FolderExist(baseDirectory.c_str())) {
		sn->con.AddLog("{AAFFFF}Папка mods не найдена — пропуск проверки сторонних.");
	}
	else {
		std::set<std::string> allowedMods;
		for (int i = 0; i < iFileListCount; ++i) {
			if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE) {
				allowedMods.insert(filelist.fileList[i].name);
			}
		}

		for (const auto& entry : std::filesystem::directory_iterator(
			baseDirectory,
			std::filesystem::directory_options::skip_permission_denied)) {

			// Проверка флага внутри цикла по файлам
			if (g_debugTaskAborted.load(std::memory_order_relaxed)) {
				sn->con.AddLog("{AAFFBB}Диагностика прервана пользователем во время проверки сторонних модов.");
				if (outProgress) *outProgress = -1.0f;
				g_hashcheck = false;
				return;
			}

			if (entry.is_directory()) {
				std::string folderName = entry.path().filename().string();
				if (allowedMods.find(folderName) == allowedMods.end()) {
					std::string fullPath = entry.path().string();
					sn->con.AddLog(" Сторонний мод обнаружен: %s", folderName.c_str());
					if (RemoveModFromDisk(fullPath)) {
						sn->con.AddLog("   Удалён успешно.");
						foundExternal = true;
					}
					else {
						sn->con.AddLog("   Ошибка при удалении!");
					}
				}
			}
		}
	}

	// === Завершение ===
	if (!foundInvalid && !foundExternal) {
		sn->con.AddLog("Всё в порядке: хэши корректны, сторонних модов нет.");
	}
	else {
		if (foundInvalid) sn->con.AddLog("Очистка: повреждённые моды удалены.");
		if (foundExternal) sn->con.AddLog("Очистка: сторонние моды удалены.");
	}

	sn->con.AddLog("{AAFFBB}Диагностика завершена.");
	if (outProgress) *outProgress = 100.0f;
	g_hashcheck = false;
}