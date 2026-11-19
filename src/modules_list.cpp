#include "main.h"
#include "modules_list.h"



FILE* fd;


int iSubModuleListCount = 0;
int iFileListCount = 0;
int iPatchListCount = 0;
int iTaskListCount = 0;
struct stfsubmodulelist submodulelist;
struct stffilelist filelist;
struct stftasklist tasklist;
struct stfpatchlist patchlist;


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

void modulelist_free(void)
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
	
	if ((ent = list_register_entry("filelist", TYPE_FILE_LIST)) != NULL)
	{
		for (i = 0; i < LIST_ENTRIES_MAX; i++)
			list_register_data(ent, &filelist.fileList[i], "");
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
	case TYPE_FILE_LIST:
	{
		if (split->argc <= 1)
			break;

		if (split->argc != 6)
			goto wrong_argc;

		stFileList* fileList = (stFileList*)ent_data->data;

		int id = parse_int(split->argv[0]);
		if (id >= 65535)
		{
			Log("Invalid file id '%d' in %s", id, list_entry_name(ent, idx));
			break;
		}
		fileList->id = id;
		fileList->name = _strdup(split->argv[1]);
		fileList->dir = _strdup(split->argv[2]);

		if (_stricmp(split->argv[3], "anomaly") == 0)
			fileList->m_type = SUBMODULE_TYPE_ANOMALY;
		else if (_stricmp(split->argv[3], "mo2") == 0)
			fileList->m_type = SUBMODULE_TYPE_MO2;
		else if (_stricmp(split->argv[3], "be") == 0)
			fileList->m_type = SUBMODULE_TYPE_BE;
		else
		{
			Log("Invalid submodule type '%s' in %s", split->argv[3], list_entry_name(ent, idx));
			break;
		}

		if (_stricmp(split->argv[4], "file") == 0)
			fileList->f_type = FILE_TYPE_FILE;
		else if (_stricmp(split->argv[4], "dir") == 0)
			fileList->f_type = FILE_TYPE_DIR;		
		else
		{
			Log("Invalid file type '%s' in %s", split->argv[4], list_entry_name(ent, idx));
			break;
		}

		fileList->hash = _strdup(split->argv[5]);
		filelist.fileListAssoc[fileList->id][fileList->f_type] = fileList; // для большей производительности
		++iFileListCount;

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

void modulelist_load(void)
{
	static int	first_load = 1;
	char		filename[512];
	char		line[512], line_copy[4096], * opt, * str, * val;
	
	int			len, i, j, match;
	int			array_idx;

	/* init & load defaults */
	if (first_load)
	{
		memset(&submodulelist, 0, sizeof(submodulelist));
		memset(&filelist, 0, sizeof(filelist));
		memset(&tasklist, 0, sizeof(tasklist));
		memset(&patchlist, 0, sizeof(patchlist));		
			
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

	snprintf(filename, sizeof(filename), "%s", MODFILES_LIST);

	//Log("Loading %s...", filename);
	fd = fopen(filename, "r");
	if (fd == NULL)
	{
		Log("Could not open %s", filename);
		return;
	}

	strcpy(line_copy, "");
	while ((fgets(line, sizeof(line), fd) != NULL))
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
			
			if (!first_load && (ent->type == TYPE_SUBMODULE_LIST) && (ent->type == TYPE_FILE_LIST) && (ent->type == TYPE_TASKS_LIST) && (ent->type == TYPE_PATCH_LIST))
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

	fclose(fd);

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
				Log("Warning: setting '%s' was set multiple times in the list file", list_entry_name(ent, j));
			}

			isset = 1;
		}

		if (!isset)
			Log("Warning: setting '%s%s' was not present in the list file", ent->name, (ent->size > 1) ? "[]" : "");
	}

	//Log("Done.");
	first_load = 0;
}

void modulelist_reload(void)
{
	modulelist_load();
	Log("module_list reloaded.");
}
