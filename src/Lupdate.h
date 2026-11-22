#pragma once

#define LAUNCHER_UPDATE_L_URL "https://raw.githubusercontent.com/0x5CE3C/AnomalyLauncher/refs/heads/updater/launcher_update.ini"
#define LAUNCHER_UPDATE_FILE "launcher_update.ini"
#define UPDATE_MODULES_LIST_URL "https://raw.githubusercontent.com/0x5CE3C/AnomalyLauncher/refs/heads/updater/files_list_update.ini"
#define UPDATE_MODPACK_CHANGELIST_URL "https://raw.githubusercontent.com/0x5CE3C/AnomalyLauncher/refs/heads/updater/modpack_changelist_update.ini"
#define MODPACK_UPDATE_CHANGELIST_FILE "modpack_changelist_update.ini"

void checklauncherupdate(); 
bool unlockedlauncherupdate(bool unlock = false);

void checkmodpacklatestchangelist();
bool unlockedmodpackupdate(bool unlock = false);

extern bool launcher_up_unlocked;
extern bool modpack_up_unlocked;

extern string launcher_latest_ver;
extern string launcher_latest_ver_link;
extern string launcher_last_changes_list;

extern string modpack_latest_ver;
extern string modpack_last_changes_list;

void filelistupdate();

void ProcessLauncherAutoUpdate();

void SetupLauncherUpdate();

void SetupModPackUpdate();

