#include "main.h"
using namespace ImGui;
void Menu_render_general()
{	
	ImGui::SeparatorText(u8"Основные параметры запуска");
	/*const char* resolutions = "\
1920 x 1080\0\
RP Undetected Names\0\
Matrix Names\0\
[Your Nickname]_num";
ImGui::Combo(u8"Разрешение экрана", &sn->set.display_resolutions_type, resolutions);
			ImGui::Separator();*/

	const char* render_t = "\
DirectX 8\0\
DirectX 9\0\
DirectX 10\0\
DirectX 11";	
	ImGui::PushItemWidth(200);	
	ImGui::Combo(u8"[Рендер]", &sn->set.render_type, render_t);
	//g_сhanged |= Edited();
	ImGui::PopItemWidth();
	//SameLine();
	//ImFuncs::HelpDescr(u8"{FF0004}для ПК 10-14 года РЕКОМЕНДУЕТСЯ DirectX 8/9, для работы DirectX 9 выключить No_Ram_Textures(Удержание текстур в памяти) всё зависит от характеристик пк, 6-8 Гб оперативы достаточно для DirectX 10, всё что выше DirectX 11");/// R G B A

	ImGui::SameLine(0.0f, 40.0f);
	ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8" | ");
	ImGui::SameLine(0.0f, 40.0f);

	const char* shadow_map = "\
Shadow Map Size 1536\0\
Shadow Map Size 2048\0\
Shadow Map Size 2560\0\
Shadow Map Size 3072\0\
Shadow Map Size 4096";
	ImGui::PushItemWidth(200);
	ImGui::Combo(u8"[Размер Карты Теней]", &sn->set.shadow_map_type, shadow_map);
	ImGui::PopItemWidth();
	//SameLine();
	//ImFuncs::HelpDescr(u8"{FF0003}изменяет качество теней, НЕ РЕКОМЕНДУЕТСЯ ставить высокое значение на слабых ПК, так как компу может стать плохо. :)");/// R G B A

	ImGui::SeparatorText(u8"Дополнительные параметры запуска");
	Menu_render_optimization();

	/*if (IsCheckAVXSupport())
	{
		pMenu->AddButton(&sn->avx_support.enabled, u8"Включить поддержку для AVX");
	}*/

	ImGui::SeparatorText(u8"Быстрые действия");
	if (pMenu->AddStaticButton(u8"Открыть папку с лаунчером"))
	{
		const char* edit = GetExePath().c_str();
		ShellExecute(NULL, "explore", edit, NULL, NULL, SW_SHOWNORMAL);
	};

	ImGui::Separator();

	if (pMenu->AddStaticButton(u8"Открыть папку с логами"))
	{
		// Получаем путь сразу в CP1251 (без конвертации в UTF-8)
		std::string openlogsf = GetExePath() + "\\appdata\\logs";
		//sn->con.AddLog("Путь к логам: %s",openlogsf.c_str());
		// FolderExist должен ожидать CP1251 строку и проверять существование папки
		if (fsm->FolderExist(openlogsf.c_str()))
		{
			// ShellExecuteA ожидает ANSI строку (CP1251 в русской Windows)
			ShellExecuteA(NULL, "explore", openlogsf.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		else
		{
			sn->con.AddLog("Папка с логами не найдена");
		}
	};

	ImGui::Separator();

	if (pMenu->AddStaticButton(u8"Открыть папку с сохранениями"))
	{
		std::string opensavesf = GetExePath() + "\\appdata\\savedgames";
		if (fsm->FolderExist(opensavesf.c_str()))
		{
			ShellExecuteA(NULL, "explore", opensavesf.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		else {
			sn->con.AddLog("Папка с сохранениями не найдена");
		}
	};

	ImGui::Separator();

	if (pMenu->AddStaticButton(u8"Открыть папку с конфигами лаунчера"))
	{
		// 1. Получаем путь к папке, где лежит EXE (уровень выше)
		std::string basePath = GetExePath();

		// 2. Добавляем имя папки с конфигами
		// Теперь путь будет: ...\Аномали Пурга Зоны\AnomalyLauncher
		std::string configPath = basePath + "\\AnomalyLauncher";

		// 3. Логируем. 
		// Если твой AddLog нормально показывает кириллицу без конвертации - пиши так:
		sn->con.AddLog("Открываем папку с конфигами: %s", configPath.c_str());

		// ЕСЛИ в логе всё равно кракозябры (РРіСЂС‹), раскомментируй эту строку вместо предыдущей:
		// sn->con.AddLog("Открываем папку с конфигами: %s", ansiToUtf8(configPath).c_str());

		// 4. Проверяем существование ВЛОЖЕННОЙ папки
		if (fsm->FolderExist(configPath.c_str()))
		{
			// 5. Открываем её
			//ShellExecuteA(NULL, "explore", configPath.c_str(), NULL, NULL, SW_SHOWNORMAL);

			std::string targetPath = GetExePath() + "\\AnomalyLauncher\\";

			// Вместо ShellExecuteA используем системную команду
			// Важно: кавычки вокруг пути обязательны, если там есть пробелы!
			//std::string cmd = "explorer.exe \"" + targetPath + "\"";
			//system(cmd.c_str());

			//sn->con.AddLog("Команда для запуска: %s", cmd.c_str());
			ShellExecuteA(NULL, "explore", targetPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		else
		{
			sn->con.AddLog("ОШИБКА: Папка не с конфигами найдена! Путь: %s", configPath.c_str());
		}
	};

	ImGui::Separator();
	

	pMenu->AddStaticButton(u8"Appdata control");

	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	ImGui::SetNextWindowSize(ImVec2(gui::WINDOW_WIDTH / 2, -footer_height_to_reserve));

	if (ImGui::BeginPopupContextItem("miscf", ImGuiPopupFlags_MouseButtonLeft))
	{

		if (pMenu->AddStaticButton(u8"Восстановить стандартный user.ltx"))
		{
			RestoreUserltx();			
		};
		SameLine();
		ImFuncs::FastDescr(u8"{FF0008} сбрасывает все настройки в игровом меню настроек");/// R G B A

		ImGui::Separator();
		if (pMenu->AddStaticButton(u8"Очистить скриншоты"))
		{

			std::string remove_screenshotsfolder = GetExePath() + "\\appdata\\screenshots";

			if (fsm->FolderExist(remove_screenshotsfolder.c_str()))
			{
				sn->con.AddLog("Обнаружен каталог с скриншотами > {FF0000} %s", remove_screenshotsfolder.c_str());
				auto thw = ThreadWrapper::create();
				thw->beginAsync([remove_screenshotsfolder]() {
					fsm->DeleteFolderWithFiles(remove_screenshotsfolder.c_str());
					});
				sn->con.AddLog("Удалили каталог с скриншотами > %s", remove_screenshotsfolder.c_str());
			}
			else {
				sn->con.AddLog("Папка с скриншотами не найдена");
			}
		};

		ImGui::Separator();

		if (pMenu->AddStaticButton(u8"Очистить кэш шейдеров"))
		{

			std::string cachefr = GetExePath() + "\\appdata\\shaders_cache";			

			if (fsm->FolderExist(cachefr.c_str()))
			{
				sn->con.AddLog("Обнаружен кэш шейдеров > {FF0000} %s", cachefr.c_str());
				auto thw = ThreadWrapper::create();
				thw->beginAsync([cachefr]() {
					fsm->DeleteFolderWithFiles(cachefr.c_str());
					});				
				sn->con.AddLog("Удалили кэш шейдеров > %s", cachefr.c_str());
			}
			else {
				sn->con.AddLog("Папка с шейдерами не найдена");
			}
		};
		SameLine();
		ImFuncs::FastDescr(u8"{FF0005} Очищает кэш шейдеров. (Удаляет в папке_с_игрой/appdata папку 'shaders_cache' ) [ используется в процессе устранения проблем с графикой ]");/// R G B A

		ImGui::Separator();

		if (pMenu->AddStaticButton(u8"Очистить Логи"))
		{
			std::string logfr1 = GetExePath() + "\\appdata\\logs";

			if (fsm->FolderExist(logfr1.c_str()))
			{
				sn->con.AddLog("Обнаружена папка с логами > {FF0000} %s", logfr1.c_str());
				fsm->DeleteFolderWithFiles(logfr1.c_str());
				sn->con.AddLog("Удалили логи > %s", logfr1.c_str());
			}
			else {
				sn->con.AddLog("Папка с логами не найдена");
			}
		}

		ImGui::Separator();
		if (pMenu->AddStaticButton(u8"Очистить Сохранения"))
		{
			std::string savdfr1 = GetExePath() + "\\appdata\\savedgames";

			if (fsm->FolderExist(savdfr1.c_str()))
			{
				sn->con.AddLog("Обнаружена папка сохранений > {FF0000}", savdfr1.c_str());
				fsm->DeleteFolderWithFiles(savdfr1.c_str());
				sn->con.AddLog("Удалили сохранения > %s", savdfr1.c_str());
			}
			else {
				sn->con.AddLog("Папка с сохранениями не найдена");
			}
		}

		ImGui::EndPopup();
	}

};