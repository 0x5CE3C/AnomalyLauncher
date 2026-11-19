#pragma once

/*
Структура системы скачивания и установки:
1) Сканирование файлов и каталогов (Докачка отсутствующих компонетнов)
2) Файловые операции и патч путей в ModOrganizer.ini (если игрок устанавливал MO2)
3) Скачивание и Установка последних патчей (если есть)
4) Загрузка и установка конфигов игрока (если нада)
*/

#define MODFILES_LIST "modules_list.txt"

enum SubModuleTypes
{
	SUBMODULE_TYPE_ANOMALY = 1,
	SUBMODULE_TYPE_MO2 = 2,
	SUBMODULE_TYPE_BE = 3,
};

enum TasksTypes
{
	TASK_FILE_MOVE = 1,	
	TASK_FILE_RENAME = 2,	
	TASK_FILE_REMOVE = 3,
	TASK_DIR_RENAME = 4,
	TASK_DIR_MOVE = 5,
	TASK_DIR_REMOVE = 6,
};

enum FileTypes
{
	FILE_TYPE_FILE = 1,
	FILE_TYPE_DIR = 2,
};

enum DLModuleTypes
{
	DL_MODULE_TYPE_ZIP = 1,
};

enum DLSubModuleTypes
{
	DL_SUB_MODULE_TYPE_ZIP = 1,
};

struct stSubModuleList
{
	uint8_t id;
	const char* name;
	SubModuleTypes m_type;
	DLSubModuleTypes dl_type;
	const char* url;
};

struct stFileList
{
	uint8_t id;
	const char* name;
	const char* dir;
	SubModuleTypes m_type;
	FileTypes f_type;
	const char* hash;
};

struct stTasksList
{
	uint8_t id;
	const char* name;
	const char* dir;
	FileTypes f_type;
	SubModuleTypes m_type;
	TasksTypes t_type;
	const char* pattern;
};

struct stPatchList
{
	uint8_t id;
	const char* name;
	const char* unpackdir;
	SubModuleTypes m_type;	
	const char* url;
};

struct stfsubmodulelist
{
	stSubModuleList	submoduleList[LIST_ENTRIES_MAX];
	stSubModuleList* submoduleListAssoc[65535][4];
};

struct stffilelist
{
	stFileList	fileList[LIST_ENTRIES_MAX];
	stFileList* fileListAssoc[65535][2];
};

struct stftasklist
{
	stTasksList	taskList[LIST_ENTRIES_MAX];
	stTasksList* taskListAssoc[65535][6];
};

struct stfpatchlist
{
	stPatchList	patchList[LIST_ENTRIES_MAX];
	stPatchList* patchListAssoc[65535][4];
};

///////////////////////////////////////////////

extern struct stfsubmodulelist submodulelist;
extern int	 iSubModuleListCount;

extern struct stffilelist filelist;
extern int	 iFileListCount;

extern struct stftasklist tasklist;
extern int	 iTaskListCount;

extern struct stfpatchlist patchlist;
extern int	 iPatchListCount;

void					modulelist_load(void);
void					modulelist_reload(void);
void					modulelist_free(void);