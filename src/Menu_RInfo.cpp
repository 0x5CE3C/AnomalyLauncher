#include "main.h"

using namespace ImGui;

void Menu_render_info() 
{
	ImGui::TextColored(ImVec4(0.00, 2.00, 2.00, 2.00), u8"Автор > 0x5CE3C");
	
	SameLine();
	ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8" | ");
	SameLine();
	
	if (ImGui::Button(u8"Сохранить настройки"))
	{
		Save();
	};

	SameLine();
	ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8" | ");
	SameLine();

	switch (DetectLaunchbyModOrganizer())
	{
		case 0:
		{
			ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8"Unk");
			break;
		}
		case 1:
		{
			ImGui::TextColored(ImVec4(0.00, 2.00, 2.00, 2.00), u8"MO2");
			break;
		}		
	}
	ImGui::SameLine();
	ImFuncs::FastDescr(u8"{AAFFFF}Датчик, показывающий какая программа запускает Лаунчер. \n Всего 2 состояния: \n[Unk - любая программа, например проводник (explorer.exe)]\n[MO2 - запущено из Mod Organizer 2]");/// R G B A
	
	SameLine();
	ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8" | ");
	SameLine();

	switch (IsAppRunningAsAdminMode())
	{
	case 0:
	{
		ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8"П");
		ImGui::SameLine();
		ImFuncs::FastDescr(u8"{FF0000}Для нормальной работы программа должна быть запущена от имени администратора!!!");/// R G B A

		break;
	}
	case 1:
	{
		ImGui::TextColored(ImVec4(0.00, 2.00, 0.00, 2.00), u8"А");
		ImGui::SameLine();
		ImFuncs::FastDescr(u8"{FF0000}Программа запущена от имени администратора");/// R G B A
		break;
	}
	//default:
		//break;
	}

	//добавить bool "отображать статус сети"

	ImGui::SameLine();
	ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8" | ");
	ImGui::SameLine();

	std::string Inet_status;
	if(sysi->internet_status == true)
	{		
		Inet_status =  u8"{00FF00}Подключено";
	}
	else
	{
		Inet_status = u8"{FF0000}Нет связи";
	}
	
	ImGui::TextWithColors(u8"{FFFF00}Статус сети: ");
	ImGui::SameLine();
	ImGui::TextWithColors(u8"%s", Inet_status.c_str());	

	if (executor->init())
	{
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8" | ");
		ImGui::SameLine();

		uint32_t remaining = executor->GetRemainingTasks();
		uint32_t all = executor->GetTotalTasks();

		ImGui::TextColored(ImVec4(0.60, 0.85, 0.00, 2.00), u8"Задачи > %i\\%i", remaining, all);
	}	

	auto httpclient = HttpClient::getInstance();

	if (httpclient->init())
	{
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8" | ");
		ImGui::SameLine();

		if (httpclient->IsDownloading())
		{
			ImGui::Text("+");
		}
		else
		{
			ImGui::Text("-");
		}
		ImFuncs::FastDescr(u8"{AAFFFF}Датчик процесса скачивания файлов. \n Всего 2 состояния: \n[\"+\" - лаунчер что-то качает]\n[\"-\" - не качает, либо процесс остановки скачивания]");/// R G B A
	}
	
};