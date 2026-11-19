#pragma once

void sleep(int ms);

void FindStringsForReplace();

//vector<string> paths;
//std::string path = GetExePath() + "\\appdata";
//GetAllFilesInDir(paths, path);

void GetListFiles(std::string path);
void GetListDirs(std::string path);
std::string GetBaseDir();//Базовый путь к лаунчеру
std::string GetDownloadDir();//Каталог загрузок
int CleanupPartFiles(const std::string& dir);

std::string getFileNameWithExtension(const std::string& s);
std::string getFileNameWithoutExtension(const std::string& s);

//Return True (found) / False (not found)
bool iszip(const string& filename);

std::string GetParentProcessName();

static bool fileop_move = false;
static bool fileop_remove = false;
static bool fileop_rename = false;;
static bool dirop_move = false;;
static bool dirop_remove = false;;
static bool dirop_rename = false;;

void FileOperation(int FileOpType, std::string filename = "", std::string path = "", std::string renamedfilename = "");

void CheckDirectPlayStatus();

extern bool directPlayEnabled;
extern bool directPlayChecked;