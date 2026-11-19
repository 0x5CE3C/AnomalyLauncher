#include "main.h"
#include "RenderConfigEditor.h"
#include "ConfigManager.h"
#include "LauncherConfig.h"

#include <cstring>
//#include "imgui.h"

static bool showConfigEditor = false;

void ToggleConfigEditor() {    
    showConfigEditor = !showConfigEditor;    
}

void RenderConfigEditor() {
    if (!showConfigEditor) return;

    ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowSize().x - 6, ImGui::GetWindowSize().y - 6));
    if (ImGui::Begin(u8"Генератор конфигурации (files_list.ini)", &showConfigEditor, ImGuiWindowFlags_AlwaysAutoResize)) {

        RenderImage(gui::device);

        auto& cfg = ConfigManager::getInstance();
        const auto& paths = LauncherConfig::getInstance().getPaths();

        //ImGui::Text(u8"Текущие пути к модулям:");
        ImGui::SeparatorText(u8"Текущие пути к модулям:");
        ImGui::BulletText(u8"Extractor: %s", paths.pathExtractor.generic_u8string().c_str());
        ImGui::BulletText(u8"Anomaly:   %s", paths.pathAnomaly.generic_u8string().c_str());
        ImGui::BulletText(u8"MO2:       %s", paths.pathMO2.generic_u8string().c_str());
        ImGui::BulletText(u8"PZ:        %s", paths.pathPZ.generic_u8string().c_str());
        
        ImGui::SeparatorText(u8"Фильтры:");
        ImGui::Text(u8"Только:");
        ImGui::SameLine();ImGui::Checkbox(u8"Extractor", &sn->filter_only_extractor.enabled);
        ImGui::SameLine();ImGui::Checkbox(u8"Anomaly", &sn->filter_only_anomaly.enabled);
        ImGui::SameLine();ImGui::Checkbox(u8"MO2", &sn->filter_only_mo2.enabled);
        ImGui::SameLine();ImGui::Checkbox(u8"PZ(modpack)", &sn->filter_only_mp.enabled);
        ImGui::Separator();

        // --- Секция 1: Разрешённые EXE файлы (Bin Files) ---
        if (ImGui::CollapsingHeader(u8"Разрешённые Файлы", ImGuiTreeNodeFlags_CollapsingHeader)) {
            ImGui::BeginChild("FileList");

            auto& cfg = ConfigManager::getInstance();
            const auto& binFiles = cfg.getBinFiles();

            for (size_t i = 0; i < binFiles.size(); ++i) {
                ImGui::PushID(static_cast<int>(i));

                // Отображаем путь
                ImGui::Text(u8"%zu: %s", i, binFiles[i].path.c_str());
                ImGui::SameLine();

                // Combo для выбора типа модуля
                const char* typeNames[] = { "all", "anomaly", "mo2", "pz", "extractor" };
                int selectedType = static_cast<int>(binFiles[i].type);
                ImGui::SetNextItemWidth(ImGui::GetWindowWidth() / 2);
                if (ImGui::Combo("##Type", &selectedType, typeNames, 5)) {
                    // Пользователь сменил тип — обновляем и сохраняем
                    cfg.removeBinFile(i); // Удаляем старую запись
                    cfg.addBinFile(binFiles[i].path, static_cast<ModuleType>(selectedType));
                    // После изменения вектора цикл лучше прервать, чтобы не было рассинхрона ID
                    ImGui::PopID();
                    break;
                }

                ImGui::SameLine();
                if (ImGui::Button(u8"Del##Bin", ImVec2(50, 0))) {
                    cfg.removeBinFile(i);
                    ImGui::PopID();
                    break; // Вектор сдвинулся, перерисуем на следующем кадре
                }

                ImGui::PopID();
            }

            static char newBinEntry[256] = "";
            static int newBinType = 0; // 0=all, 1=anomaly, 2=mo2, 3=pz, 4=extractor

            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() / 2);
            ImGui::InputText(u8"Новый путь (например, \\bin\\MyMod.exe)", newBinEntry, IM_ARRAYSIZE(newBinEntry));
            ImGui::SameLine();

            const char* typeNames[] = { "all", "anomaly", "mo2", "pz", "extractor" };
            if (ImGui::Combo(u8"Модуль", &newBinType, typeNames, 5)) {
                // Смена типа — просто обновляем переменную
            }

            if (ImGui::Button(u8"Добавить в Файлы")) {
                std::string s(newBinEntry);
                if (!s.empty()) {
                    for (char& c : s) if (c == '/') c = '\\';
                    ModuleType t = static_cast<ModuleType>(newBinType);
                    cfg.addBinFile(s, t);
                    newBinEntry[0] = '\0';
                    newBinType = 0;
                }
            }
            ImGui::EndChild();
        }

        ImGui::Separator();

        // --- Секция 2: Чёрный список папок ---
        if (ImGui::CollapsingHeader(u8"Чёрный список", ImGuiTreeNodeFlags_CollapsingHeader)) {
            ImGui::BeginChild(u8"BlacklistList");

            for (size_t i = 0; i < cfg.getBlackListRules().size(); ++i) {
                ImGui::PushID(i);
                const auto& rule = cfg.getBlackListRules()[i];
                ImGui::Text(u8"%zu: '%s' -> Модуль: %s",
                    i, rule.folderName.c_str(), ConfigManager::moduleTypeToString(rule.type).c_str());
                ImGui::SameLine();
                if (ImGui::Button(u8"Del##Black", ImVec2(50, 0))) {
                    cfg.removeBlackListRule(i);
                    break;
                }
                ImGui::PopID();
            }

            static char newBlackEntry[128] = "";
            static int selectedType = 0; // 0=all, 1=anomaly, 2=mo2, 3=pz, 4=extractor

            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() / 2);
            ImGui::InputText(u8"Имя для исключения", newBlackEntry, IM_ARRAYSIZE(newBlackEntry));
            ImGui::SameLine();
            const char* typeNames[] = { "all", "anomaly", "mo2", "pz", "extractor" };
            if (ImGui::Combo(u8"Модуль", &selectedType, typeNames, (int)(sizeof(typeNames) / sizeof(typeNames[0])))) {
                // Смена типа — ничего не делаем, просто обновляем значение
            }

            if (ImGui::Button(u8"Добавить в чёрный список")) {
                std::string folder(newBlackEntry);
                if (!folder.empty()) {
                    ModuleType t = ModuleType::All;
                    switch (selectedType) {
                    case 1: t = ModuleType::Anomaly; break;
                    case 2: t = ModuleType::MO2; break;
                    case 3: t = ModuleType::PZ; break;
                    case 4: t = ModuleType::Extractor; break;
                    default: t = ModuleType::All; break;
                    }
                    cfg.addBlackListRule(folder, t);
                    newBlackEntry[0] = '\0';
                }
            }
            ImGui::EndChild();
        }

        ImGui::Separator();

        ImGui::SeparatorText(u8"Генерация:");

        pMenu->AddButton(&sn->scanhashes.enabled, u8"Проверять хэш-суммы файлов", gui::size.x / 2);
        ImFuncs::FastDescr(u8"{BBDDDD}Если у файлов не совпадает хэш-сумма, это означает что они изменялись. \n [Внимание]:\n На вычисления хэш-сумм требуется длительное время, поэтому эта функция замедляет сканирование!!! \n Длительность сканирования зависит от веса файла.");
        ImGui::SameLine();
        pMenu->AddButton(&sn->scanhashes_dirs.enabled, u8"Проверять хэш-суммы каталогов");
        ImFuncs::FastDescr(u8"{BBDDDD}Если у каталогов не совпадает хэш-сумма, это означает что они изменялись. \n [Внимание]:\n На вычисления хэш-сумм каталогов требуется более длительное время, поэтому эта функция замедляет сканирование!!!\n Длительность сканирования зависит от веса файлов внутри каталога и подкаталогов.");

        //const auto& cfg = ConfigManager::getInstance();
        std::string configDir = fs::path(cfg.getConfigPath()).parent_path().string();

        //Log("configDir: %s", configDir.c_str());

        std::string outFilePath = configDir + "\\files_list.ini";

        static bool isGenerating = false;
        static std::string statusText = u8"Готов";

        ImGui::Text(u8"Статус: %s", statusText.c_str());
        ImGui::Spacing();

        // --- Кнопка 1: Сгенерировать список ---
        if (isGenerating) {
            ImGui::BeginDisabled();
            ImGui::Button(u8"Генерация...");
            ImGui::EndDisabled();
            //ImGui::SameLine();
            //ImGui::Text(u8"(не нажимайте повторно)");
        }
        else {
            if (ImGui::Button(u8"Сгенерировать список")) {
                isGenerating = true;
                statusText = u8"Запущено в фоне...";
                Log("[UI] Запуск генерации: %s", outFilePath.c_str());
                sn->con.AddLog("[UI] Запуск генерации: %s", outFilePath.c_str());

                std::thread([=]() mutable {
                    bool result = false;
                    try {
                        result = FileListGenerator::generate(outFilePath.c_str());
                    }
                    catch (...) {
                        Log("[CRITICAL] Исключение в потоке генерации.");
                        result = false;
                    }

                    if (result) {
                        statusText = u8"Готово! Файл создан.";
                        Log("[UI] Генерация завершена успешно.");
                        sn->con.AddLog("[UI] Генерация завершена успешно.");
                    }
                    else {
                        statusText = u8"Ошибка при создании файла.";
                        Log("[ERROR] Генерация завершилась с ошибкой.");
                        sn->con.AddLog("[ERROR] Генерация завершилась с ошибкой.");
                    }
                    isGenerating = false;
                    }).detach();
            }
        }
        ImGui::SameLine();

        if (ImGui::SmallButton(u8"Открыть папку конфигов лаунчера")) {
            // 1. Получаем путь к папке, где лежит EXE (уровень выше)
            std::string basePath = GetExePath();

            // 2. Добавляем имя папки с конфигами
            // Теперь путь будет: ...\Аномали Пурга Зоны\AnomalyLauncher
            std::string configPath = basePath + "\\AnomalyLauncher";

            // 3. Логируем. 
            // Если твой AddLog нормально показывает кириллицу без конвертации - пиши так:
            sn->con.AddLog("Открываем папку с конфигами: %s", configPath.c_str());

            // ЕСЛИ в логе всё равно кракозябры (РРіСЂС‹), раскомментируй эту строку вместо предыдущей:
            // sn->con.AddLog("Открываем папку с конфигами: %s", ansiToUtf8(configPath).c_str());

            // 4. Проверяем существование ВЛОЖЕННОЙ папки
            if (fsm->FolderExist(configPath.c_str()))
            {
                // 5. Открываем её
                //ShellExecuteA(NULL, "explore", configPath.c_str(), NULL, NULL, SW_SHOWNORMAL);

                std::string targetPath = GetExePath() + "\\AnomalyLauncher\\";

                // Вместо ShellExecuteA используем системную команду
                // Важно: кавычки вокруг пути обязательны, если там есть пробелы!
                //std::string cmd = "explorer.exe \"" + targetPath + "\"";
                //system(cmd.c_str());

                //sn->con.AddLog("Команда для запуска: %s", cmd.c_str());
                ShellExecuteA(NULL, "explore", targetPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
            else
            {
                sn->con.AddLog("ОШИБКА: Папка не с конфигами найдена! Путь: %s", configPath.c_str());
            }
        }

    }
    ImGui::End();
}


namespace fs = std::filesystem;

// Возвращает путь к папке, где ЛЕЖИТ exe (E:\AnomalyPZ)
std::string GetExeDir() {
    wchar_t pathBuf[MAX_PATH + 1] = { 0 };
    if (GetModuleFileNameW(NULL, pathBuf, MAX_PATH) == 0) return "";

    fs::path fullPath(pathBuf);
    // Возвращаем родителя файла -> E:\AnomalyPZ
    return fullPath.parent_path().string();
}

// Возвращает путь к папке с конфигами (E:\AnomalyPZ\AnomalyLauncher)
// Имя папки "AnomalyLauncher" захардкожено, так как оно у тебя фиксировано.
std::string GetConfigDir() {
    std::string exeDir = GetExeDir();
    if (exeDir.empty()) return "";

    fs::path configPath = fs::path(exeDir) / "AnomalyLauncher";
    return configPath.string();
}

void InitLauncher() {
    // 1. Получаем базовые пути
    std::string exeRoot = GetExeDir();           // E:\AnomalyPZ
    std::string configRoot = GetConfigDir();     // E:\AnomalyPZ\AnomalyLauncher

    if (exeRoot.empty() || configRoot.empty()) {
        Log("[CRITICAL] Не удалось определить пути к лаунчеру!");
        return;
    }

    Log("ExeRoot: %s", exeRoot.c_str());
    Log("ConfigRoot: %s", configRoot.c_str());

    // 2. Настраиваем пути к модулям (они все в exeRoot, кроме служебных папок)
    LauncherPaths paths{};
    paths.pathExtractor = fs::path(configRoot); // Если есть отдельная папка extractor
    paths.pathAnomaly = fs::path(exeRoot);
    paths.rootPath = paths.pathAnomaly; // E:\AnomalyPZ

    // Корень, но с фильтром исключений внутри генератора
    paths.pathMO2 = fs::path(exeRoot) / "MO2";       // Если MO2 - это папка
    paths.pathPZ = fs::path(exeRoot) / "MO2" / "mods";         // Если PZ - это папка

    LauncherConfig::getInstance().setPaths(paths);

    std::string base_filelist = configRoot + "\\AnomalyLauncher";

    // 3. Инициализируем конфиг (файл будет создан в E:\AnomalyPZ\AnomalyLauncher\)
    ConfigManager::getInstance().init(base_filelist);

    Log("Launcher initialized. Paths set, config loaded.");
}