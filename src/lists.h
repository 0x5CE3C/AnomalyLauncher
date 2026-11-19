#pragma once

#define MAX_ID 65535
//#define LIST_ENTRIES_MAX	1024

enum LIST_TYPE
{
	LIST_TYPE_FILELIST = 1,
	LIST_TYPE_SUBMODULELIST = 2,
	LIST_TYPE_PATCHLIST = 3,
	LIST_TYPE_TASKLIST = 4,	
};

typedef signed char			int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;

/////////////////////////////////////////////
//  MISC UTILS

struct str_split
{
	char* str;
	char** argv;
	int		argc;
};

void str_split_free(struct str_split* split);
struct str_split* str_split(const char* str, const char* ch);

size_t	 strlcat(char* dst, const char* src, size_t size);
size_t strlcpy(char* dst, const char* src, size_t size);
/////////////////////////////////////////////

/*
Структура системы скачивания и установки сборок игроками:
1) Сканирование файлов и каталогов (Докачка отсутствующих компонетнов)
2) Файловые операции и патч путей в ModOrganizer.ini (если игрок устанавливал MO2)
3) Скачивание и Установка последних патчей (если есть)
4) Загрузка и установка конфигов игрока (если нада)
*/

#define FILES_LIST "files_list.ini"

enum SubModuleTypes
{
	SUBMODULE_TYPE_UNKNOWN = 0,
	SUBMODULE_TYPE_ANOMALY = 1,
	SUBMODULE_TYPE_MO2 = 2,
	SUBMODULE_TYPE_BE = 3,
	SUBMODULE_TYPE_EXTRACTOR,
};

enum FileOperations
{
	FILEOP_FILE_MOVE = 1,
	FILEOP_FILE_RENAME = 2,
	FILEOP_FILE_REMOVE = 3,
	FILEOP_DIR_RENAME = 4,
	FILEOP_DIR_MOVE = 5,
	FILEOP_DIR_REMOVE = 6,
};

enum FileTypes
{
	FILE_TYPE_FILE = 1,
	FILE_TYPE_DIR = 2,
};

enum DLSubModuleTypes
{
	DL_SUB_MODULE_TYPE_ZIP = 1,
	DL_SUB_MODULE_TYPE_EXE = 2,
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
	uint16_t id;
	const char* name;
	const char* dir;
	SubModuleTypes m_type;
	FileTypes f_type;
	const char* hash;
	const char* downloadfilename;
	DLSubModuleTypes dl_type;
	const char* url;
};

struct stTasksList
{
	uint8_t id;
	const char* name;
	const char* dir;
	FileTypes f_type;
	SubModuleTypes m_type;
	FileOperations fop_type;
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

struct stfsubmodulelist { stSubModuleList submoduleList[MAX_ID]; stSubModuleList* submoduleListAssoc[MAX_ID][4]; };

struct stffilelist { stFileList fileList[MAX_ID]; stFileList* fileListAssoc[MAX_ID][4]; };

struct stftasklist { stTasksList taskList[MAX_ID]; stTasksList* taskListAssoc[MAX_ID][4]; };

struct stfpatchlist { stPatchList patchList[MAX_ID]; stPatchList* patchListAssoc[MAX_ID][4]; };


///////////////////////////////////////////////

extern struct stfsubmodulelist submodulelist;
extern int	 iSubModuleListCount;

extern struct stffilelist filelist;
extern int	 iFileListCount;

extern struct stftasklist tasklist;
extern int	 iTaskListCount;

extern struct stfpatchlist patchlist;
extern int	 iPatchListCount;


void list_parse(const std::string& filename);