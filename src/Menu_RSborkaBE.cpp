#include "main.h"


void Menu_render_sborka() 
{
	if (pMenu->AddStaticButton(u8"Телеграм канал сборки Blue Eclipse"))
	{
		ImGui::OpenPopup(u8"Телеграм канал сборки Blue Eclipse");

	};

	ImGuiWindowFlags window_flags_about = 0;
	window_flags_about |= ImGuiWindowFlags_NoResize;
	window_flags_about |= ImGuiWindowFlags_NoCollapse;	

	if (ImGui::BeginPopupModal(u8"Телеграм канал сборки Blue Eclipse", 0, window_flags_about))
	{
		//	ImGui::SetWindowSize(ImVec2(293.f, 100.f));
		ImGui::Text(u8"Откроется ссылка в вашем браузере по умолчанию, Продолжить?");
		if (pMenu->AddStaticButton(u8"Да"))
		{
			ShellExecuteA(0, "open", "https://t.me/anomaly_blue_eclipse", 0, 0, 0);
			ImGui::CloseCurrentPopup();
		}
		if (pMenu->AddStaticButton(u8"Нет"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::Separator();

	if (pMenu->AddStaticButton(u8"Тема сборки Blue Eclipse на ap-pro"))
	{
		ImGui::OpenPopup(u8"Тема сборки Blue Eclipse на ap-pro");

	};

	if (ImGui::BeginPopupModal(u8"Тема сборки Blue Eclipse на ap-pro", 0, window_flags_about))
	{
		//	ImGui::SetWindowSize(ImVec2(293.f, 100.f));
		ImGui::Text(u8"Откроется ссылка в вашем браузере по умолчанию, Продолжить?");
		if (pMenu->AddStaticButton(u8"Да"))
		{
			ShellExecuteA(0, "open", "https://ap-pro.ru/forums/topic/9748-sborka-anomaly-152-by-blue-eclipse-team/", 0, 0, 0);
			ImGui::CloseCurrentPopup();
		}
		if (pMenu->AddStaticButton(u8"Нет"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::Separator();
	if (current_track == 0 && sn->my_radio.enabled == false)
	{
		ImGui::TextWithColors("%s[N\\A]", sradio[sn->radio_station].stationname);
	}
	else
	{
		ImGui::TextWithColors("%s[%s]", sradio[sn->radio_station].stationname, current_track);
	}
	ImGui::Separator();
	pMenu->AddButton(&sn->my_radio.enabled, "Online Radio");

	pMenu->AddSlider("Volume", sn->radio_volume, 1, 100, "%d");
	ImGui::Separator();
	if (pMenu->AddStaticButton(u8"Reconnect"))
	{
		sn->radio_reconnect = 1;
	};

	ImGui::Separator();

	if (pMenu->AddStaticButton(u8"Radio Control"))
	{
		ImGui::OpenPopup(u8"Radio Control");

	};

	ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowSize().x - 6, ImGui::GetWindowSize().y - 6));
	if (ImGui::BeginPopupModal(u8"Radio Control", 0, window_flags_about))
	{
		if (pMenu->AddStaticButton(u8"Закрыть"))
		{
			ImGui::CloseCurrentPopup();
		}
		//Menu here..
		if (pMenu->AddStaticButton(u8"Reconnect"))
		{
			sn->radio_reconnect = 1;
		};
		if (current_track == 0 && sn->my_radio.enabled == false)
		{
			ImGui::TextWithColors("%s[N\\A]", sradio[sn->radio_station].stationname);
		}
		else
		{
			ImGui::TextWithColors("%s[%s]", sradio[sn->radio_station].stationname, current_track);
		}
		ImGui::Separator();
		pMenu->AddButton(&sn->my_radio.enabled, "Online Radio");
		pMenu->AddSlider("Volume", sn->radio_volume, 1, 100, "%d");
		ImGui::Separator();

		if (ImGui::BeginChild("##Stations"))
		{
			for (int id = 0; id < MAX_RADIO_STATIONS; id++)
			{
				ImGui::TextWithColors("%s | ID %d", sradio[id].stationname, sradio[id].id);
				ImGui::SameLine();
				ImGui::PushID(id);
				if (pMenu->AddStaticButton(u8"Выбрать"))
				{
					sn->radio_station = id;
					//sn->radio_reconnect = 1;
					//Log("id > %d / %d ", sn->radio_station, id);
				}
				ImGui::PopID();
			}
			ImGui::EndChild();
		};
		ImGui::EndPopup();
	}

};