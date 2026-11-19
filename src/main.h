#pragma once

#ifndef PROTECT
#define PROTECT


#pragma message( "[AnomalyLauncher]: Creating precompiled header...\n" )

#include "mainincludes.h"

#define PROJECT_FOLDER "AnomalyLauncher"
#define LVER "0.1.6.5 final"
#define AUTHOR " | ver -> "
#define PROJECT_NAME "AnomalyLauncher Extended" AUTHOR LVER
#define LOG_FILE "AnomalyLauncher.log"
#define INI_FILE "AnomalyLauncher.ini"

#define PROJECT_UPDATES_FOLDER "lupdates"

#define BE_MENU_ITEM_NAVMP3 "be_menu_main_item_nav.mp3"
#define BE_MENU_SELECTMP3 "be_menu_select.mp3"
#define BE_MENU_NAVMP3 "be_menu_nav.mp3"
#define BE_MENU_LOGMESSAGEMP3 "be_menu_log_message.mp3"
#define BE_MENU_MAINTHEME "be_theme.mp3"
#define BE_MENU_FILEDOWNLOADING "be_menu_file_downloading.mp3"

#include "necessarylibs.h"

#endif // !PROTECT
