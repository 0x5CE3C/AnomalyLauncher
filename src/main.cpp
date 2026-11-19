#include "main.h"
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdio>
#include <windows.h>
#include <memory>
#include <map>

FILE* g_flLog = NULL;

char g_szLastFunc[256];
LPTOP_LEVEL_EXCEPTION_FILTER OrigExceptionFilter;

// Структура для хранения информации о выделении памяти
struct AllocationInfo {
	size_t size;
	const char* functionName;
	int lineNumber;

	// Конструктор по умолчанию
	AllocationInfo() : size(0), functionName(nullptr), lineNumber(0) {}

	AllocationInfo(size_t sz, const char* func, int line)
		: size(sz), functionName(func), lineNumber(line) {
	}
};


// Структура для отслеживания выделенной памяти
// Ваша структура для отслеживания памяти
struct MemoryTracker {
	std::map<void*, AllocationInfo> allocations;

	void* Allocate(size_t size, const char* functionName, int lineNumber) {
		void* ptr = malloc(size);
		if (ptr) {
			allocations[ptr] = AllocationInfo(size, functionName, lineNumber);
		}
		return ptr;
	}

	void Free(void* ptr) {
		if (ptr) {
			allocations.erase(ptr);
			free(ptr);
		}
	}

	void LogLeaks() {
		if (!allocations.empty()) {
			Log("Обнаружена утечка памяти:");
			for (const auto& [ptr, info] : allocations) {
				Log("Leak: %p Size: %zu Function: %s Line: %d", ptr, info.size, info.functionName, info.lineNumber);
			}
		}
	}
};

MemoryTracker memoryTracker;


void traceLastFunc(const char* szFunc) {
    _snprintf_s(g_szLastFunc, sizeof(g_szLastFunc) - 1, "%s", szFunc);
}

LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo) {
	// Получаем текущее время
	std::time_t currentTime = std::time(nullptr);
	char* timeString = std::ctime(&currentTime);

	// Убираем символ новой строки из времени
	timeString[strlen(timeString) - 1] = '\0';

	// Логируем информацию о сбое
	Log("---------------------------------------------------------------------\n");
	Log(" %s has crashed.\n", PROJECT_NAME);
	Log(" Time of crash: %s\n", timeString);
	Log(" Last function hooked: %s\n", g_szLastFunc);
	// Логируем информацию о сбое
	Log("Exception code: 0x%08X\n", ExceptionInfo->ExceptionRecord->ExceptionCode);
	Log("Exception address: %p\n", ExceptionInfo->ExceptionRecord->ExceptionAddress);
	Log(" ---------------------------------------------------------------------");
	// Логируем утечки памяти
	memoryTracker.LogLeaks();
	Log(" ---------------------------------------------------------------------");

	if (OrigExceptionFilter) {
		return OrigExceptionFilter(ExceptionInfo);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

// Обертки для выделения и освобождения памяти
#define my_malloc(size) my_malloc_impl(size, __FUNCTION__, __LINE__)

void* my_malloc_impl(size_t size, const char* functionName, int lineNumber) {
	return memoryTracker.Allocate(size, functionName, lineNumber);
}

void my_free(void* ptr) {
	memoryTracker.Free(ptr);
}

// Обертка для strdup, использующая my_malloc
_Check_return_
_ACRTIMP _CRTALLOCATOR char* __cdecl my_strdup(_In_opt_z_ char const* s) {
	size_t len = strlen(s) + 1; // +1 для нулевого терминатора
	char* dup = static_cast<char*>(memoryTracker.Allocate(len, __FUNCTION__, __LINE__));
	if (dup) {
		memcpy(dup, s, len);
	}
	return dup;
}

int getTaskBarHeight() //учёт высоты панели задач
{
	RECT rect;
	HWND taskBar = FindWindowA("Shell_traywnd", NULL);
	if (taskBar && GetWindowRect(taskBar, &rect))
	{
		return rect.bottom - rect.top;
	}
}
POINTS m_Pos;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void OnApplicationClose() {
	if (g_SettingsDirty) {
		Save();
		sn->con.AddLog("Настройки сохранены при выходе.");
	}
}

int APIENTRY WinMain(
    HINSTANCE,
    HINSTANCE,
    LPSTR,
    int)
{
    
	OrigExceptionFilter = SetUnhandledExceptionFilter(unhandledExceptionFilter);

	try
	{	
		gui::CreateHWindow(PROJECT_NAME); // название вашего окна, заголовок
		gui::CreateDevice();
		gui::CreateImGui();

		while (gui::isRunning) // цикл рендера
		{
			gui::BeginRender();
			gui::Render();
			gui::EndRender();
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		//OnApplicationClose();

		// Логируем утечки памяти перед завершением		
		memoryTracker.LogLeaks();		

		gui::DestroyImGui(); // выход из программы
		gui::DestroyDevice();
		gui::DestroyHWindow();
	}
	catch (ExceptionHandler& e)
	{
		handleException(e);
	}
	catch (std::exception& e)
	{
		handleStandardException(e);
	}

    return EXIT_SUCCESS;
}

void Log(const char* fmt, ...)
{
	//if (!g_szWorkingDirectory) return;
	
	SYSTEMTIME	time;
	va_list		ap;

	if (g_flLog == NULL)
	{
		char	filename[512];
        _snprintf(filename, sizeof(filename), "%s\\%s", PROJECT_FOLDER, LOG_FILE);

		g_flLog = fopen(filename, "w+");

		if (g_flLog == NULL)
			return;
	}

	GetLocalTime(&time);
	fprintf(g_flLog, "[%02d:%02d:%02d.%03d] ", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
	va_start(ap, fmt);
	vfprintf(g_flLog, fmt, ap);		
	va_end(ap);
	fprintf(g_flLog, "\n");	
	fflush(g_flLog);
    //fclose(g_flLog);
}