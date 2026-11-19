#include "main.h"
using namespace ImGui;

void Menu_render_updates() 
{	
	if (ImGui::TreeNode(u8"Лаунчер"))
	{
		if (::Button(u8"Обновить информацию"))
		{
			checklauncherupdate();
		};
		
		SameLine();

		switch (launcher_up_unlocked)
		{
		case 0:
		{
			ImGui::TextColored(ImVec4(0.00, 1.85, 0.00, 2.00), u8"Актуальная версия");
			break;
		}
		case 1:
		{
			std::string installupdatewindow = u8"Установить обновление ?";

			ImVec4 accent_border = ImVec4(0.667f, 1.000f, 1.000f, 1.000f); // #AAFFFF
			ImVec4 accent_button = ImVec4(0.400f, 0.800f, 0.800f, 1.000f);

			ImVec4 border_color = fsm->ColorOpacityPulse(accent_border, 0.050f, 1.000f, 0.100f, 60);
			ImVec4 button_color = fsm->ColorOpacityPulse(accent_button, 0.050f, 1.000f, 0.100f, 20);

			ImGui::PushStyleColor(ImGuiCol_Border, border_color);
			ImGui::PushStyleColor(ImGuiCol_Button, button_color);

			if (::Button(u8"Доступно обновление"))
			{
				ImGui::OpenPopup(installupdatewindow.c_str());
			};

			ImGui::PopStyleColor(2);

			ImGuiWindowFlags window_flags_about = 0;
			window_flags_about |= ImGuiWindowFlags_NoResize;
			window_flags_about |= ImGuiWindowFlags_NoCollapse;
			window_flags_about |= ImGuiWindowFlags_NoTitleBar;
			SetNextWindowSize(ImVec2(ImGui::GetWindowSize().x - 6, ImGui::GetWindowSize().y - 6));
			if (ImGui::BeginPopupModal(installupdatewindow.c_str(), 0, window_flags_about))
			{
				int width = strlen(installupdatewindow.c_str()) * 4;
				ImGui::SameLine(ImGui::GetWindowWidth() / 2 - (width / 2));
				ImGui::PushFont(pFont);

				ImGui::Text(installupdatewindow.c_str());

				if (pMenu->AddStaticButton(u8"Установить", gui::size.x / 2))
				{
					auto thw = ThreadWrapper::create();
					thw->beginAsync([]() {
						SetupLauncherUpdate();
						});
					thw->detachAll();

					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (pMenu->AddStaticButton(u8"Отмена"))
				{
					ImGui::CloseCurrentPopup();
				}


				if (launcher_last_changes_list.empty())
				{
					//sn->con.AddLog("last_changes_list.empty()");
					return;
				};

				if (BeginChild("latestverinfo"))
				{
					ImGui::TextWithColors(u8"{AAFFFF}Версия: %s \n{FFFF00}Изменения: \n{ffffff}%s\n", cp1251_to_utf8(launcher_latest_ver).c_str(), launcher_last_changes_list.c_str());

					EndChild();
				};

				ImGui::EndPopup();
			}

			break;
		}
		//default:
			//break;		
		}

		if (launcher_last_changes_list.empty())
		{
			//sn->con.AddLog("last_changes_list.empty()");
			return;
		};

		ImGui::TextWithColors(u8"{AAFFFF}Версия: %s \n{FFFF00}Изменения: \n{ffffff}%s\n", cp1251_to_utf8(launcher_latest_ver).c_str(), launcher_last_changes_list.c_str());

		ImGui::TreePop();
		ImGui::Spacing();
	}

	if (ImGui::TreeNode(u8"Сборка"))
	{
		if (::Button(u8"Обновить информацию "))
		{
			checkmodpacklatestchangelist();
		};
		
		SameLine();

		switch (modpack_up_unlocked)
		{
		case 0:
		{
			ImGui::TextColored(ImVec4(0.00, 1.85, 0.00, 2.00), u8"Актуальная версия ");
			break;
		}
		case 1:
		{
			std::string installupdatewindow = u8"Установить обновление ? ";

				ImVec4 accent_border = ImVec4(0.667f, 1.000f, 1.000f, 1.000f); // #AAFFFF
				ImVec4 accent_button = ImVec4(0.400f, 0.800f, 0.800f, 1.000f);

				ImVec4 border_color = fsm->ColorOpacityPulse(accent_border, 0.050f, 1.000f, 0.100f, 60);
				ImVec4 button_color = fsm->ColorOpacityPulse(accent_button, 0.050f, 1.000f, 0.100f, 20);

				ImGui::PushStyleColor(ImGuiCol_Border, border_color);
				ImGui::PushStyleColor(ImGuiCol_Button, button_color);				

				if (::Button(u8"Доступно обновление "))
				{
					ImGui::OpenPopup(installupdatewindow.c_str());
				};

				ImGui::PopStyleColor(2);

			ImGuiWindowFlags window_flags_about = 0;
			window_flags_about |= ImGuiWindowFlags_NoResize;
			window_flags_about |= ImGuiWindowFlags_NoCollapse;
			window_flags_about |= ImGuiWindowFlags_NoTitleBar;
			SetNextWindowSize(ImVec2(ImGui::GetWindowSize().x - 6, ImGui::GetWindowSize().y - 6));
			if (ImGui::BeginPopupModal(installupdatewindow.c_str(), 0, window_flags_about))
			{
				int width = strlen(installupdatewindow.c_str()) * 4;
				ImGui::SameLine(ImGui::GetWindowWidth() / 2 - (width / 2));
				ImGui::PushFont(pFont);

				ImGui::Text(installupdatewindow.c_str());

				if (pMenu->AddStaticButton(u8"Установить ", gui::size.x / 2))
				{
					auto thw = ThreadWrapper::create();
					thw->beginAsync([]() {
						SetupModPackUpdate();
						});
					thw->detachAll();

					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (pMenu->AddStaticButton(u8"Отмена "))
				{
					ImGui::CloseCurrentPopup();
				}


				if (modpack_last_changes_list.empty())
				{
					//sn->con.AddLog("last_changes_list.empty()");
					return;
				};

				if (BeginChild("latestverinfo"))
				{
					ImGui::TextWithColors(u8"{AAFFFF}Версия: %s \n{FFFF00}Изменения: \n{ffffff}%s\n", cp1251_to_utf8(modpack_latest_ver).c_str(), modpack_last_changes_list.c_str());

					EndChild();
				};

				ImGui::EndPopup();
			}

			break;
		}
		//default:
			//break;		
		}


		if (modpack_last_changes_list.empty())
		{
			return;
		};


		ImGui::TextWithColors(u8"{AAFFFF}Версия: %s \n{FFFF00}Изменения: \n{ffffff}%s\n", cp1251_to_utf8(modpack_latest_ver).c_str(), modpack_last_changes_list.c_str());

		ImGui::TreePop();
		ImGui::Spacing();
	}

};