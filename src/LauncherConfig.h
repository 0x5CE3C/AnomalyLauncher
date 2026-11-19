#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

enum class ModuleType {
    Extractor,
    Anomaly,
    MO2,
    PZ,
    All
};

struct LauncherPaths {
    fs::path pathExtractor;
    fs::path pathAnomaly;
    fs::path pathMO2;
    fs::path pathPZ;

    fs::path rootPath; // <-- ÝÒÎ ÍÎÂÎÅ: îáùèé êîðåíü (E:\AnomalyPZ)
};

class LauncherConfig {
public:
    static LauncherConfig& getInstance();

    void setPaths(const LauncherPaths& paths) { m_paths = paths; }
    const LauncherPaths& getPaths() const { return m_paths; }

    std::string getPathStr(ModuleType type) const;

private:
    LauncherConfig() {}
    LauncherPaths m_paths;
};