#include "main.h"

#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

stCommand::stCommand(const char* pname, const char* pdescription, void(*pcallback)(const char* args))
{
	name = pname;
	description = pdescription;
	callback = pcallback;
}

Console::~Console()
{
	ClearLog();
	/*for (int i = 0; i < History.Size; i++)
		free(History[i]);*/
}

void Console::ClearLog()
{
	for (int i = 0; i < Items.Size; i++)
		free(Items[i]);
	Items.clear();
}
HCHANNEL menu_log_message;
static HSAMPLE menu_log_sample;
char	filename[512];
void Console::AddLog(const char* fmt, ...) IM_FMTARGS(2)
{	
	//PERF_BEGIN("void Console::AddLog(const char* fmt, ...) IM_FMTARGS(2)");

	traceLastFunc("void Console::AddLog(const char* fmt, ...) IM_FMTARGS(2)");
	// FIXME-OPT
	char buf[0x1000];
	static int c = 0;
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
	buf[IM_ARRAYSIZE(buf) - 1] = 0;
	va_end(args);
	Items.push_back(Strdup(cp1251_to_utf8(buf).c_str()));

	static bool bmenu_log_message_a = false;
	if (sn->bmenu_log_message.enabled)
	{
		//BASS_Init(-1, 44100, BASS_DEVICE_CPSPEAKERS, GetActiveWindow(), NULL);
		BASS_Init(-1, 44100, 0, 0, NULL);
		if (!bmenu_log_message_a)
		{
			//sn->con.AddLog("[info]: menu log message Enabled");
			bmenu_log_message_a = 1;
		}

		_snprintf(filename, sizeof(filename), "%s\\%s\\%s", GetExePath().c_str(), PROJECT_FOLDER, BE_MENU_LOGMESSAGEMP3);
		menu_log_sample = BASS_SampleLoad(false, filename, 0, 0, 1, NULL);
		menu_log_message = BASS_SampleGetChannel(menu_log_sample, FALSE);
		BASS_ChannelPlay(menu_log_message, FALSE);
		QWORD len = BASS_ChannelGetLength(menu_log_message, BASS_POS_BYTE);
		DWORD time = BASS_ChannelBytes2Seconds(menu_log_message, len);

		if (BASS_ChannelIsActive(menu_log_message) == BASS_ACTIVE_PLAYING)
		{
			BASS_ChannelSetAttribute(menu_log_message, BASS_ATTRIB_VOL, (float)sn->menu_log_message_volume / 100);
		}
	}
	else
	{
		if (bmenu_log_message_a)
		{
			//sn->con.AddLog("[info]:  menu log message Disabled");			
			bmenu_log_message_a = 0;
		}

		if (bmenu_log_message_a == 0)
		{
			if (BASS_ChannelIsActive(menu_log_message) == BASS_ACTIVE_PLAYING)
			{
				BASS_SampleFree(menu_log_sample);
				BASS_ChannelPause(menu_log_message);

				//sn->con.AddLog("BASS_ChannelStop");
			}
		}
	}
	//PlaySound(MAKEINTRESOURCE(IDR_WAVE2),\
		NULL, SND_RESOURCE | SND_FILENAME | SND_ASYNC);	

	//PERF_END();
	//PERF_STATS("void Console::AddLog(const char* fmt, ...) IM_FMTARGS(2)",5);
}

void Console::Draw(const char* title)
{
	RECT	um;		// damn near killed um        

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	//Set the IMGUI ON TOP POSITION X : 0 Y : 0 of the actual screen resolution.
	//ImGui::SetNextWindowPos(ImVec2(0, 0));

	//Change the Y viewport size to 200 and let the X to take the full screen : 
 //   viewport->Size.y = ImGui::GetWindowSize().y; // / 2
   // ImGui::SetNextWindowSize(viewport->Size);
	//ImGui::SetNextWindowSize(ImVec2(GetWindowSize() / 2), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y / 2), ImGuiCond_FirstUseEver);

	static bool no_titlebar = false;
	static bool no_scrollbar = false;
	static bool no_menu = true;
	static bool no_move = false;
	static bool no_resize = false;
	static bool no_collapse = false;
	static bool no_nav = false;
	static bool autosize = false;
	static bool no_save = false;

	ImGuiWindowFlags wincon_flags = 0;
	if (no_titlebar)  wincon_flags |= ImGuiWindowFlags_NoTitleBar;
	if (no_scrollbar) wincon_flags |= ImGuiWindowFlags_NoScrollbar;
	if (!no_menu)     wincon_flags |= ImGuiWindowFlags_MenuBar;
	if (no_move)      wincon_flags |= ImGuiWindowFlags_NoMove;
	if (no_resize)    wincon_flags |= ImGuiWindowFlags_NoResize;
	if (no_collapse)  wincon_flags |= ImGuiWindowFlags_NoCollapse;
	if (no_nav)       wincon_flags |= ImGuiWindowFlags_NoNav;
	if (autosize)     wincon_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	if (no_save)      wincon_flags |= ImGuiWindowFlags_NoSavedSettings;

	if (!ImGui::BeginChild(title))
	{
		ImGui::EndChild();
		return;
	}

	Filter.Draw("##Filter");
	ImGui::SameLine();
	if (ImGui::Button("X"))
	{
		Filter.Clear();
	};
	ImFuncs::FastDescr(u8"{FF0000}Очистить фильтр");
	ImGui::SameLine();
	if (ImGui::Button(u8"Очистить консоль"))
	{
		sn->con.ClearLog();
		sn->con.AddLog("{FF0000}Консоль Очищена!");
	};
	//ImGui::BeginChild("###123");
	//ImGui::EndChild();
	ImGui::Separator();

	RenderProgressBar();

	ImGui::Separator();

	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

	// Переменная для хранения строки, по которой кликнули
	static int s_clicked_index = -1;


	// === Цикл: рисуем строки ===
	for (int i = 0; i < Items.Size; i++)
	{		
		const char* item = Items[i];

		if (!Filter.PassFilter(Items[i])) continue;

		ImGui::TextWithColors(item);

		// Если ПКМ по строке — запоминаем индекс и открываем меню
		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		{
			s_clicked_index = i;
			ImGui::OpenPopup("console_context_menu");
		}
	}

	// === Один раз: рисуем контекстное меню ===
	if (ImGui::BeginPopup("console_context_menu"))
	{
		const char* item = (s_clicked_index >= 0 && s_clicked_index < Items.Size)
			? Items[s_clicked_index] : "";

		if (ImGui::Selectable(u8"Копировать строку"))
		{
			if (s_clicked_index >= 0)
				ImGui::SetClipboardText(item);
		}

		if (ImGui::Selectable(u8"Копировать всё"))
		{
			std::string full;
			for (int j = 0; j < Items.Size; j++)
			{
				full += Items[j];
				if (j < Items.Size - 1) full += "\n";
			}
			ImGui::SetClipboardText(full.c_str());
		}

		ImGui::EndPopup();
	}

	if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
		ImGui::SetScrollHereY(1.0f);
	ScrollToBottom = false;

	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::Separator();
	bool reclaim_focus = false;
	ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
	if (ImGui::InputTextWithHint(u8"#Ввод", u8"Чтобы узнать все команды введи: /cmds", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
	{
		char* s = InputBuf;
		Strtrim(s);
		if (s[0] == '/') {
			ExecCommand(s);
		}		 
		memset(s, 0, sizeof(*s));
		reclaim_focus = true;
	}
	
	ImGui::SetItemDefaultFocus();
	/*if (ImGui::IsItemHovered() || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))*/
	if (reclaim_focus)
	ImGui::SetKeyboardFocusHere(-1);	

}

int Console::TextEditCallback(ImGuiInputTextCallbackData* data)
{
	//traceLastFunc("int Console::TextEditCallback(ImGuiInputTextCallbackData* data)");
	//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
	switch (data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackCompletion:
	{
		// if (data->Buf[0] != '/') break;

		// Locate beginning of current word
		const char* word_end = (data->Buf + data->CursorPos);
		const char* word_start = word_end;
		while (word_start > data->Buf + 1)
		{
			const char c = word_start[-1];
			if (c == ' ' || c == '\t' || c == ',' || c == ';')
				break;
			word_start--;
		}

		// Build a list of candidates
		ImVector<const stCommand*> candidates;
		for (int i = 0; i < Commands.Size; i++)
			if (Strnicmp(Commands[i]->name, word_start, (int)(word_end - word_start)) == 0)
				candidates.push_back(Commands[i]);

		if (candidates.Size == 0)
		{
			// No match
			AddLog("Не найдено совпадений для \"%.*s\"!\n", (int)(word_end - word_start), word_start);
		}
		else if (candidates.Size == 1)
		{
			// Single match
			data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
			data->InsertChars(data->CursorPos, candidates[0]->name);
			data->InsertChars(data->CursorPos, " ");
		}
		else
		{
			// Multiple matches
			int match_len = (int)(word_end - word_start);
			for (;;)
			{
				int c = 0;
				bool all_candidates_matches = true;
				for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
					if (i == 0)
						c = toupper(candidates[i]->name[match_len]);
					else if (c == 0 || c != toupper(candidates[i]->name[match_len]))
						all_candidates_matches = false;
				if (!all_candidates_matches)
					break;
				match_len++;
			}

			if (match_len > 0)
			{
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0]->name, candidates[0]->name + match_len);
			}

			// List matches with descriptions
			AddLog("Возможные совпадения:");
			for (int i = 0; i < candidates.Size; i++)
				AddLog("--> %s - %s", candidates[i]->name, candidates[i]->description);
		}
		break;
	}
	case ImGuiInputTextFlags_CallbackHistory:
	{
		// Example of HISTORY
		const int prev_history_pos = HistoryPos;
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (HistoryPos == -1)
				HistoryPos = History.Size - 1;
			else if (HistoryPos > 0)
				HistoryPos--;
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (HistoryPos != -1)
				if (++HistoryPos >= History.Size)
					HistoryPos = -1;
		}

		// A better implementation would preserve the data on the current input line along with cursor position.
		if (prev_history_pos != HistoryPos)
		{
			const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
			data->DeleteChars(0, data->BufTextLen);
			data->InsertChars(0, history_str);
		}
	}
	}
	return 0;
}

void Console::AddCommand(const char* name, const char* description, void(*callback)(const char* args))
{
	Commands.push_back(new stCommand(name, description, callback));
	//AddLog("Зарегистрирована команда -> %s - %s", name, description);
}

void Console::AddHistory(const char* content)
{
	// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
	HistoryPos = -1;
	for (int i = History.Size - 1; i >= 0; i--)
		if (Stricmp(History[i], content) == 0)
		{
			free(History[i]);
			History.erase(History.begin() + i);
			break;
		}
	History.push_back(Strdup(content));
}

void Console::ExecCommand(const char* command_line)
{
	AddLog("--> {00DD00}%s", utf8_to_cp1251(command_line).c_str());

	AddHistory(command_line);

	//copy
	char buffer[256];
	memset(buffer, 0, 256); //Always remember to empty this char array, or there will be a argument problem.
	strcpy(buffer, &command_line[1]);

	//separate command and arguments
	int length = strlen(buffer);
	int cmdlen = 0;
	for (int i = 0; i < length; i++) {
		if (buffer[cmdlen] == ' ' || buffer[cmdlen] == '\0') break;
		cmdlen++;
	}
	const char* arguments = &buffer[cmdlen + 1];
	buffer[cmdlen] = 0x00;

	// Process command
	bool found = false;
	for (int i = 0; i < Commands.Size; i++) {
		if (Stricmp(buffer, Commands[i]->name) == 0) {
			Commands[i]->callback(arguments);
			found = true;
			break;
		}
	}
	if (!found)
		AddLog("{FFBB00}Неизвестная команда --> %s", utf8_to_cp1251(command_line).c_str());

	// On commad input, we scroll to bottom even if AutoScroll==false
	ScrollToBottom = true;
}