#pragma once
#include <windows.h>
#include <iostream>



static bool zip_extracted = false;
static bool unzip = false;
static int unpacked_status = 0;


namespace zipextractor
{
	// Функция для проверки наличия 7za.exe
	bool is7zaAvailable(const std::string& zipPath) {
		return std::filesystem::exists(zipPath);
	}

	void unzipfile(std::string filename, std::string unzip_path);

	void ProcessWithDownloadedFile();

	void HandleFileExtraction(int index);
	void ProcessFileExtraction(int index);
	void HandleDisabledExtraction(int index);
	void HandleModuleAnomaly(int index);
	void RemoveFileIfExists(int index);
}

extern std::string zipPath;
extern bool is_extracting;