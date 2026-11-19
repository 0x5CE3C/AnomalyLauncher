#include "main.h"
#include "ZipPacker.h"


// Флаги для Anomaly Pack
std::atomic<bool> g_stopAnomalyPack{ false };
std::atomic<bool> g_isRunningAnomalyPack{ false };

// Флаги для MO2 Mods Pack (папка mods внутри MO2)
std::atomic<bool> g_stopMo2ModsPack{ false };
std::atomic<bool> g_isRunningMo2Mods{ false };

// Флаги для MO2 Top Pack (корень MO2, кроме папки mods)
std::atomic<bool> g_stopMo2TopPack{ false };
std::atomic<bool> g_isRunningMo2Top{ false };


void AbortAnomalyPack() {
    g_stopAnomalyPack.store(true, std::memory_order_release);
}

void AbortMo2ModsPack() {
    g_stopMo2ModsPack.store(true, std::memory_order_release);
}

void AbortMo2TopPack() {
    g_stopMo2TopPack.store(true, std::memory_order_release);
}

bool IsRunningAnomalyPack() {
    return g_isRunningAnomalyPack.load(std::memory_order_acquire);
}

bool IsRunningMo2Mods() {
    return g_isRunningMo2Mods.load(std::memory_order_acquire);
}

bool IsRunningMo2Top() {
    return g_isRunningMo2Top.load(std::memory_order_acquire);
}


void PackAnomalyTaskHandler(float* outProgress) {
    // Сброс флагов перед стартом
    g_stopAnomalyPack.store(false, std::memory_order_relaxed);
    g_isRunningAnomalyPack.store(false, std::memory_order_relaxed);

    if (outProgress) *outProgress = 0.0f;

    sn->con.AddLog("{AAFFBB}Запуск архивации Anomaly (игнорируется папка MO2)...");
    g_isRunningAnomalyPack.store(true, std::memory_order_relaxed);

    const auto& paths = LauncherConfig::getInstance().getPaths();
    const char* sevenZa = zipPath.c_str(); // путь к 7za.exe

    std::string srcRoot = paths.pathAnomaly.string();
    while (!srcRoot.empty() && (srcRoot.back() == '/' || srcRoot.back() == '\\'))
        srcRoot.pop_back();

    if (!PathFileExistsA(srcRoot.c_str())) {
        sn->con.AddLog("[ERROR] Anomaly root folder not found: %s", srcRoot.c_str());
        g_isRunningAnomalyPack.store(false, std::memory_order_relaxed);
        if (outProgress) *outProgress = -1.0f;
        return;
    }

    std::string sevenZaNorm = sevenZa;
    for (char& c : sevenZaNorm) if (c == '\\') c = '/';

    // Вспомогательная лямбда: проверяет, заканчивается ли имя на .zip (регистронезависимо)
    auto isZipExtension = [](const std::string& name) -> bool {
        size_t len = name.size();
        if (len < 4) return false;
        char c1 = name[len - 4];
        char c2 = name[len - 3];
        char c3 = name[len - 2];
        char c4 = name[len - 1];
        if (c1 != '.') return false;
        // Проверяем .zip / .ZIP / смешанный регистр
        auto toLower = [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); };
        return toLower(c2) == 'z' && toLower(c3) == 'i' && toLower(c4) == 'p';
        };

    // Предварительный подсчёт элементов для прогресса
    size_t totalItems = 0;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(srcRoot, fs::directory_options::skip_permission_denied)) {
            if (g_stopAnomalyPack.load(std::memory_order_relaxed)) break;

            const auto& path = entry.path();
            const std::string pathStr = path.string();
            const char* szPath = pathStr.c_str();

            // Используем ТВОЙ метод проверки на обычный файл
            if (!fsm->is_file(szPath)) continue;

            std::string fname = path.filename().string();

            // Игнорируем лаунчер-файлы
            if (fname == "AnomalyLauncher.exe" || fname == "bass.dll" || fname == "7za.exe") continue;

            // Защита от .zip.zip: если файл уже .zip — не архивируем его
            if (isZipExtension(fname)) continue;

            // Blacklist
            bool excluded = false;
            const auto& blacklistRules = ConfigManager::getInstance().getBlackListRules();
            for (const auto& rule : blacklistRules) {
                if (isPathExcluded(pathStr, rule.folderName, ModuleType::Anomaly, rule.type)) {
                    excluded = true;
                    break;
                }
            }
            if (!excluded) totalItems++;
        }
    }
    catch (...) {
        totalItems = 1; // fallback, чтобы не делить на ноль
    }

    size_t processedItems = 0;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(srcRoot, fs::directory_options::skip_permission_denied)) {
            // ПРОВЕРКА ФЛАГА НА КАЖДОЙ ИТЕРАЦИИ
            if (g_stopAnomalyPack.load(std::memory_order_relaxed)) {
                sn->con.AddLog("{AAFFBB}Архивация Anomaly остановлена пользователем.");
                g_isRunningAnomalyPack.store(false, std::memory_order_relaxed);
                if (outProgress) *outProgress = -1.0f;
                return;
            }

            const auto& path = entry.path();
            const std::string fullPathStr = path.string();
            const char* fullPathC = fullPathStr.c_str();
            std::string fname = path.filename().string();

            // Используем ТВОЙ метод проверки на обычный файл
            if (!fsm->is_file(fullPathC)) continue;

            // Игнорируем лаунчер-файлы
            if (fname == "AnomalyLauncher.exe" || fname == "bass.dll" || fname == "7za.exe") {
                processedItems++;
                continue;
            }

            // Защита от .zip.zip
            if (isZipExtension(fname)) {
                // Можно залогировать, если хочешь видеть такие случаи
                // sn->con.AddLog("{00AA00}Пропущено (уже ZIP): %s", fname.c_str());
                processedItems++;
                continue;
            }

            // Blacklist
            bool excluded = false;
            const auto& blacklistRules = ConfigManager::getInstance().getBlackListRules();
            for (const auto& rule : blacklistRules) {
                if (isPathExcluded(fullPathStr, rule.folderName, ModuleType::Anomaly, rule.type)) {
                    excluded = true;
                    break;
                }
            }
            if (excluded) {
                processedItems++;
                continue;
            }

            // Формируем имя архива: имя файла + .zip
            std::string archiveName = fname + ".zip";

            // Выходной путь: та же папка, где лежит файл
            std::string outDir = path.parent_path().string();
            for (char& c : outDir) if (c == '\\') c = '/';
            if (!outDir.empty() && outDir.back() != '/') outDir += '/';
            std::string outPath = outDir + archiveName;

            // Пропускаем, если архив уже есть (можно убрать условие, если нужна перезапись)
            if (fsm->FileExist(outPath.c_str())) {
                processedItems++;
                continue;
            }

            std::ostringstream cmd;
            cmd << "\"" << sevenZaNorm << "\" a -tzip -mx1 -y \"" << outPath << "\" \"" << fullPathStr << "\"";

            int result = RunCmdNoWindow(cmd.str());

            // Проверка флага ПОСЛЕ запуска 7za (на случай долгой упаковки)
            if (g_stopAnomalyPack.load(std::memory_order_relaxed)) {
                fsm->FileDelete(outPath.c_str()); // очистка мусора
                sn->con.AddLog("{AAFFBB}Anomaly pack stopped during 7za execution.");
                g_isRunningAnomalyPack.store(false, std::memory_order_relaxed);
                if (outProgress) *outProgress = -1.0f;
                return;
            }

            if (result == 0 && fsm->FileExist(outPath.c_str())) {
                Log("[ARCHIVE] Success: %s created", outPath.c_str());
                sn->con.AddLog("{AAFFBB}[OK] Архив создан: %s", archiveName.c_str());
            }
            else {
                Log("[ERROR] 7za failed or file not created. Exit code: %d", result);
                Log("[DEBUG] Expected file at: %s", outPath.c_str());
                sn->con.AddLog("{FF0000}[ERROR] Не удалось создать архив: %s (код %d)", archiveName.c_str(), result);
            }

            processedItems++;

            if (totalItems > 0) {
                *outProgress = (float)(processedItems * 100) / (float)totalItems;
            }
            else {
                *outProgress = 50.0f; // заглушка, если totalItems == 0
            }
        }
    }
    catch (const std::exception& e) {
        sn->con.AddLog("[ERROR] Filesystem error in Anomaly pack: %s", e.what());
        if (g_stopAnomalyPack.load()) {
            if (outProgress) *outProgress = -1.0f;
        }
        else {
            if (outProgress) *outProgress = -2.0f; // ошибка
        }
        g_isRunningAnomalyPack.store(false, std::memory_order_relaxed);
        return;
    }

    sn->con.AddLog("{00BB00}Архивация Anomaly завершена.");
    g_isRunningAnomalyPack.store(false, std::memory_order_relaxed);
    if (outProgress) *outProgress = 100.0f;
}



// Вспомогательная функция: формирует имя архива в точном соответствии с логикой filelist
std::string GetArchiveNameForMod(const std::string& folderName) {
    std::string baseName = extractVersion(folderName);
    if (!baseName.empty()) {
        return baseName + ".zip";
    }
    return folderName + ".zip";
}

void PackMo2ModsTaskHandler(float* outProgress) {
    g_stopMo2ModsPack.store(false, std::memory_order_relaxed);
    g_isRunningMo2Mods.store(false, std::memory_order_relaxed);
    if (outProgress) *outProgress = 0.0f;

    sn->con.AddLog("{AAFFBB}Запуск архивации MO2/mods (папки первого уровня, имена по filelist-логике)...");
    g_isRunningMo2Mods.store(true, std::memory_order_relaxed);

    const auto& paths = LauncherConfig::getInstance().getPaths();
    std::string modsPath = paths.pathMO2.string() + "/mods";
    const char* sevenZa = zipPath.c_str();

    std::string rootPath = modsPath;
    while (!rootPath.empty() && (rootPath.back() == '/' || rootPath.back() == '\\'))
        rootPath.pop_back();

    if (!PathFileExistsA(rootPath.c_str())) {
        sn->con.AddLog("[ERROR] Папка MO2/mods не найдена: %s", rootPath.c_str());
        g_isRunningMo2Mods.store(false, std::memory_order_relaxed);
        if (outProgress) *outProgress = -1.0f;
        return;
    }

    std::string sevenZaNorm = sevenZa;
    for (char& c : sevenZaNorm) if (c == '\\') c = '/';

    size_t totalItems = 0;
    size_t processedItems = 0;

    // ЭТАП 1: Подсчёт только тех папок, которые реально будут архивироваться
    try {
        for (const auto& entry : fs::directory_iterator(rootPath)) {
            if (g_stopMo2ModsPack.load(std::memory_order_relaxed)) break;

            const auto& path = entry.path();
            const std::string pathStr = path.string();
            const char* szPath = pathStr.c_str(); // безопасно: pathStr живёт до конца итерации

            // Используем ТВОЙ метод проверки папки
            if (!fsm->is_folder(szPath)) continue;

            std::string fname = path.filename().string();

            if (fname == "AnomalyLauncher.exe" || fname == "bass.dll" || fname == "7za.exe")
                continue;

            auto len = fname.size();
            bool isExistingZip = false;
            if (len >= 4) {
                char c1 = fname[len - 4];
                char c2 = fname[len - 3];
                char c3 = fname[len - 2];
                char c4 = fname[len - 1];
                if (c1 == '.' && ((c2 == 'z' || c2 == 'Z') && (c3 == 'i' || c3 == 'I') && (c4 == 'p' || c4 == 'P'))) {
                    isExistingZip = true;
                }
            }
            if (isExistingZip) continue;

            bool excluded = false;
            const auto& blacklistRules = ConfigManager::getInstance().getBlackListRules();
            for (const auto& rule : blacklistRules) {
                if (isPathExcluded(pathStr, rule.folderName, ModuleType::MO2, rule.type)) {
                    excluded = true;
                    break;
                }
            }
            if (excluded) continue;

            totalItems++;
            sn->con.AddLog("{AAFFFF}[Queue] В очередь: %s -> %s",
                fname.c_str(), GetArchiveNameForMod(fname).c_str());
        }
    }
    catch (...) {
        totalItems = 1;
    }

    if (totalItems == 0) {
        sn->con.AddLog("{00BB00}Архивация MO2/mods завершена (нет папок для обработки).");
        g_isRunningMo2Mods.store(false, std::memory_order_relaxed);
        if (outProgress) *outProgress = 100.0f;
        return;
    }

    // ЭТАП 2: Непосредственная архивация
    try {
        for (const auto& entry : fs::directory_iterator(rootPath)) {
            if (g_stopMo2ModsPack.load(std::memory_order_relaxed)) {
                sn->con.AddLog("{AAFFBB}MO2 Mods pack остановлен пользователем.");
                g_isRunningMo2Mods.store(false, std::memory_order_relaxed);
                if (outProgress) *outProgress = -1.0f;
                return;
            }

            const auto& path = entry.path();
            const std::string fullPathStr = path.string();
            const char* fullPathC = fullPathStr.c_str();
            std::string fname = path.filename().string();

            // Используем ТВОЙ метод проверки папки
            if (!fsm->is_folder(fullPathC)) {
                processedItems++;
                continue;
            }

            if (fname == "AnomalyLauncher.exe" || fname == "bass.dll" || fname == "7za.exe") {
                processedItems++; continue;
            }

            auto len = fname.size();
            bool isExistingZip = (len >= 4 &&
                ((fname[len - 4] == '.' && fname[len - 3] == 'z' && fname[len - 2] == 'i' && fname[len - 1] == 'p') ||
                    (fname[len - 4] == '.' && fname[len - 3] == 'Z' && fname[len - 2] == 'I' && fname[len - 1] == 'P')));

            if (isExistingZip) {
                processedItems++; continue;
            }

            bool excluded = false;
            const auto& blacklistRules = ConfigManager::getInstance().getBlackListRules();
            for (const auto& rule : blacklistRules) {
                if (isPathExcluded(fullPathStr, rule.folderName, ModuleType::MO2, rule.type)) {
                    excluded = true; break;
                }
            }
            if (excluded) {
                processedItems++; continue;
            }

            std::string archiveName = GetArchiveNameForMod(fname);

            std::string outPath = rootPath;
            if (!outPath.empty() && outPath.back() != '/' && outPath.back() != '\\')
                outPath += '/';
            outPath += archiveName;

            std::ostringstream cmd;
            cmd << "\"" << sevenZaNorm << "\" a -tzip -mx1 -r -y \"" << outPath << "\" \"" << fullPathStr << "\"";

            int result = RunCmdNoWindow(cmd.str());

            if (g_stopMo2ModsPack.load(std::memory_order_relaxed)) {
                fsm->FileDelete(outPath.c_str());
                sn->con.AddLog("{AAFFBB}MO2 Mods pack остановлен во время архивации.");
                g_isRunningMo2Mods.store(false, std::memory_order_relaxed);
                if (outProgress) *outProgress = -1.0f;
                return;
            }

            if (fsm->FileExist(outPath.c_str())) {
                Log("[ARCHIVE] Success: %s created", outPath.c_str());
                sn->con.AddLog("{AAFFBB}[OK] Архив создан: %s", archiveName.c_str());
                processedItems++;
            }
            else {
                Log("[ERROR] 7za failed or file not created. Exit code: %d", result);
                Log("[DEBUG] Expected file at: %s", outPath.c_str());
                sn->con.AddLog("{FF0000}[ERROR] Не удалось создать архив: %s (код %d)", archiveName.c_str(), result);
                // Не прерываем весь процесс, идём дальше
            }

            if (outProgress && totalItems > 0) {
                *outProgress = (float)(processedItems * 100) / (float)totalItems;
            }
        }
    }
    catch (...) {
        sn->con.AddLog("[ERROR] Критическая ошибка в MO2 Mods pack.");
        if (outProgress) *outProgress = -2.0f;
    }

    sn->con.AddLog("{00BB00}MO2 Mods pack completed.");
    g_isRunningMo2Mods.store(false, std::memory_order_relaxed);
    if (outProgress) *outProgress = 100.0f;
}




void PackMo2TopTaskHandler(float* outProgress) {
    g_stopMo2TopPack.store(false, std::memory_order_relaxed);
    g_isRunningMo2Top.store(false, std::memory_order_relaxed);
    if (outProgress) *outProgress = 0.0f;

    sn->con.AddLog("{AAFFBB}Запуск архивации корня MO2 (папка mods игнорируется, ZIP пропускаются)...");
    g_isRunningMo2Top.store(true, std::memory_order_relaxed);

    const auto& paths = LauncherConfig::getInstance().getPaths();
    std::string rootPath = paths.pathMO2.string();
    const char* sevenZa = zipPath.c_str();

    // Нормализация пути
    while (!rootPath.empty() && (rootPath.back() == '/' || rootPath.back() == '\\'))
        rootPath.pop_back();

    if (!PathFileExistsA(rootPath.c_str())) {
        sn->con.AddLog("[ERROR] Папка MO2 не обнаружена: %s", rootPath.c_str());
        g_isRunningMo2Top.store(false, std::memory_order_relaxed);
        if (outProgress) *outProgress = -1.0f;
        return;
    }

    std::string sevenZaNorm = sevenZa;
    for (char& c : sevenZaNorm) if (c == '\\') c = '/';

    size_t totalItems = 0;
    size_t processedItems = 0;

    // --- ЭТАП 1: Подсчёт только тех файлов, которые реально будут архивироваться ---
    try {
        for (const auto& entry : fs::directory_iterator(rootPath)) {
            if (g_stopMo2TopPack.load(std::memory_order_relaxed)) break;

            const auto& path = entry.path();

            // 1. Только файлы (папки не трогаем этим хендлером, либо нужен отдельный подход с -r)
            //if (!path.is_regular_file()) continue;

            std::string fname = path.filename().string();

            // 2. Исключаем запрещенные имена
            if (fname == "mods" || fname == "AnomalyLauncher.exe" ||
                fname == "bass.dll" || fname == "7za.exe") continue;

            // 3. Исключаем уже существующие ZIP-архивы (чтобы не создавать dlls.zip внутри dlls.zip)
            auto len = fname.size();
            if (len >= 4) {
                char c1 = fname[len - 4];
                char c2 = fname[len - 3];
                char c3 = fname[len - 2];
                char c4 = fname[len - 1];
                if (c1 == '.' && ((c2 == 'z' || c2 == 'Z') && (c3 == 'i' || c3 == 'I') && (c4 == 'p' || c4 == 'P'))) {
                    continue;
                }
            }

            // 4. Проверка blacklist
            bool excluded = false;
            const auto& blacklistRules = ConfigManager::getInstance().getBlackListRules();
            for (const auto& rule : blacklistRules) {
                if (isPathExcluded(path.string(), rule.folderName, ModuleType::MO2, rule.type)) {
                    excluded = true;
                    break;
                }
            }
            if (excluded) continue;

            // Если дошли сюда — файл будет обработан
            totalItems++;
            sn->con.AddLog("{AAFFFF}[Extractor]: В очередь архивации добавлен: %s", fname.c_str());
        }
    }
    catch (...) {
        totalItems = 1; // Заглушка, чтобы не делить на 0, если ошибка итерации
    }

    if (totalItems == 0) {
        sn->con.AddLog("{00BB00}Архивация MO2 завершена (нет файлов для обработки).");
        g_isRunningMo2Top.store(false, std::memory_order_relaxed);
        if (outProgress) *outProgress = 100.0f;
        return;
    }

    // --- ЭТАП 2: Непосредственная архивация ---
    try {
        for (const auto& entry : fs::directory_iterator(rootPath)) {
            if (g_stopMo2TopPack.load(std::memory_order_relaxed)) {
                sn->con.AddLog("{AAFFBB}MO2 Top pack остановлен пользователем.");
                g_isRunningMo2Top.store(false, std::memory_order_relaxed);
                if (outProgress) *outProgress = -1.0f;
                return;
            }

            const auto& path = entry.path();

            //if (!path.is_regular_file()) continue; // Пропускаем папки

            std::string fullPath = path.string();
            std::string fname = path.filename().string();

            // Повторяем проверки (можно вынести в отдельную функцию-предикат)
            if (fname == "mods" || fname == "AnomalyLauncher.exe" ||
                fname == "bass.dll" || fname == "7za.exe") {
                processedItems++;
                continue;
            }

            auto len = fname.size();
            bool isZip = (len >= 4 &&
                ((fname[len - 4] == '.' && fname[len - 3] == 'z' && fname[len - 2] == 'i' && fname[len - 1] == 'p') ||
                    (fname[len - 4] == '.' && fname[len - 3] == 'Z' && fname[len - 2] == 'I' && fname[len - 1] == 'P')));

            if (isZip) {
                processedItems++; // Считаем как обработанный, но не архивируем
                continue;
            }

            bool excluded = false;
            const auto& blacklistRules = ConfigManager::getInstance().getBlackListRules();
            for (const auto& rule : blacklistRules) {
                if (isPathExcluded(fullPath, rule.folderName, ModuleType::MO2, rule.type)) {
                    excluded = true;
                    break;
                }
            }
            if (excluded) {
                processedItems++;
                continue;
            }

            // Формирование имени архива
            std::string archiveName = fname;
            if (!isZip) { // Защита от .zip.zip (хотя выше мы уже отсекли существующие .zip)
                archiveName += ".zip";
            }

            std::string outPath = rootPath;
            if (!outPath.empty() && outPath.back() != '/' && outPath.back() != '\\')
                outPath += '/';
            outPath += archiveName;

            // Команда для 7za
            std::ostringstream cmd;
            cmd << "\"" << sevenZaNorm << "\" a -tzip -mx1 -y \"" << outPath << "\" \"" << fullPath << "\"";

            int result = RunCmdNoWindow(cmd.str());            

            if (result == 0 && fsm->FileExist(outPath.c_str())) {
                Log("[ARCHIVE] Success: %s created", outPath.c_str());
                sn->con.AddLog("{AAFFBB}[OK] Архив создан: %s", archiveName.c_str());
            }
            else {
                Log("[ERROR] 7za failed or file not created. Exit code: %d", result);
                Log("[DEBUG] Expected file at: %s", outPath.c_str());
                sn->con.AddLog("{FF0000}[ERROR] Не удалось создать архив: %s (код %d)", archiveName.c_str(), result);
            }

            // Обновление прогресса
            if (outProgress && totalItems > 0) {
                *outProgress = (float)(processedItems * 100) / (float)totalItems;
            }
        }
    }
    catch (...) {
        sn->con.AddLog("[ERROR] Критическая ошибка в MO2 Top pack.");
        if (outProgress) *outProgress = -2.0f;
    }

    sn->con.AddLog("{00BB00}Архивация MO2 завершена.");
    g_isRunningMo2Top.store(false, std::memory_order_relaxed);
    if (outProgress) *outProgress = 100.0f;
}
