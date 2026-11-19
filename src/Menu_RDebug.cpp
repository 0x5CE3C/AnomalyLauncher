#include "main.h"
using namespace ImGui;
using namespace ImFuncs;

void Menu_render_debug() 
{
	pMenu->AddButton(&sn->dbg.enabled, "Debug Mode");
	ImFuncs::FastDescr(u8"{FF0002}для обычного игрока это своеобразный повелитель зоны(чит-режим). Включается на кнопку F7 в самой игре.");
	ImGui::Separator();
	pMenu->AddButton(&sn->devdbg.enabled, "Developer Debug Mode");
	ImFuncs::FastDescr(u8"{FFFFFF}Поскольку обычные игроки стали использовать параметр -dbg, разработчики отделили дополнительные инструменты разработчика добавив параметр -dbgdev. По сути это тот же -dbg, но с расширенными функциями.\n{AAFFBB}Если ты обычный игрок и любишь играть нечестно - используй {AAFFFF}Debug Mode{AAFFBB}!!!");
	//pMenu->AddButton(&sn->debug_ge.enabled, "Debug GE");			
	//ImFuncs::FastDescr(u8"{FF0008} отладка уровня, музыки, снаряжения, спавна.");/// R G B A
	//pMenu->AddButton(&sn->demomode.enabled, "Demo Mode");			
	//ImFuncs::FastDescr(u8"{FF0008} демо-режим");/// R G B A
	ImGui::Separator();
	pMenu->AddButton(&sn->nes_texture_storing.enabled, "Debug Textures");
	ImFuncs::FastDescr(u8"{FF0008} история загрузки\\выгрузки текстур");/// R G B A	
	ImGui::Separator();	

	//if (pMenu->AddStaticButton(u8"Проверить хэши модов")) {
	//	executor->AddTask([]() {}, TaskType::HASH_CHECK_MODS);
	//}

	ImGui::SeparatorText(u8"ModPack_Helper");

	if (pMenu->AddStaticButton(u8"Поиск сторонних модов", gui::size.x / 2))
	{
		auto thw = ThreadWrapper::create();
		thw->beginAsync(Debug_ListExternalModsV2);
		//thw->detachAll();
	}
	ImGui::SameLine();
	if (pMenu->AddStaticButton(u8"Переместить сторонние моды в резерв"))
	{
		ImGui::OpenPopup(u8"Переместить сторонние модыв резерв(Подтверждение)");				
	}
	ImFuncs::FastDescr(u8"{FFFFFF}Сторонние моды не входящие в список файлов лаунчера будут перемещены из MO2\\mods в MO2\\backup\\external_mods с привязкой ко времени.");/// R G B A	

	if (pMenu->AddStaticButton(u8"Конвертация сторонних модов в список файлов", gui::size.x / 2))
	{
		auto thw = ThreadWrapper::create();
		thw->beginAsync(Debug_ConvertExternalModsToFilelist);
		thw->detachAll();
	}	

	ImGui::SameLine();
	if (pMenu->AddStaticButton(u8"Очистить сторонние моды"))
	{
		ImGui::OpenPopup(u8"Очистить сторонние моды(Подтверждение)");
	}

	ImGuiWindowFlags window_flags_about = 0;
	window_flags_about |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags_about |= ImGuiWindowFlags_NoCollapse;

	if (ImGui::BeginPopupModal(u8"Очистить сторонние моды(Подтверждение)", 0, window_flags_about))
	{		
		//	ImGui::SetWindowSize(ImVec2(293.f, 100.f));		
		std::string backupRoot = GetMO2Folder() + "\\backup\\external_mods";

		bool exist = false;

		if (fsm->FolderExist(backupRoot.c_str()) == true)
		{
			exist = true;					
		}
		else
		{
			exist = false;
		}

		if (exist) {
			ImGui::Text(u8"Вы действительно хотите очистить резервный каталог сторонних модов?");
			ImGui::Text(u8"Путь: %s", cp1251_to_utf8(backupRoot).c_str());
			ImGui::SameLine();
			if (ImGui::Button(u8"Открыть"))
			{
				ShellExecuteA(NULL, "explore", backupRoot.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
			if (pMenu->AddStaticButton(u8"Подтвердить", GetWindowSize().x / 2))
			{
				fsm->DeleteFolderWithFiles(backupRoot.c_str());
				ImGui::CloseCurrentPopup();
			}
			SameLine();
			if (pMenu->AddStaticButton(u8"Отменить"))
			{
				ImGui::CloseCurrentPopup();
			}
		}
		else
		{
			ImGui::Text(u8"Резервный каталог сторонних модов не обнаружен!!!");
			ImGui::Text(u8"Путь: N\\A");
			if (pMenu->AddStaticButton(u8"Закрыть"))
			{				
				ImGui::CloseCurrentPopup();
			}
		}

				
		ImGui::EndPopup();
	};


	if (ImGui::BeginPopupModal(u8"Переместить сторонние модыв резерв(Подтверждение)", 0, window_flags_about))
	{
		//	ImGui::SetWindowSize(ImVec2(293.f, 100.f));		
		std::string backupRoot = GetMO2Folder() + "\\backup\\external_mods";

		
			ImGui::Text(u8"Вы действительно хотите переместить сторонние моды в резерв?");
			ImGui::Text(u8"Путь: %s", cp1251_to_utf8(backupRoot).c_str());			

			if (pMenu->AddStaticButton(u8"Подтвердить", GetWindowSize().x / 2))
			{
				executor->AddTask([]() {}, TaskType::SCANNING_AND_BACKUP_EXTERNAL_MODS);
				ImGui::CloseCurrentPopup();
			}
			SameLine();
			if (pMenu->AddStaticButton(u8"Отменить"))
			{
				ImGui::CloseCurrentPopup();
			}		

		ImGui::EndPopup();
	};


	if (fsm->FileExist(zipPath.c_str()) == true)
	{
		ImGui::SeparatorText(u8"*.zip generator");


		// Кнопка «Архивировать Anomaly»
				
			if (IsRunningAnomalyPack()) {
				TextWithColors(u8"{AAFFFF} Выполняется Архивация Anomaly");
				SameLine();
				if (pMenu->AddStaticButton(u8"Отменить"))
				{
					if (executor->init())
					{
						std::vector<Task> tasks = executor->GetAllTasks(); // Используем глобальный экземпляр  

						// Остановка задач
						for (const Task& task : tasks) {
							if ((task.status == TaskStatus::PREPARING || task.status == TaskStatus::RUNNING) && task.type == TaskType::PACK_ANOMALY_ZIP)
							{
								executor->CancelTask(task.id);
							}
						}
						AbortAnomalyPack(); // если уже идёт — остановить
					}
				};
				
			}
			else {
				if (pMenu->AddStaticButton(u8"Архивировать Anomaly")) {
					sn->currentTab = TabId::CONSOLE;
					executor->AddTask([]() {}, TaskType::PACK_ANOMALY_ZIP);
				}
			}
		

			if (IsRunningMo2Mods()) {
				TextWithColors(u8"{AAFFFF} Выполняется Архивация Модов");
				SameLine();
				if (pMenu->AddStaticButton(u8"Отменить"))
				{
					if (executor->init())
					{
						std::vector<Task> tasks = executor->GetAllTasks(); // Используем глобальный экземпляр  

						// Остановка задач
						for (const Task& task : tasks) {
							if ((task.status == TaskStatus::PREPARING || task.status == TaskStatus::RUNNING) && task.type == TaskType::PACK_MODS_ZIP)
							{
								executor->CancelTask(task.id);
							}
						}
						AbortMo2ModsPack(); // если уже идёт — остановить
					}
				};

			}
			else {
				if (pMenu->AddStaticButton(u8"Архивировать Моды")) {
					sn->currentTab = TabId::CONSOLE;
					executor->AddTask([]() {}, TaskType::PACK_MODS_ZIP);
				}
			}

			if (IsRunningMo2Top()) {
				TextWithColors(u8"{AAFFFF} Выполняется Архивация MO2");
				SameLine();
				if (pMenu->AddStaticButton(u8"Отменить"))
				{
					if (executor->init())
					{
						std::vector<Task> tasks = executor->GetAllTasks(); // Используем глобальный экземпляр  

						// Остановка задач
						for (const Task& task : tasks) {
							if ((task.status == TaskStatus::PREPARING || task.status == TaskStatus::RUNNING) && task.type == TaskType::PACK_MO2_ZIP)
							{
								executor->CancelTask(task.id);
							}
						}
						AbortMo2TopPack(); // если уже идёт — остановить
					}
				};
			}
			else {
				if (pMenu->AddStaticButton(u8"Архивировать MO2")) {
					sn->currentTab = TabId::CONSOLE;
					executor->AddTask([]() {}, TaskType::PACK_MO2_ZIP);
				}
			}
		
	}

	
};