#include "main.h"

bool launcher_up_unlocked = false;
bool modpack_up_unlocked = false;

string launcher_latest_ver;
string launcher_latest_ver_link;
string launcher_last_changes_list;

string modpack_latest_ver;
string modpack_last_changes_list;

void launcherupdate_parse();

void checklauncherupdate() {
	string updateurl = LAUNCHER_UPDATE_L_URL;
	string updatetxt = GetExePath() + "\\" + PROJECT_FOLDER + "\\" + LAUNCHER_UPDATE_FILE;
	if (DetectLaunchbyModOrganizer() == true) { return; }
	executor->AddTask([updateurl, updatetxt]() {
		filedownload(updateurl, updatetxt);
		}, TaskType::FILE_DOWNLOAD);


	executor->AddTask([]() {
		launcherupdate_parse();
		}, TaskType::FILE_UPDATE);
};

void launcherupdate_parse()
{
	string updatetxt = GetExePath() + "\\" + PROJECT_FOLDER + "\\" + LAUNCHER_UPDATE_FILE;
	if (fsm->FileExists(updatetxt.c_str()))
	{
		INI::File ini;
		// Loading from file	
		if (!ini.Load(updatetxt))
		{
			// Loading from stream
			std::ifstream op(updatetxt, std::ios::in);
			if (op.is_open())
				op >> ini;
		}

		launcher_latest_ver = ini.GetSection("Launcher")->GetValue("latest_version").AsString();

		launcher_latest_ver_link = ini.GetSection("Launcher")->GetValue("latest_url").AsString();

		launcher_last_changes_list = ini.GetSection("Launcher")->GetValue("latest_changelist").AsString();

		if (strcmp(launcher_latest_ver.c_str(), LVER) == 0)
		{
			launcher_up_unlocked = 0;
			//sn->con.AddLog("?????????? ???????? ?? ?????????");
		}
		else
		{
			//sn->con.AddLog("???????? ?????????? ????????");
			launcher_up_unlocked = 1;
		}
		//sn->con.AddLog("latest: %s actual: %s", ver.c_str(), LVER);
	}
};

void filelistupdate()
{
	string update_filelist_url;

	if (sn->umode.universal_mode.enabled)
	{
		update_filelist_url = sn->umode.filelist_url;
	}
	else
	{
		update_filelist_url = UPDATE_MODULES_LIST_URL;
	}


	if (DetectLaunchbyModOrganizer() == true) { return; }
	executor->AddTask([update_filelist_url]() {
		filedownload(update_filelist_url, GetExePath() + "\\" + PROJECT_FOLDER + "\\" + FILES_LIST);
		}, TaskType::FILE_DOWNLOAD);

	executor->AddTask([]() {
		list_parse(GetExePath() + "\\" + PROJECT_FOLDER + "\\" + FILES_LIST);
		}, TaskType::FILE_UPDATE);
};

void modpack_changelist_parse();

void checkmodpacklatestchangelist()
{
	string update_changelist_url;

	if (sn->umode.universal_mode.enabled)
	{
		update_changelist_url = sn->umode.lastupdates_url;
	}
	else
	{
		update_changelist_url = UPDATE_MODPACK_CHANGELIST_URL;
	}

	string modpackchangelist = GetExePath() + "\\" + PROJECT_FOLDER + "\\" + MODPACK_UPDATE_CHANGELIST_FILE;

	if (DetectLaunchbyModOrganizer() == true) { return; }

	executor->AddTask([update_changelist_url, modpackchangelist]() {
		filedownload(update_changelist_url, modpackchangelist);
		}, TaskType::FILE_DOWNLOAD);

	executor->AddTask([]() {
		modpack_changelist_parse();
		}, TaskType::FILE_UPDATE);
};

void modpack_changelist_parse()
{
	string modpackchangelist = GetExePath() + "\\" + PROJECT_FOLDER + "\\" + MODPACK_UPDATE_CHANGELIST_FILE;
	if (fsm->FileExists(modpackchangelist.c_str()))
	{
		INI::File ini;
		// Loading from file	
		if (!ini.Load(modpackchangelist))
		{
			// Loading from stream
			std::ifstream op(modpackchangelist, std::ios::in);
			if (op.is_open())
				op >> ini;
		}

		modpack_latest_ver = ini.GetSection("modpack")->GetValue("modpack_latest_version").AsString();
		modpack_last_changes_list = ini.GetSection("modpack")->GetValue("modpack_latest_changelist").AsString();

		if (strcmp(modpack_latest_ver.c_str(), sn->set.modpack_version.c_str()) == 0)
		{
			modpack_up_unlocked = 0;
		}
		else
		{
			modpack_up_unlocked = 1;
		}
	}
}

void ProcessLauncherAutoUpdate()
{
	static bool auto_check_launcher_update_e = false;
	static bool checkupdate = true;
	if (sn->auto_check_launcher_update.enabled)
	{
		if (!auto_check_launcher_update_e)
		{
			sn->con.AddLog(u8"[info]: Auto Updater Enabled");
			auto_check_launcher_update_e = 1;
			if (checkupdate) { checklauncherupdate(); checkupdate = false; }
			if (unlockedlauncherupdate(launcher_up_unlocked))
			{
				SetupLauncherUpdate();
			};
		}
	}
	else
	{
		if (auto_check_launcher_update_e)
		{
			sn->con.AddLog(u8"[info]: Auto Updater Disabled");
			auto_check_launcher_update_e = 0;
		}
		if (checkupdate) { checklauncherupdate(); checkupdate = false; }
	}
};

void SetupLauncherUpdate()
{
	auto httpclient = HttpClient::getInstance();

	if (launcher_latest_ver_link.empty()) {
		sn->con.AddLog("Error: Invalid download link by Launcher Updater");
		Log("Error: Invalid download link by Launcher Updater");
		return;
	}
	else
	{
		executor->AddTask([]() {
			filedownload(launcher_latest_ver_link, "AnomalyLauncher_L.exe");
			}, TaskType::FILE_DOWNLOAD);

	}

	if (strcmp(launcher_latest_ver.c_str(), LVER) != 0)
	{
		while (true)
		{
			if (progress_ > 0.0f && httpclient->IsDownloading() && httpclient->GetDownloadFileName() == "AnomalyLauncher_L.exe")
			{
				sn->con.AddLog("launcher rename");
				continue;
			}
			else if (progress_ == 100.0f && !httpclient->IsDownloading() && httpclient->GetDownloadFileName() == "AnomalyLauncher_L.exe")
			{

				if (fsm->FileExists("AnomalyLauncher.exe"))
				{
					rename("AnomalyLauncher.exe", "AnomalyLauncher_O.exe");

					ShellExecuteA(0, "open", "AnomalyLauncher_L.exe", nullptr, nullptr, SW_SHOWNORMAL);

					rename("AnomalyLauncher_L.exe", "AnomalyLauncher.exe");

					ExitProcess(0);
				}
			}
		}
	}
}



void SetupModPackUpdate()
{
	Log("[Обновление] > Запуск обновления сборки");
	if (strcmp(modpack_latest_ver.c_str(), sn->set.modpack_version.c_str()) != 0)
	{
		if (sn->download_system.enabled == false) sn->download_system.enabled = true;
		if (sn->scanhashes.enabled == false) sn->scanhashes.enabled = true;
		if (sn->scanhashes_dirs.enabled == false) sn->scanhashes_dirs.enabled = true;

		filelistupdate();
		executor->AddTask([]() {}, TaskType::HASH_CHECK_MODS);
		executor->AddTask([]() {}, TaskType::SCANNING);
	}
}