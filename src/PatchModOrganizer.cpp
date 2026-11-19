#include "main.h"


// UTF-8 -> @ByteArray(\xd0\xa1...)
std::string makeByteArray(const std::string& utf8Str) {
    if (utf8Str.empty()) {
        return "@ByteArray()";
    }

    std::ostringstream oss;
    oss << "@ByteArray(";

    // Проходим по каждому байту строки.
    // std::string::length() НЕ включает нулевой байт '\0', если строка создана корректно.
    for (unsigned char c : utf8Str) {
        oss << "\\x"
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(c);
    }
    oss << ")";

    return oss.str();
}


std::string makeByteArrayOnlyCyrillic(const std::string& cp1251Str) {
    if (cp1251Str.empty()) {
        return "@ByteArray()";
    }

    std::ostringstream oss;
    oss << "@ByteArray(";

    for (unsigned char c : cp1251Str) {
        // 0x80 (128 bytes) ASCII (включая латиницу, цифры, знаки препинания, CR/LF) — оставляем как есть
        if (c < 0x80) {
            oss << c;
        }
        // CP1251 кириллица и прочие не?ASCII — кодируем как \xNN
        else {
            oss << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(c);            
        }
    }

    oss << ")";
    return oss.str();
}


FILE* f = NULL;

void PatchModOrganizerbtn() {
    
    string modpack_profile_name;
    
    switch (sn->umode.project_mode) {
    case PROJECTMODES::PM_Anomaly_PZ:
        modpack_profile_name = "Purge of Zone";
        break;
    case PROJECTMODES::PM_Anomaly_MP:
        modpack_profile_name = sn->umode.modpack_name;
        break;
    default:
        break;
    }

    //Правильная установка слешей
    string gamepath = fsm->ReplaceAllcharactersInString(GetExePath().c_str(), "\\", "\\\\");
    string exepath = fsm->ReplaceAllcharactersInString(GetExePath().c_str(), "\\", "/");


    string profileutf8 = ansiToUtf8(modpack_profile_name);
    string gamepathutf8 = ansiToUtf8(gamepath);

    std::string ba_profile = makeByteArrayOnlyCyrillic(profileutf8);
    std::string ba_gamepath = makeByteArrayOnlyCyrillic(gamepathutf8);

    //const char* param =\
        "[ModOrganizer.ini Patch] >/" \
        "gamepath: %s/"\
        "profile arraybytes: %s/"\
        "exepath: %s/"\
        "profile: %s/"\
        "profile arraybytes: %s";
    //sn->con.AddLog(param,\
        gamepath.c_str(),\
        ba_gamepath.c_str(),\
        exepath.c_str(), \
        modpack_profile_name.c_str(),\
        ba_profile.c_str());

    // 2. Путь к игре (берём как есть из GetExePath)
    // Пример возврата: "E:\Сборка Аномали Редух Test Ver"
    std::string gamepathAnsi = gamepath;

    // Конвертируем ANSI (CP1251) -> UTF-8
    std::string gamepathUtf8 = ansiToUtf8(gamepathAnsi);

    // Для customExecutables нам нужен путь к папке bin
    // Используем уже готовый gamepathUtf8 (с прямыми слэшами)
    std::string bin_path = gamepathUtf8 + "/bin";

    // ... (логика создания папки MO2) ...
    if (!fsm->FolderExist("MO2")) {
        fsm->CreateFolder("MO2");
    }

    //Sleep(500); // может не понадобится

    FILE* f = fopen("MO2\\ModOrganizer.ini", "wb"); // ТОЛЬКО binary! = utf8
    if (!f) {
        sn->con.AddLog("{FF0000}Ошибка: не удалось открыть ModOrganizer.ini!");
        return;
    }

    // Шаблон конфига
const char* config_template =
"[General]\n"
"gameName=STALKER Anomaly\n"
"selected_profile=%s\n" // ba_profile.c_str(), 1
"gamePath=%s\n" // ba_gamePath.c_str(), 2
"version = 2.5.2\n"
"first_start = false\n"
"\n"
"[PluginPersistance]\n"
"Python %% 20Proxy\\tryInit = false\n"
"\n"
"[Settings]\n"
"profile_local_inis = false\n"
"profile_local_saves = false\n"
"profile_archive_invalidation = true\n"
"filter_regex = false\n"
"regex_case_sensitive = false\n"
"regex_extended = false\n"
"filter_scroll_to_selection = false\n"
"compact_downloads = false\n"
"meta_downloads = false\n"
"autohide_downloads = false\n"
"check_for_updates = false\n"
"use_prereleases = false\n"
"center_dialogs = false\n"
"show_change_game_confirmation = true\n"
"show_menubar_on_alt = true\n"
"double_click_previews = true\n"
"overwrittenLooseFilesColor = @Variant(\\0\\0\\0\\x43\\x1@@\\0\\0\\xff\\xff\\0\\0\\0\\0)\n"
"overwritingLooseFilesColor = @Variant(\\0\\0\\0\\x43\\x1@@\\xff\\xff\\0\\0\\0\\0\\0\\0)\n"
"overwrittenArchiveFilesColor = @Variant(\\0\\0\\0\\x43\\x1@@\\0\\0\\xff\\xff\\xff\\xff\\0\\0)\n"
"overwritingArchiveFilesColor = @Variant(\\0\\0\\0\\x43\\x1@@\\xff\\xff\\0\\0\\xff\\xff\\0\\0)\n"
"containsPluginColor = @Variant(\\0\\0\\0\\x43\\x1@@\\0\\0\\0\\0\\xff\\xff\\0\\0)\n"
"containedColor = @Variant(\\0\\0\\0\\x43\\x1@@\\0\\0\\0\\0\\xff\\xff\\0\\0)\n"
"colorSeparatorScrollbars = true\n"
"display_foreign = true\n"
"collapsible_separators_asc = true\n"
"collapsible_separators_dsc = true\n"
"collapsible_separators_conflicts_from = true\n"
"collapsible_separators_conflicts_to = true\n"
"collapsible_separators_per_profile = false\n"
"save_filters = false\n"
"auto_collapse_on_hover = false\n"
"autocheck_update_install = true\n"
"collapsible_separators_icons_1 = true\n"
"collapsible_separators_icons_2 = true\n"
"collapsible_separators_icons_3 = true\n"
"collapsible_separators_icons_7 = true\n"
"log_level = 1\n"
"crash_dumps_type = 1\n"
"crash_dumps_max = 5\n"
"loot_log_level = 2\n"
"endorsement_integration = true\n"
"tracked_integration = true\n"
"category_mappings = true\n"
"hide_api_counter = false\n"
"force_enable_core_files = true\n"
"lock_gui = false\n"
"archive_parsing_experimental = false\n"
"offline_mode = true\n" // Автономный режим без интернета - по умолчанию true включено
"use_proxy = false\n"
"use_custom_browser = false\n"
"custom_browser =\n"
"executable_blacklist =\"Chrome.exe;Firefox.exe;TSVNCache.exe;TGitCache.exe;Steam.exe;GameOverlayUI.exe;Discord.exe;GalaxyClient.exe;Spotify.exe;Brave.exe\"\n"
"skip_file_suffixes = .mohidden\n"
"skip_directories = .git\n"
"style = 1809 Dark Mode.qss\n"
"\n"
"\n"
"[customExecutables]\n"
"size=4\n"
"1\\arguments=\n"                    // параметры запуска
"1\\binary=%s/AnomalyDX11.exe\n" // целевой файл exepath.c_str() 3
"1\\hide=false\n"                    // скрыть из списка
"1\\ownicon=true\n"
"1\\steamAppID=\n"
"1\\title=Anomaly (DX11)\n"
"1\\toolbar=false\n"                 // отображение в тулбаре сверху
"1\\workingDirectory=%s/bin\n"       // рабочая директория exepath.c_str() 4
"2\\arguments=\n"
"2\\binary=%s/AnomalyDX10.exe\n" // exepath.c_str() 5
"2\\hide=false\n"
"2\\ownicon=true\n"
"2\\steamAppID=\n"
"2\\title=Anomaly (DX10)\n"
"2\\toolbar=false\n"
"2\\workingDirectory=%s/bin\n"       // exepath.c_str() 6
"3\\arguments=\n"
"3\\binary=%s/AnomalyDX8.exe\n"  // exepath.c_str() 7
"3\\hide=false\n"
"3\\ownicon=true\n"
"3\\steamAppID=\n"
"3\\title=Anomaly (DX8)\n"
"3\\toolbar=false\n"
"3\\workingDirectory=%s/bin\n"       // exepath.c_str() 8
"4\\arguments=\n"
"4\\binary=%s/AnomalyLauncher.exe\n" // exepath.c_str() 9
"4\\hide=false\n"
"4\\ownicon=true\n"
"4\\steamAppID=\n"
"4\\title=Anomaly Launcher\n"
"4\\toolbar=false\n"
"4\\workingDirectory=%s\n"           // exepath.c_str() 10
"\n"
"[recentDirectories]\n"
"size = 2\n"
"1\\name=editExecutableBinary\n"     // хз что это
"1\\directory=%s\n"                  // exepath.c_str() 11
"2\\name=editExecutableDirectory\n"
"2\\directory=%s\n"                  // exepath.c_str() 12
"\n"
"[Geometry]\n"
"EditExecutablesDialog_geometry = @ByteArray(\\x1\\xd9\\xd0\\xcb\\0\\x3\\0\\0\\0\\0\\x1:\\0\\0\\0\\xa0\\0\\0\\x3\\xff\\0\\0\\x2u\\0\\0\\x1:\\0\\0\\0\\xbe\\0\\0\\x3\\xff\\0\\0\\x2u\\0\\0\\0\\0\\0\\0\\0\\0\\x6\\0\\0\\0\\x1:\\0\\0\\0\\xbe\\0\\0\\x3\\xff\\0\\0\\x2u)\n"
"MainWindow_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\0\\xfd\\0\\0\\0\\x1\\0\\0\\0\\x2\\0\\0\\x4j\\0\\0\\0Y\\xfc\\x1\\0\\0\\0\\x1\\xfb\\0\\0\\0\\xe\\0l\\0o\\0g\\0\\x44\\0o\\0\\x63\\0k\\x1\\0\\0\\0\\0\\0\\0\\x4j\\0\\0\\0R\\0\\xff\\xff\\xff\\0\\0\\x4j\\0\\0\\x1\\xef\\0\\0\\0\\x4\\0\\0\\0\\x4\\0\\0\\0\\b\\0\\0\\0\\b\\xfc\\0\\0\\0\\x1\\0\\0\\0\\x2\\0\\0\\0\\x1\\0\\0\\0\\xe\\0t\\0o\\0o\\0l\\0\\x42\\0\\x61\\0r\\x1\\0\\0\\0\\0\\xff\\xff\\xff\\xff\\0\\0\\0\\0\\0\\0\\0\\0)\n"
"MainWindow_geometry = @ByteArray(\\x1\\xd9\\xd0\\xcb\\0\\x3\\0\\0\\0\\0\\0\\xb4\\0\\0\\0\\x16\\0\\0\\x5\\x1d\\0\\0\\x2\\xe5\\0\\0\\0\\xb4\\0\\0\\0\\x34\\0\\0\\x5\\x1d\\0\\0\\x2\\xe5\\0\\0\\0\\0\\0\\0\\0\\0\\x6\\0\\0\\0\\0\\xb4\\0\\0\\0\\x34\\0\\0\\x5\\x1d\\0\\0\\x2\\xe5)\n"
"MainWindow_docks_logDock_size = 89\n"
"MainWindow_menuBar_visibility = true\n"
"MainWindow_statusBar_visibility = true\n"
"MainWindow_toolBar_visibility = true\n"
"toolbar_size = @Size(42 36)\n"
"toolbar_button_style = 0\n"
"MainWindow_splitter_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\x1\\0\\0\\0\\x2\\0\\0\\x3\\xde\\0\\0\\x2\\xb0\\x1\\xff\\xff\\xff\\xff\\x1\\0\\0\\0\\x1\\0)\n"
"MainWindow_categoriesSplitter_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\x1\\0\\0\\0\\x2\\0\\0\\x1\\b\\0\\0\\x2\\xe8\\0\\xff\\xff\\xff\\xff\\x1\\0\\0\\0\\x1\\0)\n"
"MainWindow_monitor = 0\n"
"MainWindow_categoriesGroup_visibility = false\n"
"MainWindow_espList_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x2\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x2p\\0\\0\\0\\x4\\x1\\x1\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\x64\\xff\\xff\\xff\\xff\\0\\0\\0\\x81\\0\\0\\0\\0\\0\\0\\0\\x4\\0\\0\\x1\\xc0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x32\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0I\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x35\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\x3\\xe8\\0\\0\\0\\0\\x35\\0\\0\\0\\0)\n"
"MainWindow_downloadView_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\x1\\0\\0\\0\\x1\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\b\\xf0\\0\\0\\0\\x4\\0\\0\\0\\a\\0\\0\\0\\x64\\0\\0\\0\\x5\\0\\0\\0\\x64\\0\\0\\0\\x6\\0\\0\\0\\x64\\0\\0\\0\\x4\\0\\0\\0\\x64\\0\\0\\x1 > \\0\\0\\0\\b\\x1\\x1\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x64\\xff\\xff\\xff\\xff\\0\\0\\0\\x81\\0\\0\\0\\0\\0\\0\\0\\b\\0\\0\\0\\x32\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x44\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\x3\\xe8\\x1\\0\\0\\0\\0\\0\\0\\0\\0)\n"
"MainWindow_savegameList_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\xad\\0\\0\\0\\x2\\x1\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x64\\xff\\xff\\xff\\xff\\0\\0\\0\\x81\\0\\0\\0\\0\\0\\0\\0\\x2\\0\\0\\0\\xfd\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\xb0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\x3\\xe8\\0\\0\\0\\0\\x64\\0\\0\\0\\0)\n"
"MainWindow_dataTree_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x2\\x9b\\0\\0\\0\\x5\\x1\\x1\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x64\\xff\\xff\\xff\\xff\\0\\0\\0\\x81\\0\\0\\0\\0\\0\\0\\0\\x5\\0\\0\\x1\\v\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\x3\\xe8\\0\\0\\0\\0\\x64\\0\\0\\0\\0)\n"
"MainWindow_modList_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\t\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\vh\\x5\\0\\0\\0\\x5\\0\\0\\0\\x5\\0\\0\\0 \\0\\0\\0\\b\\0\\0\\0 \\0\\0\\0\\n\\0\\0\\0 \\0\\0\\0\\x3\\0\\0\\0 \\0\\0\\0\\x6\\0\\0\\0 \\0\\0\\x2\\xf0\\0\\0\\0\\v\\x1\\x1\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0'\\xff\\xff\\xff\\xff\\0\\0\\0\\x81\\0\\0\\0\\0\\0\\0\\0\\v\\0\\0\\x1\\x8e\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0N\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x32\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0G\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0R\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0I\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\x3\\xe8\\x1\\0\\0\\0I\\0\\0\\0\\0)\n"
"ModInfoDialog_tabTextSplitter_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\x1\\0\\0\\0\\x2\\0\\0\\0\\xc8\\0\\0\\x1\\x19\\0\\xff\\xff\\xff\\xff\\x1\\0\\0\\0\\x1\\0)\n"
"ModInfoDialog_tabIniSplitter_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\x1\\0\\0\\0\\x2\\0\\0\\0\\xc8\\0\\0\\x1\\x19\\x1\\xff\\xff\\xff\\xff\\x1\\0\\0\\0\\x1\\0)\n"
"ModInfoDialog_tabImagesSplitter_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\x1\\0\\0\\0\\x2\\0\\0\\0\\x91\\0\\0\\0\\x84\\0\\xff\\xff\\xff\\xff\\x1\\0\\0\\0\\x1\\0)\n"
"ModInfoDialog_ESPsSplitter_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\x1\\0\\0\\0\\x2\\0\\0\\0t\\0\\0\\0l\\x1\\xff\\xff\\xff\\xff\\x1\\0\\0\\0\\x1\\0)\n"
"ModInfoDialog_overwriteExpander_state = @ByteArray(\\x1)\n"
"ModInfoDialog_overwrittenExpander_state = @ByteArray(\\0)\n"
"ModInfoDialog_noConflictExpander_state = @ByteArray(\\0)\n"
"ModInfoDialog_overwriteTree_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\x1\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x3\\xa2\\0\\0\\0\\x2\\x1\\x1\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x64\\xff\\xff\\xff\\xff\\0\\0\\0\\x81\\0\\0\\0\\0\\0\\0\\0\\x2\\0\\0\\x1\\xdf\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\x1\\xc3\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\x3\\xe8\\0\\0\\0\\0\\x64\\0\\0\\0\\0)\n"
"ModInfoDialog_noConflictTree_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\x1\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\x1\\x1\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x64\\xff\\xff\\xff\\xff\\0\\0\\0\\x81\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\x3\\xe8\\0\\0\\0\\0\\x64\\0\\0\\0\\0)\n"
"ModInfoDialog_overwrittenTree_state = @ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\x1\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x2\\xe7\\0\\0\\0\\x2\\x1\\x1\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x64\\xff\\xff\\xff\\xff\\0\\0\\0\\x81\\0\\0\\0\\0\\0\\0\\0\\x2\\0\\0\\x2\\x83\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\x3\\xe8\\0\\0\\0\\0\\x64\\0\\0\\0\\0)\n"
"ModInfoDialog_conflictsAdvancedList_state = \"@ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\x1\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1,\\0\\0\\0\\x3\\x1\\x1\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x64\\xff\\xff\\xff\\xff\\0\\0\\0\\x81\\0\\0\\0\\0\\0\\0\\0\\x3\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\x3\\xe8\\0\\0\\0\\0\\x64\\0\\0\\0\\0)\"\n"
"ModInfoDialog_filetree_state = \"@ByteArray(\\0\\0\\0\\xff\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x2l\\0\\0\\0\\x4\\x1\\x1\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x64\\xff\\xff\\xff\\xff\\0\\0\\0\\x81\\0\\0\\0\\0\\0\\0\\0\\x4\\0\\0\\x1,\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0\\x64\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\0x\\0\\0\\0\\x1\\0\\0\\0\\0\\0\\0\\x3\\xe8\\0\\0\\0\\0\\x64\\0\\0\\0\\0)\"\n"
"ModInfoDialog_geometry = @ByteArray(\\x1\\xd9\\xd0\\xcb\\0\\x3\\0\\0\\0\\0\\x1\\xf\\0\\0\\0\\xa0\\0\\0\\x4\\xf0\\0\\0\\x2\\xf3\\0\\0\\x1\\xf\\0\\0\\0\\xbe\\0\\0\\x4\\xf0\\0\\0\\x2\\xf3\\0\\0\\0\\0\\0\\0\\0\\0\\x6\\0\\0\\0\\x1\\xf\\0\\0\\0\\xbe\\0\\0\\x4\\xf0\\0\\0\\x2\\xf3)\n"
"SettingsDialog_geometry = @ByteArray(\\x1\\xd9\\xd0\\xcb\\0\\x3\\0\\0\\0\\0\\x1\\x8e\\0\\0\\0\\x17\\0\\0\\x4\\xc1\\0\\0\\x2\\x93\\0\\0\\x1\\x8e\\0\\0\\0\\x35\\0\\0\\x4\\xc1\\0\\0\\x2\\x93\\0\\0\\0\\0\\0\\0\\0\\0\\x6\\0\\0\\0\\x1\\x8e\\0\\0\\0\\x35\\0\\0\\x4\\xc1\\0\\0\\x2\\x93)\n"
"PreviewDialog_geometry = \"@ByteArray(\\x1\\xd9\\xd0\\xcb\\0\\x3\\0\\0\\0\\0\\0\\xc3\\0\\0\\0=\\0\\0\\x3\\xa2\\0\\0\\x2\\xa1\\0\\0\\0\\xc3\\0\\0\\0[\\0\\0\\x3\\xa2\\0\\0\\x2\\xa1\\0\\0\\0\\0\\0\\0\\0\\0\\x6\\0\\0\\0\\0\\xc3\\0\\0\\0[\\0\\0\\x3\\xa2\\0\\0\\x2\\xa1)\"\n"
"\n"
"[Widgets]\n"
"MainWindow_executablesListBox_index = 4\n"
"MainWindow_tabWidget_index = 0\n"
"MainWindow_dataTabShowOnlyConflicts_checked = false\n"
"MainWindow_dataTabShowFromArchives_checked = false\n"
"MainWindow_groupCombo_index = 0\n"
"MainWindow_modList_index = 1.0 Main Mod, 1.1 Patches, 7.0 Graphic, 8.0 Performance, 99.0 The mods of your choice, New Storyline of Monolith, Overwrite\n"
"MainWindow_filters_index = 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0\n"
"MainWindow_filtersAnd_checked = true\n"
"MainWindow_filtersOr_checked = false\n"
"MainWindow_filtersSeparators_index = 0\n"
"ModInfoTabOrder = tabText tabIni tabImages tabESPs tabConflicts tabCategories tabNexus tabNotes tabFiles\n"
"ModInfoDialog_tabWidget_index = 0\n"
"ModInfoDialog_imagesShowDDS_checked = false\n"
"ModInfoDialog_tabConflictsTabs_index = 0\n"
"ModInfoDialog_conflictsAdvancedShowNoConflict_checked = true\n"
"ModInfoDialog_conflictsAdvancedShowAll_checked = true\n"
"ModInfoDialog_conflictsAdvancedShowNearest_checked = false\n"
"SettingsDialog_tabWidget_index = 3\n"
"\n"
"[Servers]\n"
"size = 0\n"
"\n"
"[Plugins]\n"
"BAIN %% 20Wizard %% 20Installer\\prefer_fomod = true\n"
"BAIN %% 20Wizard %% 20Installer\\prefer_omod = false\n"
"BAIN %% 20Wizard %% 20Installer\\priority = 120\n"
"BSA %% 20Extractor\\only_alternate_source = true\n"
"BSA %% 20Packer\\blacklisted_files = \".txt;.hkx;.xml;.ini;.bk2\"\n"
"BSA %% 20Packer\\compress_archives = true\n"
"BSA %% 20Packer\\create_plugins = false\n"
"BSA %% 20Packer\\hide_loose_assets = false\n"
"Basic %% 20diagnosis %% 20plugin\\check_alternategames = false\n"
"Basic %% 20diagnosis %% 20plugin\\check_conflict = true\n"
"Basic %% 20diagnosis %% 20plugin\\check_errorlog = true\n"
"Basic %% 20diagnosis %% 20plugin\\check_fileattributes = false\n"
"Basic %% 20diagnosis %% 20plugin\\check_font = true\n"
"Basic %% 20diagnosis %% 20plugin\\check_missingmasters = true\n"
"Basic %% 20diagnosis %% 20plugin\\check_overwrite = true\n"
"Basic %% 20diagnosis %% 20plugin\\ow_ignore_empty = false\n"
"Basic %% 20diagnosis %% 20plugin\\ow_ignore_log = false\n"
"Bundle %% 20Installer\\auto_reinstall = true\n"
"Cyberpunk %% 202077 %% 20Support %% 20Plugin\\auto_deploy_redmod = true\n"
"Cyberpunk %% 202077 %% 20Support %% 20Plugin\\clear_cache_after_game_update = true\n"
"Cyberpunk %% 202077 %% 20Support %% 20Plugin\\configure_RootBuilder = true\n"
"Cyberpunk %% 202077 %% 20Support %% 20Plugin\\enforce_archive_load_order = false\n"
"Cyberpunk %% 202077 %% 20Support %% 20Plugin\\enforce_redmod_load_order = true\n"
"Cyberpunk %% 202077 %% 20Support %% 20Plugin\\reverse_archive_load_order = false\n"
"Cyberpunk %% 202077 %% 20Support %% 20Plugin\\reverse_redmod_load_order = false\n"
"Cyberpunk %% 202077 %% 20Support %% 20Plugin\\skipStartScreen = false\n"
"DDS %% 20Preview %% 20Plugin\\background %% 20a = 0\n"
"DDS %% 20Preview %% 20Plugin\\background %% 20b = 0\n"
"DDS %% 20Preview %% 20Plugin\\background %% 20g = 0\n"
"DDS %% 20Preview %% 20Plugin\\background %% 20r = 0\n"
"DDS %% 20Preview %% 20Plugin\\channels = RGBA\n"
"DDS %% 20Preview %% 20Plugin\\log %% 20gl %% 20errors = false\n"
"Enderal %% 20Support %% 20Plugin\\sse_downloads = false\n"
"FNIS %% 20Checker\\sensitive = false\n"
"FNIS %% 20Integration %% 20Tool\\fnis - path =\n"
"FNIS %% 20Integration %% 20Tool\\initialised = false\n"
"FNIS %% 20Integration %% 20Tool\\output - logs - path = \n"
"FNIS %% 20Integration %% 20Tool\\output - logs - to - mod = true\n"
"FNIS %% 20Integration %% 20Tool\\output - path =\n"
"FNIS %% 20Integration %% 20Tool\\output - to - mod = true\n"
"Fallout %% 20NV %% 20Support %% 20Plugin\\enable_loot_sorting = false\n"
"Fallout %% 20TTW %% 20Support %% 20Plugin\\enable_loot_sorting = false\n"
"Fomod %% 20Installer\\prefer = true\n"
"Fomod %% 20Installer\\see_disabled_mods = false\n"
"Fomod %% 20Installer\\use_any_file = false\n"
"Fomod %% 20Installer %% 20C %% 23\\prefer = true\n"
"INI %% 20Editor\\associated = true\n"
"INI %% 20Editor\\external = false\n"
"Oblivion %% 20Support %% 20Plugin\\nehrim_downloads = false\n"
"Preview %% 20Base\\blacklisted_extensions =\n"
"Simple %% 20Installer\\silent = false\n"
"Skyrim %% 20Special %% 20Edition %% 20Support %% 20Plugin\\enderal_downloads = false\n"
"Skyrim %% 20Support %% 20Plugin\\sse_downloads = false\n"
"Skyrim %% 20VR %% 20Support %% 20Plugin\\enderal_downloads = false\n"
"Starfield %% 20Support %% 20Plugin\\enable_esp_warning = true\n"
"Starfield %% 20Support %% 20Plugin\\enable_loadorder_fix = true\n"
"Starfield %% 20Support %% 20Plugin\\enable_management_warnings = true\n"
"Subnautica %% 20Below %% 20Zero %% 20Support %% 20Plugin\\use_qmods = false\n"
"Subnautica %% 20Support %% 20Plugin\\use_qmods = false\n"
"Valheim %% 20Support %% 20Plugin\\search_overwrite_file_content = true\n"
"Valheim %% 20Support %% 20Plugin\\sync_overwrite = true\n"
"\n"
"[pluginBlacklist]\n"
"size = 0\n";

fprintf(f, config_template,
        ba_profile.c_str(),
        ba_gamepath.c_str(),              // В этом поле будет @ByteArray(E:/...)

        bin_path.c_str(), gamepathUtf8.c_str(), // 1
        bin_path.c_str(), gamepathUtf8.c_str(), // 2
        bin_path.c_str(), gamepathUtf8.c_str(), // 3
        gamepathUtf8.c_str(), gamepathUtf8.c_str(), // 4

        gamepathUtf8.c_str(), gamepathUtf8.c_str() // recentDirectories
    );

    fclose(f);
    f = NULL;// жесткий сброс
    //sn->con.AddLog("ModOrganizer.ini успешно записан.");
    sn->con.AddLog("Обнаружена папка > {FFFF00} MO2 {FFFFFF}| патчим {AAFFBB}ModOrganizer.ini");
}