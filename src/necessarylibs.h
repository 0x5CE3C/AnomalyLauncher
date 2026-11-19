#pragma once

// DirectX
#include <d3d9.h>
#include <d3d9types.h>

// Библиотеки сторонних разработчиков
#include "bass.h"
#pragma comment(lib, "bass.lib")

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DISABLE_DEMO_WINDOWS
#define IMGUI_DISABLE_DEBUG_TOOLS

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>
#include "imgui_toggle.h"
#include "ImColor.h"
#include "Fonts/itsmyfontforgocnet.h"
#include "Fonts/IconFontAwesome5.h"
#include "ImFuncs.h"

// CURL
#define CURL_STATICLIB
#include "curl/curl.h"
#include "Network/HttpClient.h"
#include "Network/HTTPWebPageScraper.h"

// my incls
#include "CExceptionHandler.hpp"
#include "ThreadWrapper.hpp"
void Log(const char* fmt, ...);
//#include "PDetector.hpp" // Профайлер
#include "ini_parser.h"
#include "listparser.hpp"
#include "config.h"
#include "gui.h"
#include "resource.h"
#include "math_stuff.h"
#include "md5.h"
#include "L_Main.h"
#include "Menu_Functions.h"
#include "misc_funcs.h"
#include "FileSystemManager.h"
#include "Systeminfo.h"
#include "FileDownloader.h"


void gen_random(char* s, const int len);

TCHAR szPath[MAX_PATH];
char* CurrentPath;

int getTaskBarHeight();
void traceLastFunc(const char* szFunc);

#include "L_funcs.h"
#include "LParams.h"

#include "Menu.h"
#include "lists.h"
#include "scanner.h"

#include "CTaskManager.h"

#include "Lupdate.h"

#include "ZipExtractor.hpp"

void my_free(void* ptr);
_Check_return_
_ACRTIMP _CRTALLOCATOR char* __cdecl my_strdup(_In_opt_z_ char const* s);

#include "Menu_ThemeEditor.h"
#include "L_ModPack_Helper.h"


#include <shlwapi.h> // нужен для PathRemoveFileSpec
#pragma comment(lib, "Shlwapi.lib")

//Filelist Editor/Generator
#include "LauncherConfig.h"
#include "ConfigManager.h"
#include "FileListGenerator.h"
#include "RenderConfigEditor.h"

//Запаковщик в zip
#include "ZipPacker.h"