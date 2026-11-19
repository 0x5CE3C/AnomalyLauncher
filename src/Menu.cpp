#include "main.h"
#include "ImFuncs.h"

void UpdateSystemInfo()
{
	sysi->SetUpdateInterval(2000);
	sysi->UpdateInfo();
}

namespace fs = std::filesystem;


void RenderProgressBar()
{
	static float progress = 0.0f;  // Set your progress value here (0.0 to 1.0)
	progress = progress_ / 100;
	bool showprogressbar = false;

	auto httpclient = HttpClient::getInstance();

	if (httpclient->IsDownloading())
	{
		if (progress > 0.0f)
		{
			ImGui::TextWithColors(u8"{FF4500} [ %s ] ", cp1251_to_utf8(httpclient->GetDownloadFileName().c_str()).c_str());
			ImGui::SameLine();
			ImGui::TextWithColors(download_info.c_str());
			showprogressbar = true;
			if (showprogressbar)
			{
				ImGui::ProgressBar(progress, ImVec2(ImGui::GetWindowSize().x, 17));				
			}
		}
	}


	if (!httpclient->IsDownloading())
	{
		//sn->con.AddLog(u8"else if (file_downloaded == true)");
		//Log("else if (file_downloaded == true)");
		if (progress >= 99.0f || progress != 0.0f)
		{			
			showprogressbar = 0;
			progress = 0.0f;
			{				
				ImGui::TextWithColors(u8"{AAFFFF}Файл [ %s ] успешно загружен!", cp1251_to_utf8(httpclient->GetDownloadFileName().c_str()).c_str());
				//ImGui::ProgressBar(progress, ImVec2(ImGui::GetWindowSize().x, 17));				
			}
		}
	}


}


bool g_сhanged = false;
bool g_SettingsDirty = false;    // Флаг: были изменения
float g_LastSettingChangeTime = 0.0f; // Время последнего изменения

void UpdateAutoSave() {
	const float AUTO_SAVE_DELAY = 2.0f; // Сохранять через 2 секунды после последнего изменения
	float currentTime = ImGui::GetTime();

	if (g_SettingsDirty && (currentTime - g_LastSettingChangeTime) >= AUTO_SAVE_DELAY) {
		Save();           // Твоя функция сохранения в файл
		g_SettingsDirty = false;
		sn->con.AddLog("Настройки автоматически сохранены.");
	}
}

bool Edited() {
	return ImGui::IsItemEdited();
}

using namespace ImGui;
void MyGui::renderImguiMenu(bool* p_open)
{
	funcs_enabler();	
	radio_control();

	auto thw = ThreadWrapper::create();


	thw->beginThread(UpdateSystemInfo);			
	thw->detachAll();


	Menu_render_info();
	ImGui::Separator();
	
	if (launcher_up_unlocked) {
		// Приоритет 1: обновление лаунчера
		if (pMenu->AddStaticButton(u8"Обновить лаунчер", 0.0f, true)) {
			auto thw = ThreadWrapper::create();
			thw->beginAsync([]() {
				SetupLauncherUpdate();
				});
			thw->detachAll();
		}
	}
	else if (modpack_up_unlocked) {
		if (!sn->disable_update_modpack_button.enabled)
		{
			// Приоритет 2: обновление сборки (только если лаунчер не требует обновления)
			if (executor->init())
			{
				if (executor->GetRemainingTasks() > 0)
				{
					ImGui::Text(u8"Обновление сборки >");ImGui::SameLine();
					float prog = 0.0f;
					prog = executor->GetProgress();
					if (prog >= 100.0f) { executor->ResetCounters(); }
					// Общий прогресс выполнения
					ImGui::Text(u8"Общий прогресс выполнения: %.1f%%", prog);
					ImGui::ProgressBar(prog / 100.0f, ImVec2(-1.0f, 0.0f));
					ImGui::Spacing();
				}
				else
				{
					if (pMenu->AddStaticButton(u8"Обновить сборку", 0.0f, true)) {
						auto thw = ThreadWrapper::create();
						thw->beginAsync([]() {
							SetupModPackUpdate();
							});
						thw->detachAll();
					}
					ImFuncs::FastDescr(u8"{AAFFFF} Данная клавиша так же устанавливает сборку способом докачки сломанных или недостающих частей.\n\
		{FFFFFF}удаляет сторонние модификации которые вы когда-то ставили поверх сборки!!!\n\
		{FFFF00}Чтобы избежать удаления перейдите в раздел [Отладка] > нажмите \n[ Переместить сторонние моды в резерв ]\n\
		Ваши моды переместятся в > {FFFFFF}\\MO2\\backup\\external_mods\\");/// R G B A
				}
			}
		}
		else
		{	
			if (executor->init())
			{
				if (executor->GetRemainingTasks() > 0)
				{
					if (!sn->buseMO2.enabled) {
						if (fsm->FileExist(GetRenderer().c_str()) == false) {
							BeginDisabled();
							if (pMenu->AddStaticButton(u8"Играть")) {
							}
							EndDisabled();
							module_anomaly_damaged = true;
						}
						else {
							if (scanner_anomaly_enable == true || module_anomaly_damaged == true) {
								BeginDisabled();
								if (pMenu->AddStaticButton(u8"Играть")) {
								}
								EndDisabled();
							}
							else {
								BeginDisabled();
								if (pMenu->AddStaticButton(u8"Играть")) {
								}
								EndDisabled();
							}
						}
					}
					else {
						if (DetectLaunchbyModOrganizer()) {
							sn->buseMO2.enabled = false;
							sn->download_system.enabled = false;
						}
						std::string strMO2 = GetMO2Folder() + "\\" + "ModOrganizer.exe";
						if (fsm->FileExist(strMO2.c_str()) == false) {
							BeginDisabled();
							if (pMenu->AddStaticButton(u8"Запустить Mod Organizer 2")) {
							}
							EndDisabled();
						}
						else {
							BeginDisabled();
							if (pMenu->AddStaticButton(u8"Запустить Mod Organizer 2")) {
							}
							EndDisabled();
						}
					}
				}
				else
				{
					if (!sn->buseMO2.enabled) {
						if (fsm->FileExist(GetRenderer().c_str()) == false) {
							BeginDisabled();
							if (pMenu->AddStaticButton(u8"Играть")) {
								ShellExecuteA(0, "open", GetRenderer().c_str(), GetLaunchparams().c_str(), 0, SW_SHOWNORMAL);
							}
							EndDisabled();
							module_anomaly_damaged = true;
						}
						else {
							if (scanner_anomaly_enable == true || module_anomaly_damaged == true) {
								BeginDisabled();
								if (pMenu->AddStaticButton(u8"Играть")) {
									ShellExecuteA(0, "open", GetRenderer().c_str(), GetLaunchparams().c_str(), 0, SW_SHOWNORMAL);
								}
								EndDisabled();
							}
							else {
								if (pMenu->AddStaticButton(u8"Играть")) {
									ShellExecuteA(0, "open", GetRenderer().c_str(), GetLaunchparams().c_str(), 0, SW_SHOWNORMAL);
									ExitProcess(0);
								}
							}
						}
					}
					else {
						if (DetectLaunchbyModOrganizer()) {
							sn->buseMO2.enabled = false;
							sn->download_system.enabled = false;
						}
						std::string strMO2 = GetMO2Folder() + "\\" + "ModOrganizer.exe";
						if (fsm->FileExist(strMO2.c_str()) == false) {
							BeginDisabled();
							if (pMenu->AddStaticButton(u8"Запустить Mod Organizer 2")) {
								ShellExecuteA(0, "open", strMO2.c_str(), 0, 0, SW_SHOWNORMAL);
							}
							EndDisabled();
						}
						else {
							if (pMenu->AddStaticButton(u8"Запустить Mod Organizer 2")) {
								ShellExecuteA(0, "open", strMO2.c_str(), 0, 0, SW_SHOWNORMAL);
								ExitProcess(0);
							}
						}
					}
				}
			}
		}
	}
	else {
		
		
			if (!sn->buseMO2.enabled) {
				if (fsm->FileExist(GetRenderer().c_str()) == false) {
					BeginDisabled();
					if (pMenu->AddStaticButton(u8"Играть")) {
						ShellExecuteA(0, "open", GetRenderer().c_str(), GetLaunchparams().c_str(), 0, SW_SHOWNORMAL);
					}
					EndDisabled();
					module_anomaly_damaged = true;
				}
				else {
					if (scanner_anomaly_enable == true || module_anomaly_damaged == true) {
						BeginDisabled();
						if (pMenu->AddStaticButton(u8"Играть")) {
							ShellExecuteA(0, "open", GetRenderer().c_str(), GetLaunchparams().c_str(), 0, SW_SHOWNORMAL);
						}
						EndDisabled();
					}
					else {
						if (pMenu->AddStaticButton(u8"Играть")) {
							ShellExecuteA(0, "open", GetRenderer().c_str(), GetLaunchparams().c_str(), 0, SW_SHOWNORMAL);
							ExitProcess(0);
						}
					}
				}
			}
			else {
				if (DetectLaunchbyModOrganizer()) {
					sn->buseMO2.enabled = false;
					sn->download_system.enabled = false;
				}
				std::string strMO2 = GetMO2Folder() + "\\" + "ModOrganizer.exe";
				if (fsm->FileExist(strMO2.c_str()) == false) {
					BeginDisabled();
					if (pMenu->AddStaticButton(u8"Запустить Mod Organizer 2")) {
						ShellExecuteA(0, "open", strMO2.c_str(), 0, 0, SW_SHOWNORMAL);
					}
					EndDisabled();
				}
				else {
					if (pMenu->AddStaticButton(u8"Запустить Mod Organizer 2")) {
						ShellExecuteA(0, "open", strMO2.c_str(), 0, 0, SW_SHOWNORMAL);
						ExitProcess(0);
					}
				}
			}
		//}
	}			

	ImGui::Separator();

	float anim_speed = 0.2f;
	ImGuiToggleFlags toggle_flags = ImGuiToggleFlags_Animated;
	ImGui::Toggle(u8"Запускать Mod Organizer 2 вместо Anomaly", &sn->buseMO2.enabled, toggle_flags, anim_speed);
	ImFuncs::FastDescr(u8"{AAFFFF} Переключатель позволяет запускать сразу Mod Organizer 2, затем сборку модов на Anomaly.");/// R G B A
	ImGui::SameLine();
	ImGui::Toggle(u8"Отключение кнопки \"Обновить сборку\"", &sn->disable_update_modpack_button.enabled, toggle_flags, anim_speed);	

	g_сhanged |= Edited();	

	ImGui::Separator();

	if (!sn->buseMO2.enabled)
	{
		if (fsm->FolderExist("bin") == false)
		{
			TextWrapped(u8"папка bin не обнаружена");
		}
		else
		{
			if (fsm->FileExist(GetRenderer().c_str()) == false)
			{
				TextWrapped(u8"файл рендера %s в папке bin не обнаружен", cp1251_to_utf8(GetRenderer().c_str()).c_str());
			}
			else
			{
				TextWrapped(u8"параметры запуска: %s %s", cp1251_to_utf8(GetRenderer().c_str()).c_str(), cp1251_to_utf8(GetLaunchparams().c_str()).c_str());
			}
		}
	}
	else
	{
		if (fsm->FolderExist(GetMO2Folder().c_str()) == false)
		{
			TextWrapped(u8"папка MO2 не обнаружена");
		}
		else
		{			
			std::string strMO2 = GetMO2Folder() + "\\" + "ModOrganizer.exe";
			if (fsm->FileExist(strMO2.c_str()) == false)
			{
				TextWrapped(u8"файл ModOrganizer.exe в папке MO2 не обнаружен");
			}
			else
			{
				TextWrapped(u8"файл MO2 обнаружен: %s", cp1251_to_utf8(strMO2).c_str());
			}
		}
	}
	
	ImGui::Separator();

		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("##Tabs", tab_bar_flags))
		{
			
			ImGuiTabItemFlags tab_item_flags = ImGuiTabItemFlags_NoTooltip;
			ImGuiStyle& style = ImGui::GetStyle();
			ImVec4 activeColor = style.Colors[ImGuiCol_TabActive]; // цвет активного таба
			ImVec4 normalColor = style.Colors[ImGuiCol_Tab];   // обычный цвет таба

			auto DrawTab = [&](const char* label, TabId id) {
				bool isActive = (sn->currentTab == id);
				if (isActive) {
					ImGui::PushStyleColor(ImGuiCol_Tab, activeColor);
					ImGui::PushStyleColor(ImGuiCol_TabActive, activeColor);
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Tab, normalColor);
					ImGui::PushStyleColor(ImGuiCol_TabActive, normalColor);
				}

				if (ImGui::TabItemButton(label, tab_item_flags))
					sn->currentTab = id;

				ImGui::PopStyleColor(2);
				};


			
			DrawTab(u8"Общие", TabId::GENERAL);			
			DrawTab(u8"Отладка", TabId::DEBUG);			
			DrawTab(u8"Задачи", TabId::TASKS);
			DrawTab(u8"Модули", TabId::MODULES);
			DrawTab(u8"Листы", TabId::LISTS);						

			bool has_any_update = (launcher_up_unlocked == 1) || (modpack_up_unlocked == 1);

			if (has_any_update)
			{
				ImVec4 accent_border = ImVec4(0.667f, 1.000f, 1.000f, 1.000f); // #AAFFFF
				ImVec4 accent_button = ImVec4(0.400f, 0.800f, 0.800f, 1.000f);

				ImVec4 border_color = fsm->ColorOpacityPulse(accent_border, 0.050f, 1.000f, 0.100f, 60);
				ImVec4 button_color = fsm->ColorOpacityPulse(accent_button, 0.050f, 1.000f, 0.100f, 20);

				ImGui::PushStyleColor(ImGuiCol_Border, border_color);
				ImGui::PushStyleColor(ImGuiCol_Button, button_color);

				DrawTab(u8"Обновления", TabId::CHANGELOG);
/*				if (ImGui::TabItemButton(u8"Обновления", tab_item_flags))
				{
					sn->currentTab = TabId::CHANGELOG;
					// Никаких глобальных флагов — просто переходим
				}*/

				ImGui::PopStyleColor(2);
			}
			else
			{
				DrawTab(u8"Обновления", TabId::CHANGELOG);
				/*if (ImGui::TabItemButton(u8"Обновления", tab_item_flags))
				{
					sn->currentTab = TabId::CHANGELOG;
				}*/
			}	

			DrawTab(u8"Помощь", TabId::HELP);
			DrawTab(u8"Настройки", TabId::SETTINGS);
			DrawTab(u8"Консоль", TabId::CONSOLE);
			//DrawTab(u8"О программе", TabId::CREDITS);			

			ImGui::EndTabBar();
		}

			if (sn->currentTab == TabId::GENERAL)
			{
				Menu_render_general();
			}
			else if (sn->currentTab == TabId::DEBUG)
			{
				//sn->currentTab = TabId::DEBUG;
				if (BeginChild("Settings")) {
					Menu_render_debug();
					EndChild();
				}
			}
			else if (sn->currentTab == TabId::TASKS)
			{
				if (executor->init() == true)
				{
					executor->RenderTaskTab();
				}
				else
				{
					Text(u8"Менеджер задач не инициализирован");
				}
			}
			else if (sn->currentTab == TabId::MODULES)
			{
				Menu_render_modules();
			}
			else if (sn->currentTab == TabId::LISTS)
			{
				Menu_render_lists();
			}
			else if (sn->currentTab == TabId::CHANGELOG)
			{
				if (BeginChild("lastchanges")) {
					Menu_render_updates();
					EndChild();
				}
			}
			else if (sn->currentTab == TabId::SETTINGS)
			{
				if (BeginChild("Settings")) {
					Menu_render_settings();
					EndChild();
				}
			}
			else if (sn->currentTab == TabId::HELP)
			{
				if (BeginChild("libskit")) {
					Menu_render_helper();
					EndChild();
				}
			}
			else if (sn->currentTab == TabId::CONSOLE)
			{
				//if (BeginChild("Console")) {
					/*if (pMenu->AddStaticButton(u8"Очистить"))
					{
						sn->con.ClearLog();
						sn->con.AddLog("{FF0000}Консоль Очищена!");
					};*/
					sn->con.Draw(u8"console");
				//}
			}
			/*else if (sn->currentTab == TabId::CREDITS)
			{
				ImGuiChildFlags chflags = ImGuiChildFlags_None;
				ImGuiWindowFlags wflags = ImGuiWindowFlags_NoScrollbar;
				if (ImGui::BeginChild("credits", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar))
				{
					Menu_render_credits();
					ImGui::EndChild();
				}
			}*/

}

