#include "main.h"



FILE* fdd;

struct list_entry_data
{
	int			count;				/* number of times it has been set */
	void* data;				/* teh value */
	const char* def;				/* default */
};

struct list_entry
{
	const char* name;
	int						type;
	int						size;	/* number of data items (1 for non-arrays) */
	struct list_entry_data* data;	/* size # of list_entry_data */
};

int	 iUserFilesListCount = 0;
int	 iDownLoadListCount = 0;

struct stUserfileslist userfileslist;
struct stfdownloadlist downloadlist;


static struct list_entry* list_entry;
static int				list_entry_count;

static const char* list_entry_name(const struct list_entry* ent, int idx)
{
	static char str[64];

	if (ent->size == 1)
		return ent->name;

	if (idx == -1)
		snprintf(str, sizeof(str), "%s[]", ent->name);
	else
		snprintf(str, sizeof(str), "%s[%d]", ent->name, idx);

	return str;
}

static int parse_int(const char* str)
{
	return strtol(str, NULL, 0);
}

static struct list_entry_data* list_register_data(struct list_entry* ent, void* data, const char* def)
{
	struct list_entry_data* d;
	void* tmp;

	ent->size++;
	tmp = realloc(ent->data, ent->size * sizeof(struct list_entry_data));
	if (tmp == NULL)
		goto out;
	ent->data = (struct list_entry_data*)tmp;

	d = &ent->data[ent->size - 1];
	d->count = 0;
	d->data = data;
	d->def = _strdup(def);

	return d;
out:;
	ent->size--;
	return NULL;
}

static struct list_entry* list_register_entry(const char* name, int type)
{
	struct list_entry* ent = NULL;
	void* tmp;

	list_entry_count++;

	tmp = realloc(list_entry, list_entry_count * sizeof(struct list_entry));
	if (tmp == NULL)
		goto out;
	list_entry = (struct list_entry*)tmp;

	ent = &list_entry[list_entry_count - 1];

	ent->name = _strdup(name);
	ent->type = type;
	ent->size = 0;
	ent->data = NULL;

	return ent;

out:;
	list_entry_count--;
	return NULL;
}

void userfileslist_free(void)
{
	struct list_entry* ent;
	int					i, j, c;

	for (i = 0; i < list_entry_count; i++)
	{
		ent = &list_entry[i];

		if (ent->name != NULL)
			free((void*)ent->name);
	}

	free(list_entry);
	list_entry_count = 0;
}

static void list_init(void)
{
	struct list_entry* ent;
	static int	i;

	if ((ent = list_register_entry("dlist", TYPE_USER_DOWNLOAD_LIST)) != NULL)
	{
		for (i = 0; i < LIST_ENTRIES_MAX; i++)
			list_register_data(ent, &downloadlist.downloadList[i], "");
	}
	if ((ent = list_register_entry("flist", TYPE_USER_FILE_LIST)) != NULL)
	{
		for (i = 0; i < LIST_ENTRIES_MAX; i++)
			list_register_data(ent, &userfileslist.UserFileList[i], "");
	}
}


static void list_entry_parse_type(struct list_entry* ent, int idx, const char* val)
{
	struct list_entry_data* ent_data = ent->data + idx;
	struct str_split* split = str_split(val, "\t ");
	int						ok = 0, i;

	if (split == NULL)
	{
		Log("Out of memory");
		return;
	}

	if (split->argc <= 0)
	{
		Log("Missing value for %s", list_entry_name(ent, idx));
		return;
	}

	switch (ent->type)
	{
	case TYPE_INT:
	{
		if (split->argc != 1)
			goto wrong_argc;
		*(int*)ent_data->data = parse_int(split->argv[0]);
		ok = 1;
		break;
	}
	case TYPE_FLOAT:
	{
		if (split->argc != 1)
			goto wrong_argc;
		*(float*)ent_data->data = (float)atof(val);
		ok = 1;
		break;
	}
	case TYPE_STRING:
	{
		if (split->argc != 1)
			goto wrong_argc;
		strlcpy((char*)ent_data->data, split->argv[0], LIST_STRLEN_MAX);
		ok = 1;
		break;
	}
	case TYPE_USER_DOWNLOAD_LIST:
	{
		if (split->argc <= 1)
			break;

		if (split->argc != 4)
			goto wrong_argc;

		stDownLoadList* downloadList = (stDownLoadList*)ent_data->data;

		int id = parse_int(split->argv[0]);
		if (id >= 65535)
		{
			Log("Invalid downloadlist id '%d' in %s", id, list_entry_name(ent, idx));
			break;
		}
		downloadList->id = id;
		downloadList->name = _strdup(split->argv[1]);

		std::string fext = _strdup(split->argv[1]);
		if (fext.substr(fext.find_last_of(".") + 1) == "zip")
		{
			downloadList->dl_type = DL_SUB_MODULE_TYPE_ZIP;
		}
		else
		{
			Log("Invalid extension type '%s' in %s", split->argv[1], list_entry_name(ent, idx));
			break;
		}		

		downloadList->url = _strdup(split->argv[2]);
		//downloadlist.downloadListAssoc[downloadList->id][downloadList->dl_type] = downloadList; // для большей производительности
		++iDownLoadListCount;

		ok = 1;
		break;
	}
	case TYPE_USER_FILE_LIST:
	{
		if (split->argc <= 1)
			break;

		if (split->argc != 6)
			goto wrong_argc;

		stUserFileList* userfileList = (stUserFileList*)ent_data->data;

		int id = parse_int(split->argv[0]);
		if (id >= 65535)
		{
			Log("Invalid file id '%d' in %s", id, list_entry_name(ent, idx));
			break;
		}
		userfileList->id = id;
		userfileList->name = _strdup(split->argv[1]);
		userfileList->dir = _strdup(split->argv[2]);		

		if (_stricmp(split->argv[4], "file") == 0)
			userfileList->f_type = FILE_TYPE_FILE;
		else if (_stricmp(split->argv[4], "dir") == 0)
			userfileList->f_type = FILE_TYPE_DIR;
		else
		{
			Log("Invalid file type '%s' in %s", split->argv[4], list_entry_name(ent, idx));
			break;
		}
		
		userfileslist.UserFileListAssoc[userfileList->id][userfileList->f_type] = userfileList; // для большей производительности
		++iUserFilesListCount;

		ok = 1;
		break;
	}

	default:
		Log("BUG: %s has invalid data type", list_entry_name(ent, idx));
	}

	str_split_free(split);

	if (ok)
		ent_data->count++;

	return;

wrong_argc:;
	Log("Wrong number of arguments (%d) for %s", split->argc, list_entry_name(ent, idx));
	str_split_free(split);
}

static void list_entry_parse(struct list_entry* ent, int idx, const char* val)
{
	if (ent->size <= 0)
	{
		Log("BUG: %s has no data entries", list_entry_name(ent, idx));
		return;
	}

	if (ent->size > 1)
	{
		/* array */
		if (idx == -1)
		{
			/* foo[], find first unused data entry */
			for (idx = 0; idx < ent->size; idx++)
			{
				if (ent->data[idx].count == 0)
					break;
			}
		}

		if (idx >= ent->size)
			Log("Array index for %s out of bounds", list_entry_name(ent, idx));
		else
		{
			list_entry_parse_type(ent, idx, val);
		}
	}
	else
	{
		if (idx != -1)
			Log("Setting %s is not an array", list_entry_name(ent, idx));
		else
			list_entry_parse_type(ent, 0, val);
	}
}

void userfileslist_load(void)
{
	static int	first_load = 1;
	char		filename[512];
	char		line[512], line_copy[4096], * opt, * str, * val;

	int			len, i, j, match;
	int			array_idx;

	/* init & load defaults */
	if (first_load)
	{
		memset(&downloadlist, 0, sizeof(downloadlist));
		memset(&userfileslist, 0, sizeof(userfileslist));
		list_init();

		/* read defaults */
		for (i = 0; i < list_entry_count; i++)
		{
			for (j = 0; j < list_entry[i].size; j++)
			{
				list_entry_parse(&list_entry[i], -1, list_entry[i].data[j].def);
			}
		}
	}

	for (i = 0; i < list_entry_count; i++)
	{
		for (j = 0; j < list_entry[i].size; j++)
		{
			list_entry[i].data[j].count = 0;
		}
	}

	snprintf(filename, sizeof(filename), "%s", USERFILES_LIST);

	//Log("Loading %s...", filename);
	fdd = fopen(filename, "r");
	if (fdd == NULL)
	{
		Log("Could not open %s", filename);
		return;
	}

	strcpy(line_copy, "");
	while ((fgets(line, sizeof(line), fdd) != NULL))
	{
		len = (int)strlen(line);

		/* strip trailing newlines, spaces, and tabs */
		while (len > 0 && strchr("\t\n\r ", line[len - 1]))
			line[--len] = 0;

		/* skip comments and empty lines */
		if (line[0] == '#' || line[0] == ';' || line[0] == 0)
			continue;

		if (len > 0 && line[len - 1] == '\\')
		{
			line[len - 1] = ' ';
			strlcat(line_copy, line, sizeof(line_copy));
			continue;
		}

		strlcat(line_copy, line, sizeof(line_copy));

		str = line_copy;
		array_idx = -1;

		/* strip leading spaces */
		LTRIM(str);

		/* extract setting name
		 foo[123] = bar
		 ^^^ */
		while
			(
				(*str >= '0' && *str <= '9')
				|| (*str >= 'A' && *str <= 'Z')
				|| (*str >= 'a' && *str <= 'z')
				|| *str == '_'
				|| *str == '-'
				)
		{
			str++;
		}

		/* parse array index
		 foo[123] = bar
			^^^^ */
		if (*str == '[')
		{
			*str++ = 0;
			LTRIM(str);
			if (*str != ']')	/* foo[] uses the next free array index; leave array_idx set to -1 */
			{
				array_idx = parse_int(str);
				if (array_idx < 0)
				{
					Log("Parse error: %s (Negative array index)", line);
					goto parse_continue;
				}

				if ((str = strchr(str, ']')) == NULL)
				{
					Log("Parse error: %s (missing ']')", line);
					goto parse_continue;
				}
			}

			/* we're at the ']' here */
			str++;
		}

		/* find start of value
		 foo[123] = bar
				 ^^ */
		if (*str == '=')
		{
			*str = 0;
		}
		else
		{
			*str++ = 0;
			LTRIM(str);
			if (*str != '=')
			{
				Log("Parse error: %s (expected '=')", line);
				goto parse_continue;
			}
		}

		/* we're at the '=' here */
		opt = line_copy;
		val = str + 1;

		match = 0;

		for (i = 0; i < list_entry_count; i++)
		{
			struct list_entry* ent = &list_entry[i];

			if (strcmp(opt, ent->name) != 0)
				continue;

			/* don't reload patches (cause this would fuck things up...) */
			if (!first_load && (ent->type == TYPE_USER_DOWNLOAD_LIST) && (ent->type == TYPE_USER_FILE_LIST))
			{
				match = 1;
				break;
			}

			list_entry_parse(ent, array_idx, val);
			match = 1;

			break;
		}

		if (!match)
			Log("Unknown setting '%s'", opt);

	parse_continue:;
		strcpy(line_copy, "");
	}

	fclose(fdd);

	for (i = 0; i < list_entry_count; i++)
	{
		struct list_entry* ent = &list_entry[i];
		int					isset = 0;

		for (j = 0; j < ent->size; j++)
		{
			if (ent->data[j].count == 0)
				continue;

			if (ent->data[j].count > 1)
			{
				Log("Warning: setting '%s' was set multiple times in the .list file", list_entry_name(ent, j));
			}

			isset = 1;
		}

		if (!isset)
			Log("Warning: setting '%s%s' was not present in the .list file", ent->name, (ent->size > 1) ? "[]" : "");
	}

	//Log("Done.");
	first_load = 0;
}

void userfileslist_reload(void)
{
	userfileslist_load();
	Log("Settings reloaded.");
}
