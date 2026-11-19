#include "main.h"
#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cctype>

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::init(const std::string& exePath) {
    // ИСПРАВЛЕНИЕ: добавляем .string() для конвертации пути в строку
    m_configPath = fs::path(exePath).parent_path().string() + "\\base_filelist.list";

    load();
}

ModuleType ConfigManager::parseModuleType(const std::string& str) {
    if (str == "all") return ModuleType::All;
    if (str == "anomaly") return ModuleType::Anomaly;
    if (str == "mo2") return ModuleType::MO2;
    if (str == "pz") return ModuleType::PZ;
    if (str == "extractor") return ModuleType::Extractor;
    return ModuleType::All;
}

std::string ConfigManager::moduleTypeToString(ModuleType t) {
    switch (t) {
    case ModuleType::All: return "all";
    case ModuleType::Anomaly: return "anomaly";
    case ModuleType::MO2: return "mo2";
    case ModuleType::PZ: return "pz";
    case ModuleType::Extractor: return "extractor";
    default: return "all";
    }
}

void ConfigManager::load() {
    if (!fs::exists(m_configPath)) {
        initDefaults();
        save();
        std::cout << "[Config] Создан новый base_filelist.list" << std::endl;
        return;
    }

    std::ifstream file(m_configPath);
    if (!file.is_open()) {
        std::cerr << "[Config] Ошибка открытия файла конфигурации!" << std::endl;
        initDefaults();
        return;
    }

    m_binFiles.clear();
    m_blacklistRules.clear();

    std::string line;
    bool inBinSection = true;

    while (std::getline(file, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);

        if (line.empty() || line[0] == '#') continue;

        if (line == "[FILES]") { inBinSection = true; continue; }
        if (line == "[BLACKLIST]") { inBinSection = false; continue; }

        for (char& c : line) if (c == '/') c = '\\';

        if (inBinSection) {
            // Формат: путь|тип  (например: \bin\AnomalyDX11.exe|anomaly)
            size_t pos = line.find('|');
            BinFileRule rule;
            if (pos != std::string::npos) {
                rule.path = line.substr(0, pos);
                std::string typeStr = line.substr(pos + 1);
                size_t tStart = typeStr.find_first_not_of(" ");
                if (tStart != std::string::npos) {
                    size_t tEnd = typeStr.find_last_not_of(" ");
                    typeStr = typeStr.substr(tStart, tEnd - tStart + 1);
                }
                rule.type = parseModuleType(typeStr);
            }
            else {
                // Если типа нет — считаем All
                rule.path = line;
                rule.type = ModuleType::All;
            }
            m_binFiles.push_back(rule);
        }
        else {
            size_t pos = line.find('|');
            BlacklistRule rule;
            if (pos != std::string::npos) {
                rule.folderName = line.substr(0, pos);
                std::string typeStr = line.substr(pos + 1);
                size_t tStart = typeStr.find_first_not_of(" ");
                if (tStart != std::string::npos) {
                    size_t tEnd = typeStr.find_last_not_of(" ");
                    typeStr = typeStr.substr(tStart, tEnd - tStart + 1);
                }
                rule.type = parseModuleType(typeStr);
            }
            else {
                rule.folderName = line;
                rule.type = ModuleType::All;
            }
            m_blacklistRules.push_back(rule);
        }
    }
    file.close();

    if (m_binFiles.empty() && m_blacklistRules.empty()) {
        initDefaults();
        save();
    }
}


void ConfigManager::save() {
    std::ofstream file(m_configPath);
    if (!file.is_open()) return;

    file << "# Base FileList Configuration\n";
    file << "# Format: path|type (all, anomaly, mo2, pz, extractor)\n\n";

    file << "[FILES]\n";
    for (const auto& item : m_binFiles) {
        file << item.path << "|" << moduleTypeToString(item.type) << "\n";
    }

    file << "\n[BLACKLIST]\n";
    for (const auto& rule : m_blacklistRules) {
        file << rule.folderName << "|" << moduleTypeToString(rule.type) << "\n";
    }
}


void ConfigManager::initDefaults() {
    // Теперь храним и путь, и тип
    // Базовые файлы и тип
    m_binFiles = {
        {"AnomalyDX8.exe", ModuleType::Anomaly},
        {"AnomalyDX9.exe", ModuleType::Anomaly},
        {"AnomalyDX10.exe", ModuleType::Anomaly},
        {"AnomalyDX11.exe", ModuleType::Anomaly},
        {"AnomalyDX8AVX.exe", ModuleType::Anomaly},
        {"AnomalyDX9AVX.exe", ModuleType::Anomaly},
        {"AnomalyDX10AVX.exe", ModuleType::Anomaly},
        {"AnomalyDX11AVX.exe", ModuleType::Anomaly}
        /*"E:\AnomalyPZ\db\configs"
"E:\AnomalyPZ\db\levels"
"E:\AnomalyPZ\db\meshes"
"E:\AnomalyPZ\db\sounds"
"E:\AnomalyPZ\db\textures"
"E:\AnomalyPZ\db\files.db0"
"E:\AnomalyPZ\db\shaders.db0"
"E:\AnomalyPZ\db\configs\ai.db0"
"E:\AnomalyPZ\db\configs\configs.db0"
"E:\AnomalyPZ\db\configs\scripts.db0"
"E:\AnomalyPZ\db\configs\spawns.db0"
"E:\AnomalyPZ\db\levels\level_jupiter.db0"
"E:\AnomalyPZ\db\levels\level_jupiter_underground.db0"
"E:\AnomalyPZ\db\levels\level_k00_marsh.db0"
"E:\AnomalyPZ\db\levels\level_k01_darkscape.db0"
"E:\AnomalyPZ\db\levels\level_k02_trucks_cemetery.db0"
"E:\AnomalyPZ\db\levels\level_l01_escape.db0"
"E:\AnomalyPZ\db\levels\level_l02_garbage.db0"
"E:\AnomalyPZ\db\levels\level_l03_agroprom.db0"
"E:\AnomalyPZ\db\levels\level_l03u_agr_underground.db0"
"E:\AnomalyPZ\db\levels\level_l04_darkvalley.db0"
"E:\AnomalyPZ\db\levels\level_l04u_labx18.db0"
"E:\AnomalyPZ\db\levels\level_l05_bar.db0"
"E:\AnomalyPZ\db\levels\level_l06_rostok.db0"
"E:\AnomalyPZ\db\levels\level_l07_military.db0"
"E:\AnomalyPZ\db\levels\level_l08_yantar.db0"
"E:\AnomalyPZ\db\levels\level_l08u_brainlab.db0"
"E:\AnomalyPZ\db\levels\level_l09_deadcity.db0"
"E:\AnomalyPZ\db\levels\level_l10_limansk.db0"
"E:\AnomalyPZ\db\levels\level_l10_radar.db0"
"E:\AnomalyPZ\db\levels\level_l10_red_forest.db0"
"E:\AnomalyPZ\db\levels\level_l10u_bunker.db0"
"E:\AnomalyPZ\db\levels\level_l11_hospital.db0"
"E:\AnomalyPZ\db\levels\level_l11_pripyat.db0"
"E:\AnomalyPZ\db\levels\level_l12_stancia.db0"
"E:\AnomalyPZ\db\levels\level_l12_stancia_2.db0"
"E:\AnomalyPZ\db\levels\level_l12u_control_monolith.db0"
"E:\AnomalyPZ\db\levels\level_l12u_sarcofag.db0"
"E:\AnomalyPZ\db\levels\level_l13_generators.db0"
"E:\AnomalyPZ\db\levels\level_l13u_warlab.db0"
"E:\AnomalyPZ\db\levels\level_labx8.db0"
"E:\AnomalyPZ\db\levels\level_pripyat.db0"
"E:\AnomalyPZ\db\levels\level_y04_pole.db0"
"E:\AnomalyPZ\db\levels\level_zaton.db0"
"E:\AnomalyPZ\db\levels\levels.db0"
"E:\AnomalyPZ\db\meshes\anims.db0"
"E:\AnomalyPZ\db\meshes\meshes.db0"
"E:\AnomalyPZ\db\meshes\meshes_actors.db0"
"E:\AnomalyPZ\db\meshes\meshes_dynamics.db0"
"E:\AnomalyPZ\db\meshes\meshes_weapons.db0"
"E:\AnomalyPZ\db\sounds\sounds.db0"
"E:\AnomalyPZ\db\sounds\sounds_ambient.db0"
"E:\AnomalyPZ\db\sounds\sounds_music.db0"
"E:\AnomalyPZ\db\sounds\sounds_voices.db0"
"E:\AnomalyPZ\db\textures\textures.db0"
"E:\AnomalyPZ\db\textures\textures.db1"
"E:\AnomalyPZ\db\textures\textures.db2"
"E:\AnomalyPZ\db\textures\textures.db3"
"E:\AnomalyPZ\db\textures\textures.db4"
"E:\AnomalyPZ\db\textures\textures_actors.db0"
"E:\AnomalyPZ\db\textures\textures_actors.db1"
"E:\AnomalyPZ\db\textures\textures_concrete.db0"
"E:\AnomalyPZ\db\textures\textures_concrete.db1"
"E:\AnomalyPZ\db\textures\textures_detail.db0"
"E:\AnomalyPZ\db\textures\textures_metal.db0"
"E:\AnomalyPZ\db\textures\textures_metal.db1"
"E:\AnomalyPZ\db\textures\textures_sky.db0"
"E:\AnomalyPZ\db\textures\textures_stone.db0"
"E:\AnomalyPZ\db\textures\textures_ui.db0"
"E:\AnomalyPZ\db\textures\textures_weapons.db0"
"E:\AnomalyPZ\db\textures\textures_weapons.db1"
"E:\AnomalyPZ\db\textures\textures_weapons.db2"*/
    };

    // Базовый Черный список
    m_blacklistRules = {
        {"temp", ModuleType::All},
        {"components", ModuleType::All},
        {"gamedata", ModuleType::All},          
        {"mods", ModuleType::MO2},
        {"crashDumps", ModuleType::MO2}, 
        {"ModOrganizer.ini", ModuleType::MO2},        
        {"nxmhandler.log", ModuleType::MO2},
        {"webcache", ModuleType::MO2},
        {"overwrite", ModuleType::MO2},
        {"logs", ModuleType::MO2},
        {"downloads", ModuleType::MO2},
        {"MO2", ModuleType::Anomaly},        
        {"AnomalyLauncher", ModuleType::Anomaly},        
        {"AnomalyLauncher.pdb", ModuleType::All},
        {"appdata", ModuleType::Anomaly},
        {"base_filelist.list", ModuleType::Extractor},
        {"files_list.ini", ModuleType::Extractor},
        {"AnomalyLauncher.ini", ModuleType::Extractor},
        {"AnomalyLauncher.log", ModuleType::Extractor},
        {"be_menu_file_downloading.mp3", ModuleType::Extractor},
        {"be_menu_log_message.mp3", ModuleType::Extractor},
        {"be_menu_main_item_nav.mp3", ModuleType::Extractor},
        {"be_menu_nav.mp3", ModuleType::Extractor},
        {"be_menu_select.mp3", ModuleType::Extractor},
        {"be_theme.mp3", ModuleType::Extractor},
        {"launcher_last_update.ini", ModuleType::Extractor},
        {"modpack_changelist.ini", ModuleType::Extractor}
    };
}

void ConfigManager::addBinFile(const std::string& path, ModuleType type) {
    for (const auto& r : m_binFiles) {
        if (r.path == path && r.type == type) return; // Дубликат
    }
    m_binFiles.push_back({ path, type });
    save();
}
void ConfigManager::removeBinFile(size_t index) {
    if (index < m_binFiles.size()) {
        m_binFiles.erase(m_binFiles.begin() + index);
        save();
    }
}

void ConfigManager::addBlackListRule(const std::string& folder, ModuleType type) {
    for (const auto& r : m_blacklistRules) {
        if (r.folderName == folder && r.type == type) return;
    }
    BlacklistRule rule{ folder, type };
    m_blacklistRules.push_back(rule);
    save();
}

void ConfigManager::removeBlackListRule(size_t index) {
    if (index < m_blacklistRules.size()) {
        m_blacklistRules.erase(m_blacklistRules.begin() + index);
        save();
    }
}