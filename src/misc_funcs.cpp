#include "main.h"
#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>
#include <psapi.h>

static bool func_nnabler_init = true;

void sleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
};

void GetListFiles(std::string path)
{	
	for (/*const auto& file*/ const std::filesystem::directory_entry &file: filesystem::directory_iterator(path))
	{
		if (filesystem::is_directory(file))
		{			
			sn->con.AddLog("[Сканер] > %s\\", file.path().string());		
			Log("[Сканер] > %s\\", file.path().string());
		}
		else
		{								
			sn->con.AddLog("[Сканер] > %s", file.path().string());			
			Log("[Сканер] > %s", file.path().string());
		}
	}	
};

//void GetListFiles(std::string path)
//{	
//	for (/*const auto& file*/ const std::filesystem::directory_entry &file: filesystem::recursive_directory_iterator(path))
//	{
//		if (filesystem::is_directory(file))
//		{			
//			sn->con.AddLog(u8"[Сканер] > %s\\", file.path().string());			
//		}
//		else
//		{								
//			sn->con.AddLog(u8"[Сканер] > %s", file.path().string());			
//		}
//	}	
//};

void GetListDirs(std::string path)
{
	for (/*const auto& file*/ const std::filesystem::directory_entry& file : filesystem::directory_iterator(path))
	{
		if (filesystem::is_directory(file))
		{
			sn->con.AddLog(u8"[DirsParser] > %s", file.path().string());
			Log("%s", file.path().string());
		}
	}
};



using std::string;
string getFileNameWithExtension(const string& s) {
	char sep = '/';
#ifdef _WIN32
	sep = '\\';
#endif
	size_t i = s.rfind(sep, s.length());
	if (i != string::npos)
	{
		string filename = s.substr(i + 1, s.length() - i);
		string rawname = filename.substr(0, s.length());
		return(rawname);
	}
	return("");
}
string getFileNameWithoutExtension(const string& s) {
	char sep = '/';
#ifdef _WIN32
	sep = '\\';
#endif
	size_t i = s.rfind(sep, s.length());
	if (i != string::npos)
	{
		string filename = s.substr(i + 1, s.length() - i);
		size_t lastindex = filename.find_last_of(".");
		string rawname = filename.substr(0, lastindex);
		return(rawname);
	}
	return("");
}

bool iszip(const string &filename) 
{
	std::string fext = my_strdup(filename.c_str());
	if (fext.substr(fext.find_last_of(".") + 1) == "zip")
	{
		return true;
	}
	else
	{
		return false;
	}
};

std::string GetParentProcessName()
{
	// Получаем идентификатор текущего процесса
	DWORD pID = GetCurrentProcessId();
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe32;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Получаем информацию о первом процессе
	if (Process32First(hProcessSnap, &pe32))
	{
		do
		{
			// Находим процесс, идентификатор которого равен текущему процессу
			if (pe32.th32ProcessID == pID)
			{
				// Открываем родительский процесс
				HANDLE hParentProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ParentProcessID);
				if (hParentProcess)
				{
					TCHAR parentProcessName[MAX_PATH];
					// Получаем имя родительского процесса
					GetModuleFileNameEx(hParentProcess, NULL, parentProcessName, sizeof(parentProcessName) / sizeof(TCHAR));
					CloseHandle(hParentProcess); // Закрываем дескриптор родительского процесса
					CloseHandle(hProcessSnap); // Закрываем дескриптор снимка

					std::string processname = parentProcessName;

					char sep = '/';
#ifdef _WIN32
					sep = '\\';
#endif
					size_t i = processname.rfind(sep, processname.length());
					if (i != string::npos)
					{
						string filename = processname.substr(i + 1, processname.length() - i);
						string rawname = filename.substr(0, processname.length());
						return(rawname);
					}
					//return std::string(parentProcessName); // Возвращаем имя родительского процесса
				}
			}
		} while (Process32Next(hProcessSnap, &pe32)); // Переходим к следующему процессу
	}

	CloseHandle(hProcessSnap); // Закрываем дескриптор снимка, если не удалось найти процесс
	return "Parent process not found"; // Если родительский процесс не найден
}

void FileOperation(int FileOpType, std::string filename, std::string path, std::string renamedfilename)
{
	Log("%d, %s, %s", FileOpType, filename.c_str(), path.c_str());
	switch (FileOpType)
	{
		case FileOperations::FILEOP_FILE_MOVE:
		{
			if (filename.empty() || path.empty()) { return; }

			fsm->MoveTo(filename.c_str(), path.c_str());
			fileop_move = true;
			break;
		}
		case FileOperations::FILEOP_FILE_REMOVE:
		{
			if (filename.empty() || path.empty() || renamedfilename.empty()) { return; }

			fsm->FileDelete(filename.c_str());
			fileop_remove = true;
			break;
		}
		case FileOperations::FILEOP_FILE_RENAME:
		{
			if (filename.empty() || path.empty() || renamedfilename.empty()) { return; }

			fsm->RenameFile(filename.c_str(), renamedfilename.c_str());
			fileop_rename = true;
			break;
		}
		case FileOperations::FILEOP_DIR_MOVE:
		{
			if (filename.empty() || path.empty() || renamedfilename.empty()) { return; }

			fsm->MoveTo(filename.c_str(), path.c_str());
			dirop_move = true;
			break;
		}
		case FileOperations::FILEOP_DIR_REMOVE:
		{
			if (filename.empty() || path.empty() || renamedfilename.empty()) { return; }

			fsm->DeleteFolderWithFiles(path.c_str());
			dirop_remove = true;
			break;
		}
		case FileOperations::FILEOP_DIR_RENAME:
		{
			if (filename.empty() || path.empty() || renamedfilename.empty()) { return; }
			fsm->RenameFile(filename.c_str(), renamedfilename.c_str());
			dirop_rename = true;
			break;
		}
		
	}	
};

const char* cmdLine = "DISM.exe /Online /Get-Features";

bool RunDismGetFeatures(std::vector<std::string>& outputLines) {
	// Устанавливаем кодировку вывода консоли на CP1251 (Windows-1251) для кириллицы
	//SetConsoleOutputCP(1251);
	//SetConsoleCP(1251);  // Устанавливаем кодовую страницу ввода тоже

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	HANDLE hReadPipe, hWritePipe;
	if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
		throw std::runtime_error("CreatePipe failed");
	}
	SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.hStdOutput = hWritePipe;
	si.hStdError = hWritePipe;
	si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	// Подготавливаем команду в изменяемом буфере
	std::string cmdStr = cmdLine;
	std::vector<char> cmdLine(cmdStr.c_str(), cmdStr.c_str() + cmdStr.size() + 1);

	if (!CreateProcessA(
		NULL,
		cmdStr.data(),
		NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi
	)) {
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
		return false;
	}

	CloseHandle(hWritePipe);

	std::string result;
	char buffer[512];//4096
	DWORD bytesRead;

	while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
		buffer[bytesRead] = '\0';
		for (DWORD i = 0; i < bytesRead; ++i) {
			if (buffer[i] == '\r') continue; // Пропускаем CR
			if (buffer[i] == '\n') {				
				std::string cp1251_str = cp866_to_cp1251(buffer);
				//sn->con.AddLog("%s", cp1251_str.c_str());
				outputLines.push_back(cp1251_str);
				cp1251_str.clear();
			}
			else {
				result += buffer[i];
			}
		}		
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hReadPipe);

	return true;
}



bool GetDirectPlayStatus(bool& isEnabled) {
	std::vector<std::string> lines;
	if (!RunDismGetFeatures(lines))
		return false;

	for (size_t i = 0; i + 1 < lines.size(); ++i) {
		const auto& line1 = lines[i];
		const auto& line2 = lines[i + 1];

		if (line1.find("DirectPlay") != std::string::npos) {
			// Ищем «Включен» / «Отключен» во второй строке
			if (line2.find("Включен") != std::string::npos) {
				isEnabled = true;
				return true;
			}
			else if (line2.find("Отключен") != std::string::npos) {
				isEnabled = false;
				return true;
			}
			// Если формат другой — можно добавить обработку по ключу "State" для английской локали
		}
	}
	return false; // Не найдено
}


// Где-то в классе лаунчера
bool directPlayEnabled = false;
bool directPlayChecked = false; // чтобы не запускать DISM каждый кадр

void CheckDirectPlayStatus() {
	if (directPlayChecked) return;
	directPlayChecked = true;

	bool status = false;
	if (!GetDirectPlayStatus(status)) {
		// Ошибка: нет прав админа или DISM недоступен
		directPlayEnabled = false; // или специальный флаг «unknown»
		return;
	}
	directPlayEnabled = status;
}

// C:\Games\AnomalyLauncher - возвращает путь к папке лаунчера
std::string GetBaseDir() {
	return GetExePath(); 
}


std::string GetDownloadDir() {
	std::string base = GetBaseDir();
	std::string dir = base + "\\downloads";
	std::error_code ec;
	if (!std::filesystem::exists(dir)) {
		std::filesystem::create_directories(dir, ec);
		if (ec) {
			// Если не смогли создать downloads — fallback на base
			Log("[Init] Не удалось создать downloads, используем базовую папку: %s", ec.message().c_str());
			return base;
		}
	}
	return dir;
}


//Очистка от мусора
int CleanupPartFiles(const std::string& dir) {
	fs::path baseDir = fs::path(dir);
	if (!fs::exists(baseDir) || !fs::is_directory(baseDir)) {
		Log("[Cleanup] Каталог не найден: %s", dir.c_str());
		return -1;
	}

	int count = 0;
	try {
		for (const auto& entry : fs::directory_iterator(baseDir)) {
			if (!entry.is_regular_file()) continue;

			auto ext = entry.path().extension().string();
			if (ext == ".part") {
				std::error_code ec;
				fs::remove(entry.path(), ec);
				if (!ec) {
					++count;
					Log("[Cleanup] Удален .part: %s", entry.path().string().c_str());
				}
				else {
					Log("[Cleanup] Не удалось удалить: %s, причина: %s",
						entry.path().string().c_str(),
						ec.message().c_str());
				}
			}
		}
	}
	catch (const fs::filesystem_error& e) {
		Log("[Cleanup] Ошибка при сканировании каталога: %s", e.what());
		return -1;
	}
	catch (...) {
		Log("[Cleanup] Неожиданная ошибка при очистке мусора");
		return -1;
	}

	return count;
}