#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include "LauncherConfig.h"

namespace fs = std::filesystem;

struct BlacklistRule {
    std::string folderName;
    ModuleType type;
};

// НОВОЕ: правило для разрешённого EXE
struct BinFileRule {
    std::string path;
    ModuleType type;
};

class ConfigManager {
public:
    static ConfigManager& getInstance();

    void init(const std::string& exePath);

    //const std::vector<std::string>& getBinFiles() const { return m_binFiles; }
    const std::vector<BinFileRule>& getBinFiles() const { return m_binFiles; }
    const std::vector<BlacklistRule>& getBlackListRules() const { return m_blacklistRules; }

    // Редактирование    
    void addBinFile(const std::string& path, ModuleType type);
    void removeBinFile(size_t index);

    void addBlackListRule(const std::string& folder, ModuleType type);
    void removeBlackListRule(size_t index);

    static std::string moduleTypeToString(ModuleType t);

    // ДОБАВЬ ЭТОТ МЕТОД
    const std::string& getConfigPath() const { return m_configPath; }
private:
    ConfigManager() {}
    std::string m_configPath;

    //std::vector<std::string> m_binFiles;
    std::vector<BinFileRule> m_binFiles; // БЫЛО: vector<string>, СТАЛО: vector<BinFileRule>
    std::vector<BlacklistRule> m_blacklistRules;

    void load();
    void save();
    void initDefaults();

    static ModuleType parseModuleType(const std::string& str);
    
};