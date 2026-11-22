#include "main.h"
#include "UrlImporter.h"
#include "imgui_stdlib.h"
using namespace ImFuncs;
using namespace ImGui;


std::string GetConfigPath()
{
	char szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);

	char* lastSlash = strrchr(szFilePath, '\\');
	if (lastSlash) {
		*lastSlash = 0; // Обрезаем имя exe, остаётся только путь
	}

	return std::string(szFilePath);
}

void Menu_render_lists() 
{	
	RenderFilesList();

	pMenu->AddMenuText(u8" Генераторы списков", false, ImColorPZBase, true);
	if (pMenu->AddStaticButton(u8"Генератор конфигурации (files_list.ini)"))
	{
		ToggleConfigEditor();
	};

	RenderConfigEditor();

	if (pMenu->AddStaticButton(u8"Обновить URL из файла")) {
		// Создаём экземпляр импортера
		UrlImporter importer;

		// 1. Получаем путь к папке, где лежит exe
		std::string basePath = GetExePath();

		// Путь, куда сохранить обновлённый listini.
		// Лучше сохранять туда же, откуда ты читаешь конфиг (например, рядом с exe или в папке Data)
		std::string savePathStr = basePath + "\\AnomalyLauncher\\files_list.ini";

		// Запускаем импорт: откроет диалог выбора файла, обновит filelist.fileList[], сохранит на диск
		bool success = importer.importAndSave(savePathStr.c_str());

		if (success) {
			ImGui::OpenPopup("Popup_Success");
		}
		else {
			ImGui::OpenPopup("Popup_Error");
		}
	}

	// Простые попапы для обратной связи (чтобы юзер видел результат)
	if (ImGui::BeginPopup("Popup_Success")) {
		ImGui::Text(u8"URL успешно обновлены!");
		ImGui::Separator();
		if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Popup_Error")) {
		ImGui::Text(u8"Не удалось обновить URL (проверь файл со ссылками).");
		ImGui::Separator();
		if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	

	//pMenu->AddMenuText(u8" Список патчей и внутренних задач:", true, ImColorOrange, false);	
	//SameLine();
	//ImFuncs::HelpDescr(u8"В процессе доработки...");
	//ImFuncs::HelpDescr(u8"Выполняет необходимые файловые операции(перемещение, удаление, переименование) до или после установки патчей для правильной работы всех модулей");

	/// 
	//pMenu->AddMenuText(u8" Список Конфигураций Игрока:", true, ImColorOrange, false);	
	//SameLine();
	//ImFuncs::HelpDescr(u8"В процессе доработке...");
	//HelpDescr(u8"Ваши настройки Аномали и модов, которые можно добавить в zip-архив и загрузить на Яндекс Диск, а потом в любой момент скачать, он распакуется автоматически в указанную вами папку. [ Рекомендуется восстанавливать когда Мод Органайзер выключен ]");
	

	pMenu->AddMenuText(u8" Универсальный режим", false, ImColorPZBase, true);	

	float anim_speed = 0.2f;
	ImGuiToggleFlags toggle_flags = ImGuiToggleFlags_Animated;

	ImGui::Toggle(u8"Включить Универсальный режим", &sn->umode.universal_mode.enabled, toggle_flags, anim_speed);
	FastDescr(u8"{AAFFFF}Используется для создания кастомных сборок отличных от [Purge of Zone]");

	if (sn->umode.universal_mode.enabled)
	{		
		ImGui::PushItemWidth(gui::size.x / 2);
		ImGui::InputTextWithHint(u8"Имя профиля сборки в Mod Organizer 2", u8"введите название", &sn->umode.modpack_name);
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(gui::size.x / 2);
		ImGui::InputTextWithHint(u8"ссылка на список файлов", u8"введите ссылку", &sn->umode.filelist_url);
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(gui::size.x / 2);
		ImGui::InputTextWithHint(u8"ссылка на список изменений в сборке", u8"введите ссылку", &sn->umode.lastupdates_url);
		ImGui::PopItemWidth();
	}

};

void RenderFilesList()
{
	static bool filelist_open = false;
	if (pMenu->AddStaticButton(u8"Список файлов"))
	{
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowSize().x - 6, ImGui::GetWindowSize().y - 6));
		filelist_open = true;
	};
	/*if (lock_modules_list)
	{
		sn->con.AddLog(u8"[Сканер] Modules List разблокирован!");
		lock_modules_list = 0;
	}*/

	//}
	ImGuiWindowFlags window_flags_rc = 0;
//	window_flags_rc |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags_rc |= ImGuiWindowFlags_NoCollapse;	
	window_flags_rc |= ImGuiWindowFlags_NoTitleBar;
	//window_flags_rc |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
	/*window_flags_rc |= ImGuiWindowFlags_AlwaysVerticalScrollbar;*/

	if(filelist_open)
	{		
		std::string filelistname = u8"Список файлов";
		if (ImGui::Begin(filelistname.c_str(), &filelist_open, window_flags_rc))
		{			
			RenderImage(gui::device);

			int width = strlen(filelistname.c_str()) * 4;
			ImGui::SameLine(ImGui::GetWindowWidth() / 2 - width);
			ImGui::PushFont(pFont);
			
			ImGui::Text(filelistname.c_str());

			ImGui::SameLine(ImGui::GetWindowWidth() - 30);

			if (ImGui::Button("--"))
			{
				ShowWindow(GetActiveWindow(), SW_SHOWMINIMIZED);
			}
			ImFuncs::FastDescr(u8"{FF0008}свернуть"); // R G B A
			ImGui::SameLine();
			if (ImGui::Button("X"))
			{
				filelist_open = false;
			}
			ImFuncs::FastDescr(u8"{FF0009}закрыть"); // R G B A                
			ImGui::PopFont();

			if (ImGui::Button(u8"Обновить лист"))
			{
				list_parse(GetExePath() + "\\" + PROJECT_FOLDER + "\\" + FILES_LIST);
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"Скачать новый лист"))
			{
				filelistupdate();
			}
		
			ImGuiTextFilter       Filter;
			Filter.Draw(u8"Поиск по имени файла и каталогам", ImGui::GetWindowWidth() / 2);
			ImGui::SameLine();
			ImFuncs::HelpDescr(u8"поиск по каталогам происходит только для типа файла Directory");																	

			ImGuiOldColumnFlags c_flags = ImGuiOldColumnFlags_NoResize;

			BeginColumns("filelist_h", 9, c_flags);
			ImGui::TextWrapped(u8"номер");
			ImGui::NextColumn();
			ImGui::TextWrapped(u8"имя файла");
			ImGui::NextColumn();
			ImGui::TextWrapped(u8"каталог");
			ImGui::NextColumn();
			ImGui::TextWrapped(u8"тип модуля");
			ImGui::NextColumn();
			ImGui::TextWrapped(u8"тип файла");
			ImGui::NextColumn();
			ImGui::TextWrapped(u8"хеш");
			ImGui::NextColumn();
			ImGui::TextWrapped(u8"имя загружаемого файла");
			ImGui::NextColumn();
			ImGui::TextWrapped(u8"тип загружаемого файла");
			ImGui::NextColumn();
			ImGui::TextWrapped(u8"ссылка");
			ImGui::NextColumn();
			Separator();
			EndColumns();

			Separator();
			BeginGroup();
			{
				if(BeginChild("filelistcols"))
				{
					BeginColumns("filelist", 9, c_flags);
					/*ImGui::TextWrapped(u8"номер");
					ImGui::NextColumn();
					ImGui::TextWrapped(u8"имя файла");
					ImGui::NextColumn();
					ImGui::TextWrapped(u8"каталог");
					ImGui::NextColumn();
					ImGui::TextWrapped(u8"тип модуля");
					ImGui::NextColumn();
					ImGui::TextWrapped(u8"тип файла");
					ImGui::NextColumn();
					ImGui::TextWrapped(u8"хеш");
					ImGui::NextColumn();
					ImGui::TextWrapped(u8"имя загружаемого файла");
					ImGui::NextColumn();
					ImGui::TextWrapped(u8"тип загружаемого файла");
					ImGui::NextColumn();
					ImGui::TextWrapped(u8"ссылка");
					ImGui::NextColumn();
					Separator();*/


					for (int i = 0; i < iFileListCount; i++)
					{
						const char* item = filelist.fileList[i].name;
						if (!Filter.PassFilter(item))
							continue;	
				
						if (filelist.fileList[i].m_type == SUBMODULE_TYPE_EXTRACTOR)
						{						
							if (filelist.fileList[i].f_type == FILE_TYPE_FILE)
							{							
								TextWrapped("%i", filelist.fileList[i].id);							
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].name).c_str());							
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].dir).c_str());							
								NextColumn();
								TextWrapped(u8"Extractor");							
								NextColumn();
								TextWrapped(u8"File");
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].hash);
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].downloadfilename);
								NextColumn();
								TextWrapped(u8"%s", (filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_ZIP ? u8"zip-архив" : filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_EXE ? u8"приложение" : u8"неизвестно"));
								NextColumn();
								TextWrapped(u8"%s", (strcmp(filelist.fileList[i].url, "") == 0 ? u8"-" : u8"+"));
								NextColumn();
								Separator();
							}
						}

						if (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY)
						{
							if (filelist.fileList[i].f_type == FILE_TYPE_DIR)
							{
								TextWrapped("%i", filelist.fileList[i].id);
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].name).c_str());
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].dir).c_str());
								NextColumn();
								TextWrapped(u8"Anomaly");
								NextColumn();
								TextWrapped(u8"Directory");
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].hash);
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].downloadfilename);
								NextColumn();
								TextWrapped(u8"%s", (filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_ZIP ? u8"zip-архив" : u8"неизвестно") );
								NextColumn();
								TextWrapped(u8"%s", (strcmp(filelist.fileList[i].url, "") == 0 ? u8"-" : u8"+"));
								NextColumn();
								Separator();
							}
							if (filelist.fileList[i].f_type == FILE_TYPE_FILE)
							{
								TextWrapped("%i", filelist.fileList[i].id);
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].name).c_str());
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].dir).c_str());
								NextColumn();
								TextWrapped(u8"Anomaly");
								NextColumn();
								TextWrapped(u8"File");
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].hash);
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].downloadfilename);
								NextColumn();
								TextWrapped(u8"%s", (filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_ZIP ? u8"zip-архив" : filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_EXE ? u8"приложение" : u8"неизвестно"));
								NextColumn();
								TextWrapped(u8"%s", (strcmp(filelist.fileList[i].url,"") == 0 ? u8"-" : u8"+"));
								NextColumn();
								Separator();
							}					
						}

						if (filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2)
						{
							if (filelist.fileList[i].f_type == FILE_TYPE_DIR)
							{
								TextWrapped("%i", filelist.fileList[i].id);
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].name).c_str());
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].dir).c_str());
								NextColumn();
								TextWrapped(u8"MO2");
								NextColumn();
								TextWrapped(u8"Directory");
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].hash);
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].downloadfilename);
								NextColumn();
								TextWrapped(u8"%s", (filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_ZIP ? u8"zip-архив" : u8"неизвестно"));
								NextColumn();
								TextWrapped(u8"%s", (strcmp(filelist.fileList[i].url, "") == 0 ? u8"-" : u8"+"));
								NextColumn();
								Separator();
							}
							if (filelist.fileList[i].f_type == FILE_TYPE_FILE)
							{
								TextWrapped("%i", filelist.fileList[i].id);
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].name).c_str());
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].dir).c_str());
								NextColumn();
								TextWrapped(u8"MO2");
								NextColumn();
								TextWrapped(u8"File");
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].hash);
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].downloadfilename);
								NextColumn();
								TextWrapped(u8"%s", (filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_ZIP ? u8"zip-архив" : filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_EXE ? u8"приложение" : u8"неизвестно"));
								NextColumn();
								TextWrapped(u8"%s", (strcmp(filelist.fileList[i].url, "") == 0 ? u8"-" : u8"+"));
								NextColumn();
								Separator();
							}
						}

						if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE)
						{
							if (filelist.fileList[i].f_type == FILE_TYPE_DIR)
							{
								TextWrapped("%i", filelist.fileList[i].id);
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].name).c_str());
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].dir).c_str());
								NextColumn();
								TextWrapped(u8"%s", (sn->umode.universal_mode.enabled == true ? cp1251_to_utf8(sn->umode.modpack_name).c_str() : u8"Purge of Zone"));
								NextColumn();
								TextWrapped(u8"Directory");
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].hash);
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].downloadfilename);
								NextColumn();
								TextWrapped(u8"%s", (filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_ZIP ? u8"zip-архив" : u8"неизвестно"));
								NextColumn();
								TextWrapped(u8"%s", (strcmp(filelist.fileList[i].url, "") == 0 ? u8"-" : u8"+"));
								NextColumn();
								Separator();
							}
							if (filelist.fileList[i].f_type == FILE_TYPE_FILE)
							{
								TextWrapped("%i", filelist.fileList[i].id);
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].name).c_str());
								NextColumn();
								TextWrapped(u8"%s", cp1251_to_utf8(filelist.fileList[i].dir).c_str());
								NextColumn();
								TextWrapped(u8"%s", (sn->umode.universal_mode.enabled == true ? cp1251_to_utf8(sn->umode.modpack_name).c_str() : u8"Purge of Zone"));
								NextColumn();
								TextWrapped(u8"File");
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].hash);
								NextColumn();
								TextWrapped(u8"%s", filelist.fileList[i].downloadfilename);
								NextColumn();
								TextWrapped(u8"%s", (filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_ZIP ? u8"zip-архив" : filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_EXE ? u8"приложение" : u8"неизвестно"));
								NextColumn();
								TextWrapped(u8"%s", (strcmp(filelist.fileList[i].url, "") == 0 ? u8"-" : u8"+"));
								NextColumn();
								Separator();
							}
						}
					}
					EndColumns();

					ImGui::EndChild();
				}	

				EndGroup();
			}

			ImGui::End();
		}
	}
};