#pragma once

void funcs_enabler();

// Scanning modules:
/// Check Exist AnomalyLauncher updates folder
std::string GetLUFolder();
void CheckAnomalyLaucherFolder();

/// Check Exist Anomaly
std::string GetAnomalybinFolder();
std::string GetAnomalydbFolder();
std::string GetAnomalygamedataFolder();
std::string GetAnomalyFolder();

/// Check MO2 if installed
std::string GetMO2Folder();

/// Check Blue Eclipse if installed
std::string GetBEFolder();

bool IsCheckAVXSupport();
void RestoreUserltx();
void PatchModOrganizerbtn();


void PatchModOrganizerfunc(const char* fmt, ...);

bool DetectLaunchbyModOrganizer();
extern float amplitude;


enum PROJECTMODES
{
	PM_Anomaly_PZ,
	PM_Anomaly_MP,
};