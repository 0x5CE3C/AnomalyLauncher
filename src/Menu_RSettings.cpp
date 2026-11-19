#include "main.h"
#include "imgui_stdlib.h"
using namespace ImGui;
using namespace ImFuncs;

void Menu_render_settings()
{ 
	float anim_speed = 0.2f;
	ImGuiToggleFlags toggle_flags = ImGuiToggleFlags_Animated;

	// 1. Находим максимальную ширину текста среди всех строк
	float max_text_width = 0.0f;
	max_text_width = ImMax(max_text_width, ImGui::CalcTextSize(u8"Фоновая музыка").x);
	max_text_width = ImMax(max_text_width, ImGui::CalcTextSize(u8"Кнопки").x);
	max_text_width = ImMax(max_text_width, ImGui::CalcTextSize(u8"Вкладки").x);
	max_text_width = ImMax(max_text_width, ImGui::CalcTextSize(u8"Выпадающее меню").x);
	max_text_width = ImMax(max_text_width, ImGui::CalcTextSize(u8"Загрузка файлов").x);
	max_text_width = ImMax(max_text_width, ImGui::CalcTextSize(u8"Консоль и переключатели").x);

	// 2. Считаем финальную координату X для начала слайдеров:
	// Позиция = Отступ слева + Ширина самого Toggle (круглого элемента) + Ширина текста + Зазор
	float toggle_circle_width = ImGui::GetFrameHeight() * 1.5f; // Примерная ширина самого тумблера Toggle
	float spacing = ImGui::GetStyle().ItemInnerSpacing.x; // Стандартный внутренний зазор ImGui

	float slider_start_x = ImGui::GetCursorPosX() + toggle_circle_width + max_text_width + (spacing * 2);

	SeparatorText(u8"Звук");

	ImGui::Toggle(u8"Фоновая музыка", &sn->playbackgroundtheme.enabled, toggle_flags, anim_speed);		
	ImGui::SameLine(slider_start_x);	
	pMenu->AddSlider(u8"background_music_volume", sn->background_music_volume, 1, 100, "%d");

	ImGui::Toggle(u8"Кнопки", &sn->bmenu_item_nav.enabled, toggle_flags, anim_speed);
	ImGui::SameLine(slider_start_x);
	pMenu->AddSlider(u8"menu_item_nav_volume", sn->menu_item_nav_volume, 1, 100, "%d");

	ImGui::Toggle(u8"Вкладки", &sn->bmenu_nav.enabled, toggle_flags, anim_speed);
	ImGui::SameLine(slider_start_x);
	pMenu->AddSlider(u8"menu_nav_volume", sn->menu_nav_volume, 1, 100, "%d");


	ImGui::Toggle(u8"Выпадающее меню", &sn->bmenu_select.enabled, toggle_flags, anim_speed);
	ImGui::SameLine(slider_start_x);
	pMenu->AddSlider(u8"menu_select_volume", sn->menu_select_volume, 1, 100, "%d");

	ImGui::Toggle(u8"Загрузка файлов", &sn->bmenu_file_download.enabled, toggle_flags, anim_speed);
	ImGui::SameLine(slider_start_x);
	pMenu->AddSlider(u8"menu_file_download_volume", sn->menu_file_download_volume, 1, 100, "%d");
	
	ImGui::Toggle(u8"Консоль и переключатели", &sn->bmenu_log_message.enabled, toggle_flags, anim_speed);
	ImGui::SameLine(slider_start_x);
	pMenu->AddSlider(u8"menu_log_message_volume", sn->menu_log_message_volume, 1, 100, "%d");


	BeginGroup();
	SeparatorText(u8"Обновление");	
	ImGui::Toggle(u8"Обновлять лаунчер автоматически", &sn->auto_check_launcher_update.enabled, toggle_flags, anim_speed);
	ImGui::Toggle(u8"Скачивать список файлов при запуске", &sn->auto_update_filelist.enabled, toggle_flags, anim_speed);
	//ImGui::Toggle(u8"Скачивать список последних изменений при запуске", &sn->auto_update_changelist.enabled, toggle_flags, anim_speed);
	EndGroup();
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8" | ");	
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::SeparatorText(u8"Сеть");

	// 0 = без лимита, дальше в байтах
	static const uint64_t options_bps[] = {
		0,
		1 * 1024 * 1024,        // 1 МБ/с
		2 * 1024 * 1024,        // 2 МБ/с
		5 * 1024 * 1024,        // 5 МБ/с
		10 * 1024 * 1024,        // 10 МБ/с
		20 * 1024 * 1024,
		30 * 1024 * 1024,
		40 * 1024 * 1024,
		50 * 1024 * 1024,
		60 * 1024 * 1024,
		70 * 1024 * 1024,
		80 * 1024 * 1024
	};

	// Правильный массив указателей (так Combo работает корректно)
	static const char* labels[] = {
		u8"Без лимита",
		u8"1 МБ/с",
		u8"2 МБ/с",
		u8"5 МБ/с",
		u8"10 МБ/с",
		u8"20 МБ/с",
		u8"30 МБ/с",
		u8"40 МБ/с",
		u8"50 МБ/с",
		u8"60 МБ/с",
		u8"70 МБ/с",
		u8"80 МБ/с"
	};

	const size_t labelCount = IM_ARRAYSIZE(labels);

	// Текущее значение: ищем индекс в массиве
	uint64_t currentBps = HttpClient::getInstance()->GetMaxDownloadSpeedBps();
	int selected_index = sn->set.httpclient_limit_speed_type;

	for (size_t i = 0; i < IM_ARRAYSIZE(options_bps); ++i) {
		if (currentBps == options_bps[i]) {
			selected_index = static_cast<int>(i);
			break;
		}
	}
	// Если не совпало — оставляем как есть (или можно принудительно ставить 0)

	ImGui::PushItemWidth(180);
	if (ImGui::Combo(u8"[Лимит скорости]", &selected_index, labels, labelCount)) {
		uint64_t newBps = options_bps[selected_index];
		HttpClient::getInstance()->setBandwidthLimit(newBps);
		//sn->con.AddLog("Bandwidth limit changed to %llu Bps", newBps);
	}
	ImGui::PopItemWidth();

	// Безопасный выбор строки для отображения
	//const char* limitLabel = (currentBps == 0) ? labels[0] : labels[selected_index];
	//ImGui::Text(u8"Текущий лимит: %s", limitLabel);

	ImGui::EndGroup();
	
	// SeparatorText(u8"Рендер");
	// ImGui::Toggle(u8"Отображать частицы", &sn->particles.enabled, toggle_flags, anim_speed);
}