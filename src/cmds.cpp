#include "main.h"



void registerAllCommands()
{	
	sn->con.AddCommand("cmds", "Список Команд", cmd_help);
	sn->con.AddCommand("q", "Выход", cmd_exit);
	sn->con.AddCommand("cls", "Очистить Консоль", cmd_cls);		
	sn->con.AddCommand("getexename", "[Отладка]Получить имя лаунчера", cmd_getexename);
	sn->con.AddCommand("getexenameext", "[Отладка]Получить имя лаунчера с расширением", cmd_getexenameext);
	sn->con.AddCommand("cleanjc", "Очистка журналов событий Windows, для остановки используй cleanjc 0", cmd_cleanjc);	
	sn->con.AddCommand("diag", "[Отладка]Диагностика проблем лаунчера", cmd_diag);
	sn->con.AddCommand("getbasedir", "[Отладка]Получить рабочий каталог", cmd_getbasedir);	
	sn->con.AddCommand("clearparts", "[Отладка]Поиск и очистка висячих .part файлов", cmd_clearparts);
	
};

Console::Console()
{
	memset(InputBuf, 0, sizeof(InputBuf));		
	
	HistoryPos = -1;	
	AutoScroll = true;
	ScrollToBottom = false;
}
