#include "main.h"
#include "LauncherConfig.h"

LauncherConfig& LauncherConfig::getInstance() {
    static LauncherConfig instance;
    return instance;
}

std::string LauncherConfig::getPathStr(ModuleType type) const {
    switch (type) {
    case ModuleType::Extractor: return m_paths.pathExtractor.string();
    case ModuleType::Anomaly:  return m_paths.pathAnomaly.string();
    case ModuleType::MO2:      return m_paths.pathMO2.string();
    case ModuleType::PZ:       return m_paths.pathPZ.string();
    default:                   return "";
    }
}