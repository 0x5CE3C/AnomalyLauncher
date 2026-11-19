#pragma once

#define USERFILES_LIST "userfiles_list.txt"

struct stDownLoadList
{
	uint8_t id;
	const char* name;
	DLSubModuleTypes dl_type;
	const char* url;
};

struct stfdownloadlist
{
	stDownLoadList	downloadList[LIST_ENTRIES_MAX];
	//stDownLoadList* downloadListAssoc[65535][4];
};

struct stUserFileList
{
	uint8_t id;
	const char* name;
	const char* dir;		
	FileTypes f_type;
};

struct stUserfileslist
{
	stUserFileList		UserFileList[LIST_ENTRIES_MAX];
	stUserFileList*	UserFileListAssoc[65535][2];
};

///////////////////////////////////////////////
extern struct stUserfileslist userfileslist;
extern int	 iUserFilesListCount;

extern struct stfdownloadlist downloadlist;
extern int	 iDownLoadListCount;

void					userfileslist_load(void);
void					userfileslist_reload(void);
void					userfileslist_free(void);