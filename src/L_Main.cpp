#include "main.h"

stsn* sn = 0;

bool createlauncherfolder = true;

void sn_init()
{
	try
	{
		sn = new stsn();
		memset(sn, 0, sizeof(stsn*));

		registerAllCommands();

		// «ащита от битых значений
		if (sn->currentTab < 0 || sn->currentTab > static_cast<int>(TabId::CONSOLE)) {
			sn->currentTab = TabId::GENERAL;
		}

		if (createlauncherfolder)
		{
			CheckAnomalyLaucherFolder();
		}

		Load();

		InitLauncher();

		if (!HttpClient::getInstance()->init())
		{
			HttpClient::getInstance();			
		}

		Log("[ParentProcess] > %s", GetParentProcessName().c_str());

		DetectLaunchbyModOrganizer();

		if (fsm->FileExist("commandline.txt") == true)
		{
			rename("commandline.txt", "commandline.backup.txt");
		}


		if (fsm->FileExists("AnomalyLauncher_O.exe"))
		{
			remove("AnomalyLauncher_O.exe");
		}

		/*thpw->init();
		if (thpw->init() == true)
		{
			thpw->createThreadPool(1);
		}*/		


		try {
			executor->init();			

			checkmodpacklatestchangelist();

			ProcessLauncherAutoUpdate();

			if (sn->auto_update_filelist.enabled)
			{
				filelistupdate();
			}

			list_parse(GetExePath() + "\\" + PROJECT_FOLDER + "\\" + FILES_LIST);
		}
		catch (std::exception& e)
		{
			Log("[Error]TaskExecutor::init(); : %s", e.what());
		}

	}
	catch (const std::exception& e) { Log("[ sn_init(); ] Error: %s !", e.what()); }
}

void ExtractRes(WORD wResId, LPCSTR ResType, LPCSTR lpszOutputPath) {
	HRSRC hrsrc = FindResourceA(NULL, MAKEINTRESOURCE(wResId), ResType);
	if (!hrsrc) return; // об€зательно проверь, иначе UB

	HGLOBAL hLoaded = LoadResource(NULL, hrsrc);
	LPVOID lpLock = LockResource(hLoaded);
	DWORD dwSize = SizeofResource(NULL, hrsrc);

	HANDLE hFile = CreateFileA(lpszOutputPath, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		// тут можно залогировать ошибку GetLastError()
		FreeResource(hLoaded);
		return;
	}

	DWORD dwByteWritten;
	WriteFile(hFile, lpLock, dwSize, &dwByteWritten, NULL);
	CloseHandle(hFile);
	FreeResource(hLoaded);
}

std::vector<std::string> GetFileList(const char* dir, const char* pattern) {
	std::vector<std::string> fileList;
	HANDLE hFind;
	WIN32_FIND_DATAA findData;

	char findStr[MAX_PATH];
	snprintf(findStr, MAX_PATH, "%s\\%s", dir, pattern);

	hFind = FindFirstFileA(findStr, &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!strcmp(findData.cFileName, ".") || !strcmp(findData.cFileName, "..")) continue;
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

			fileList.emplace_back(findData.cFileName);

		} while (FindNextFileA(hFind, &findData));
		FindClose(hFind);
	}
	return fileList;
}

std::vector<std::string> GetAllDirsList(const char* dir, const char* pattern) {
	std::vector<std::string> fileList;
	HANDLE hFind;
	WIN32_FIND_DATAA findData;

	char findStr[MAX_PATH];
	snprintf(findStr, MAX_PATH, "%s\\%s", dir, pattern);

	hFind = FindFirstFileA(findStr, &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!strcmp(findData.cFileName, ".") || !strcmp(findData.cFileName, "..")) continue;
			/*if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{	continue;	}*/

			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				fileList.emplace_back(findData.cFileName);
			}//continue;			
		} while (FindNextFileA(hFind, &findData));
		FindClose(hFind);
	}
	return fileList;
}

BOOL IsAppRunningAsAdminMode()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is enabled in 

	// the primary access token of the process.

	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.

	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	// Throw the error if something failed in the function.

	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsRunAsAdmin;
}