#pragma once

enum ScanModuleTypes
{
	SCAN_MODULE_TYPE_ANOMALY = 1,
	SCAN_MODULE_TYPE_MO2 = 2,
	SCAN_MODULE_TYPE_BE = 3,
	SCAN_MODULE_TYPE_EXTRACTOR = 4,
};

void ScanModule(int m_type);

extern bool scanner_anomaly_enable;
extern bool module_anomaly_damaged;

extern bool scanner_mo2_enable;
extern bool module_mo2_damaged;

extern bool scanner_be_enable;
extern bool module_be_damaged;

extern bool scanner_extractor_enable;
extern bool module_extractor_damaged;

void CheckDirectory(int i, int& dirs_found, int& dirs_not_found, int& dirs_match, int& dirs_no_match);

void CheckFile(int i, int& files_found, int& files_not_found, int& files_match, int& files_no_match);

void ProcessDamagedModule(int i);

void StopScanning(int m_type, int& dirs_not_found, int& dirs_no_match, int& files_not_found, int& files_no_match);

void LogSummary(int m_type, int dirs_found, int dirs_not_found, int dirs_match, int dirs_no_match, int files_found, int files_not_found, int files_match, int files_no_match);

void ResetCounters(int m_type, int& dirs_found, int& dirs_not_found, int& dirs_match, int& dirs_no_match,  int& files_found, int& files_not_found, int& files_match, int& files_no_match);


string GetDirectoryPath(int i);
string GetFilePath(int i);