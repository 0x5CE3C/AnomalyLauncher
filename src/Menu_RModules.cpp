#include "main.h"

using namespace ImGui;
using namespace ImFuncs;
void Menu_render_modules() 
{	
	if (BeginChild("Modules")) {
	

	if (fsm->FileExist(zipPath.c_str()) == false)
	{
		module_extractor_damaged = true;
	}
	else
	{
		module_extractor_damaged = false;
	}

	if (module_extractor_damaged)
	{
		pMenu->AddMenuText(u8" Extractor", true, ImColorOrange, false);
		SameLine();
		Text(u8" Модуль повреждён или отсутствует!");
		SameLine();
		if (scanner_extractor_enable)
		{
			TextWithColors(u8"{AAFFFF} Выполняется Сканирование");
			SameLine();
			if (pMenu->AddStaticButton(u8"Отменить"))
			{
				scanner_extractor_enable = false;
			};
		}
		else
		{
			if (pMenu->AddStaticButton(u8"Проверить целостность файлов"))
			{
				scanner_extractor_enable = true;
				auto thw = ThreadWrapper::create();
				thw->beginAsync(ScanModule, SCAN_MODULE_TYPE_EXTRACTOR);
				thw->detachAll();
			};
		}
	}
	else
	{
		if (module_anomaly_damaged)
		{
			pMenu->AddMenuText(u8" Anomaly", true, ImColorOrange, false);
			SameLine();
			Text(u8" Модуль повреждён или отсутствует!");
			SameLine();
			if (scanner_anomaly_enable)
			{
				TextWithColors(u8"{AAFFFF} Выполняется Сканирование");
				SameLine();
				if (pMenu->AddStaticButton(u8"Отменить"))
				{
					scanner_anomaly_enable = false;
				};
			}
			else
			{
				if (pMenu->AddStaticButton(u8"Проверить целостность файлов"))
				{
					scanner_anomaly_enable = true;
					auto thw = ThreadWrapper::create();
					thw->beginAsync(ScanModule, SCAN_MODULE_TYPE_ANOMALY);
					thw->detachAll();
				};
			}
		}
		else
		{
			pMenu->AddMenuText(u8" Anomaly", true, ImColorGreen, false);
			SameLine();
			if (scanner_anomaly_enable)
			{
				TextWithColors(u8"{AAFFFF} Выполняется Сканирование");
				SameLine();
				if (pMenu->AddStaticButton(u8"Отменить"))
				{
					scanner_anomaly_enable = false;
				};
			}
			else
			{
				if (pMenu->AddStaticButton(u8"Принудительно сканировать"))
				{
					//sn->con.AddLog(u8"Нажата клавиша > {FF0000} 'Проверить целостность файлов'");
					scanner_anomaly_enable = true;
					auto thw = ThreadWrapper::create();
					thw->beginAsync(ScanModule, SCAN_MODULE_TYPE_ANOMALY);
					thw->detachAll();
				};
			}
			Text(u8"Путь: %s", cp1251_to_utf8(GetExePath().c_str()).c_str());
		}

		Separator();

		if (module_mo2_damaged)
		{
			pMenu->AddMenuText(u8" Mod Organizer 2", true, ImColorOrange, false);
			SameLine();
			Text(u8" Модуль повреждён или отсутствует!");
			SameLine();
			if (scanner_mo2_enable)
			{
				TextWithColors(u8"{AAFFFF} Выполняется Сканирование");
				SameLine();
				if (pMenu->AddStaticButton(u8"Отменить"))
				{
					scanner_mo2_enable = false;
				};
			}
			else
			{
				if (pMenu->AddStaticButton(u8"Проверить целостность файлов "))
				{
					scanner_mo2_enable = true;
					auto thw = ThreadWrapper::create();
					thw->beginAsync(ScanModule, SCAN_MODULE_TYPE_MO2);
					thw->detachAll();
				};
			}
		}
		else
		{
			pMenu->AddMenuText(u8" Mod Organizer 2", true, ImColorGreen, false);
			SameLine();
			if (scanner_mo2_enable)
			{
				TextWithColors(u8"{AAFFFF} Выполняется Сканирование");
				SameLine();
				if (pMenu->AddStaticButton(u8"Отменить"))
				{
					scanner_mo2_enable = false;
				};
			}
			else
			{
				if (pMenu->AddStaticButton(u8"Принудительно сканировать "))
				{
					scanner_mo2_enable = true;
					auto thw = ThreadWrapper::create();
					thw->beginAsync(ScanModule, SCAN_MODULE_TYPE_MO2);
					thw->detachAll();
				};
			}
			Text(u8"Путь: %s", cp1251_to_utf8(GetMO2Folder().c_str()).c_str());
			if (pMenu->AddStaticButton(u8"Пропатчить ModOrganizer.ini"))
			{
				PatchModOrganizerbtn();
			};
		}

		Separator();	

		std::string modpack_name = u8" сборка ";

		switch (sn->umode.project_mode) {
		case PROJECTMODES::PM_Anomaly_PZ:			
			modpack_name.append(u8"Purge of Zone");
			break;
		case PROJECTMODES::PM_Anomaly_MP:
			modpack_name.append(sn->umode.modpack_name);
			break;
		default:
			break;
		}

		if (module_be_damaged)
		{
			pMenu->AddMenuText(modpack_name.c_str(), true, ImColorOrange, false);
			SameLine();
			Text(u8" Модуль повреждён или отсутствует!");
			SameLine();
			if (scanner_be_enable)
			{
				TextWithColors(u8"{AAFFFF} Выполняется Сканирование");
				SameLine();
				if (pMenu->AddStaticButton(u8"Отменить"))
				{
					scanner_be_enable = false;
				};
			}
			else
			{
				if (pMenu->AddStaticButton(u8"Проверить целостность файлов"))
				{
					scanner_be_enable = true;
					auto thw = ThreadWrapper::create();
					thw->beginAsync(ScanModule, SCAN_MODULE_TYPE_BE);
					thw->detachAll();
				};
			}
		}
		else
		{
			pMenu->AddMenuText(modpack_name.c_str(), true, ImColorGreen, false);
			SameLine();
			if (scanner_be_enable)
			{
				TextWithColors(u8"{AAFFFF} Выполняется Сканирование");
				SameLine();
				if (pMenu->AddStaticButton(u8"Отменить"))
				{
					scanner_be_enable = false;
				};
			}
			else
			{
				if (pMenu->AddStaticButton(u8"Принудительно сканировать  "))
				{
					scanner_be_enable = true;
					auto thw = ThreadWrapper::create();
					thw->beginAsync(ScanModule, SCAN_MODULE_TYPE_BE);
					thw->detachAll();
				};
			}
			Text(u8"Путь: %s", cp1251_to_utf8(GetBEFolder().c_str()).c_str());
		}
	}


	SeparatorText(u8"Опции сканирования");
	pMenu->AddButton(&sn->download_system.enabled, u8"Использовать систему докачки");	
	ImFuncs::FastDescr(u8"{AADDDD}Во время сканирования любого из модулей не найденные файлы или каталоги будут автоматически скачиваться по ссылкам из списка. Списки можно посмотреть во вкладке 'Листы'.");
	Separator();	
	pMenu->AddButton(&sn->scanhashes.enabled, u8"Проверять хэш-суммы файлов", gui::size.x / 2);
	ImFuncs::FastDescr(u8"{BBDDDD}Если у файлов не совпадает хэш-сумма, это означает что они изменялись. \n [Внимание]:\n На вычисления хэш-сумм требуется длительное время, поэтому эта функция замедляет сканирование!!! \n Длительность сканирования зависит от веса файла.");
	SameLine();
	pMenu->AddButton(&sn->scanhashes_dirs.enabled, u8"Проверять хэш-суммы каталогов");
	ImFuncs::FastDescr(u8"{BBDDDD}Если у каталогов не совпадает хэш-сумма, это означает что они изменялись. \n [Внимание]:\n На вычисления хэш-сумм каталогов требуется более длительное время, поэтому эта функция замедляет сканирование!!!\n Длительность сканирования зависит от веса файлов внутри каталога и подкаталогов.");
	Separator();

		if (strcmp(modpack_latest_ver.c_str(), sn->set.modpack_version.c_str()) == 0)
		{
			if (isRunningCheckHash())
			{
				TextWithColors(u8"{AAFFFF} Выполняется Восстановление Модулей");
				SameLine();
				if (pMenu->AddStaticButton(u8"Отменить"))
				{
					if (executor->init())
					{
						std::vector<Task> tasks = executor->GetAllTasks(); // Используем глобальный экземпляр  

						// Остановка задач
						for (const Task& task : tasks) {
							if ((task.status == TaskStatus::PREPARING || task.status == TaskStatus::RUNNING) && task.type == TaskType::HASH_CHECK_MODS)
							{
								executor->CancelTask(task.id);
							}

							if ((task.status == TaskStatus::PREPARING || task.status == TaskStatus::RUNNING) && task.type == TaskType::SCANNING_RESTORE_ALL)
							{
								executor->CancelTask(task.id);

							}
						}

						// Остановка сканеров и всего прочего.
						/*AbortCurrentDebugTask();
						cancellationRequested = true;
						scanner_extractor_enable = false;
						scanner_anomaly_enable = false;
						scanner_mo2_enable = false;
						scanner_be_enable = false;
						if (sn->download_system.enabled) sn->download_system.enabled = false;
						if (sn->scanhashes.enabled) sn->scanhashes.enabled = false;
						if (sn->scanhashes_dirs.enabled) sn->scanhashes_dirs.enabled = false;*/
					}
				};
			}
			else
			{
				if (pMenu->AddStaticButton(u8"Провера целостности и восстановление ВСЕХ модулей"))
				{
					if (sn->download_system.enabled == false) sn->download_system.enabled = true;
					if (sn->scanhashes.enabled == false) sn->scanhashes.enabled = true;
					if (sn->scanhashes_dirs.enabled == false) sn->scanhashes_dirs.enabled = true;

					filelistupdate();
					executor->AddTask([]() {}, TaskType::HASH_CHECK_MODS);
					executor->AddTask([]() {}, TaskType::SCANNING_RESTORE_ALL);
				}
			}		
		}

	EndChild();
	};
};