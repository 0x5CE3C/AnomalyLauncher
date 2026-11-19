#pragma once

#include <string>
#include "LauncherConfig.h"
#include <regex>


struct FileListItem {
    int id;
    std::string filename;
    std::string dir;
    std::string moduleType;
    std::string fileType;
    std::string hash;
    std::string downloadFilename;
    std::string downloadUrl;
};

class FileListGenerator {
public:
    static bool generate(const char* outPath);
};

std::string extractVersion(const std::string& filename);

bool isPathExcluded(const std::string& fullPath, const std::string& ruleFolder, ModuleType currentModType, ModuleType ruleType);

int RunCmdNoWindow(const std::string& cmdAnsi);

void GenerateModsToZip();