#include "main.h"
#include "ZipExtractor.hpp"

std::string zipPath = GetExePath() + "\\" + PROJECT_FOLDER + "\\" + "7za.exe";

bool is_extracting = false;

namespace zipextractor 
{
	void unzipfile(std::string filename, std::string unzip_path) {
		/*std::string zipPath = "D:\\Anomaly-1.5.3-Full.2\\AnomalyLauncher\\7za.exe";
		std::string command = "\"" + zipPath + "\" x \"" + filename + "\" -o\"" + unzip_path + "\" -y";*/
		std::string out_path = GetExePath() + "\\" + unzip_path;
		std::string command = "\"" + zipPath + "\" x \"" + filename + "\" -o\"" + out_path + "\\" + "\" -y";

		sn->con.AddLog("Command: %s", command.c_str());
		Log("Command: %s", command.c_str());
		// Создаем пайп для перенаправления вывода
		HANDLE hReadPipe, hWritePipe;
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;

		CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);
		SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

		PROCESS_INFORMATION pi;
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.hStdOutput = hWritePipe; // Перенаправление стандартного вывода
		si.hStdError = hWritePipe;  // Перенаправление стандартного вывода ошибок
		si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

		si.wShowWindow = SW_HIDE; // Скрываем окно

		// Создаем процесс
		if (!CreateProcess(NULL, const_cast<char*>(command.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
			Log("Ошибка создания процесса: %d", GetLastError());
			sn->con.AddLog("Ошибка создания процесса: %d", GetLastError());
			unpacked_status = 0; // Устанавливаем статус неудачи
			return;
		}

		CloseHandle(hWritePipe); // Закрываем пайп для записи

		// Считываем вывод
		char buffer[128];
		std::string output;
		DWORD bytesRead;
		while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
			buffer[bytesRead] = '\0'; // Добавляем терминальный символ
			output += buffer; // Сохраняем вывод
			//sn->con.AddLog("%s", buffer); // Логируем информацию
			//Log("%s", buffer); // Логируем информацию
		}

		// Ожидаем завершения процесса
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Проверяем код возврата процесса
		DWORD exitCode;
		GetExitCodeProcess(pi.hProcess, &exitCode);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hReadPipe);

		sn->con.AddLog("Вывод распаковки:\n%s", output.c_str());
		Log("Вывод распаковки:\n%s", output.c_str());

		if (exitCode == 0) {
			sn->con.AddLog("Архив успешно распакован!"); // Логируем информацию
			Log("Архив успешно распакован!");
			unpacked_status = 1; // Устанавливаем статус удачи		
		}
		else {
			sn->con.AddLog("Ошибка при распаковке архива."); // Логируем информацию
			Log("Ошибка при распаковке архива.");
			unpacked_status = 0; // Устанавливаем статус неудачи
		}

	}


	void ProcessWithDownloadedFile() {
		auto httpclient = HttpClient::getInstance();
		if (!httpclient->IsDownloading()) {
			for (int i = 0; i < iSubModuleListCount; i++) {				
				if (strcmp(httpclient->GetDownloadFileName().c_str(), submodulelist.submoduleList[i].name) == 0) {
					if (fsm->FileExist(submodulelist.submoduleList[i].name)) {
						HandleFileExtraction(i);
					}
				}
			}
		}
	}


	void HandleFileExtraction(int index) {
		if (fsm->FileExist(filelist.fileList[index].downloadfilename)) {

			if (iszip(filelist.fileList[index].downloadfilename) && unpacked_status == 0) {
				if (!zip_extracted && !is_extracting) {
					is_extracting = true;
					sn->con.AddLog(u8"[info]: Extracting Enabled");
					Log("[info]: Extracting Enabled");
					ProcessFileExtraction(index);
					zip_extracted = true;
					Log("zip_extracted status: %i", zip_extracted);
					is_extracting = false; // Сбрасываем флаг после завершения извлечения
				}
			}

			if (zip_extracted && unpacked_status == 1) {
				Log("zip_extracted %i, unpacked_status: %i", zip_extracted, unpacked_status);
				zip_extracted = false; // Если требуется сброс после успеха
				HandleDisabledExtraction(index);
			}			
		}
	}


	void ProcessFileExtraction(int index) {
		auto thw = ThreadWrapper::create();
		if (filelist.fileList[index].m_type == SUBMODULE_TYPE_ANOMALY || filelist.fileList[index].m_type == SUBMODULE_TYPE_MO2 || filelist.fileList[index].m_type == SUBMODULE_TYPE_BE)
		{
			if (strcmp(filelist.fileList[index].dir, "") == 0) {
				Log("filelist.fileList[%i].dir: %s", index, filelist.fileList[index].dir);
				thw->beginThreadex(unzipfile, 5, filelist.fileList[index].downloadfilename, filelist.fileList[index].dir);
			}
			else {
				thw->beginThreadex(unzipfile, 5, filelist.fileList[index].downloadfilename, filelist.fileList[index].dir);
			}
		}
	}

	void HandleDisabledExtraction(int index) {
		if (!zip_extracted && unpacked_status == 1) {
			sn->con.AddLog(u8"[info]: Extracting Disabled");
			Log("[info]: Extracting Disabled");
			unpacked_status = 0;
			Log("zip_extracted %i, unpacked_status: %i", zip_extracted, unpacked_status);

			//HandleModuleAnomaly(index);
			RemoveFileIfExists(index);
		}
	}



	void HandleModuleAnomaly(int index) {
		auto thw = ThreadWrapper::create();
		if (module_anomaly_damaged) {
			if (!scanner_anomaly_enable) {
				Log("Enable Scanner Anomaly");
				scanner_anomaly_enable = true;
				thw->beginAsync(ScanModule, SCAN_MODULE_TYPE_ANOMALY);
				thw->detachAll();				
			}
			// Закомментированный код для других типов
			/*
			if (!scanner_mo2_enable) {
				Log("Enable Scanner Anomaly");
				// thw->beginAsync(ScanModule, SCAN_MODULE_TYPE_MO2);
				// thw->detachAll();
			}
			if (!scanner_be_enable) {
				Log("Enable Scanner Anomaly");
				// thw->beginAsync(ScanModule, SCAN_MODULE_TYPE_BE);
				// thw->detachAll();
			}
			*/
		}
	}

	void RemoveFileIfExists(int index) {
		if (fsm->FileExist(filelist.fileList[index].downloadfilename)) {
			remove(filelist.fileList[index].downloadfilename);
			Log("remove: %s", filelist.fileList[index].downloadfilename);
		}
	}


}
