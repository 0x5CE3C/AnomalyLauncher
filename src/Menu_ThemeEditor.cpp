#include "main.h"

float themeFade = 0.0f;
bool isFading = false;
int pendingTheme = 0;

void RenderThemeSelector() {
	ImGui::Text("?? Выбери тему");
	const char* names[] = {
		"Стандартная", "Серо-красная", "Тёмная с акцентом",
		"Красно-чёрная", "?? Аномалия", "?? ПДА", "?? Сеть"
	};
	for (int i = 0; i < 7; i++) {
		if (ImGui::Button(names[i])) SetThemeWithAnimation(i);
		ImGui::SameLine();
		if (sn->set.theme_type == i && !isFading) ImGui::Text("?");
		else if (isFading && pendingTheme == i) ImGui::Text("??");
	}
}

void RenderThemeAndFontMenu() {
	ImGui::Text(u8"?? Тема интерфейса");
	const char* themeNames[] = {
		u8"Стандартная", u8"Серо-красная", u8"Тёмная с акцентом",
		u8"Красно-чёрная", u8"?? Аномалия", u8"?? ПДА", u8"?? Сеть"
	};

	for (int i = 0; i < 7; i++) {
		if (ImGui::Button(themeNames[i])) {
			SetThemeWithAnimation(i);
		}
		ImGui::SameLine();
		if (sn->set.theme_type == i && !isFading) ImGui::Text("Apply");
		else if (isFading && pendingTheme == i) ImGui::Text("Updated");
	}
}

void Initialize() {	
	ApplyCurrentTheme(); // Применяем тему
}

void ImGuiMenuTheme_GrayRed();
void ImGuiMenuTheme_DarkAccent();
void ImGuiMenuTheme_RedBlack();
void ImGuiMenuTheme_Anomaly();
void ImGuiMenuTheme_PDA();
void ImGuiMenuTheme_Cyber();
void ImGuiMenuTheme();

void ApplyCurrentTheme() {
    switch (sn->set.theme_type) {
    case 1: ImGuiMenuTheme_GrayRed(); break;
    case 2: ImGuiMenuTheme_DarkAccent(); break;
    case 3: ImGuiMenuTheme_RedBlack(); break;
    case 4: ImGuiMenuTheme_Anomaly(); break;
    case 5: ImGuiMenuTheme_PDA(); break;
    case 6: ImGuiMenuTheme_Cyber(); break;
    default: ImGuiMenuTheme(); break; // 0
    }
}

void SetThemeWithAnimation(int themeId) {
    pendingTheme = themeId;
    isFading = true;
    themeFade = 0.0f;
}


///////// THEMES
void ImGuiMenuTheme()
{
	ImGuiStyle& Style = ImGui::GetStyle();
	auto Color = Style.Colors;

	Style.WindowMinSize = ImVec2(762, 426);

	Style.WindowTitleAlign = ImVec2(0.50f, 0.50f);
	Style.FrameRounding = 0.0;
	Style.WindowPadding = ImVec2(2, 2);
	Style.WindowRounding = 0.5f;
	Style.ItemSpacing = ImVec2(1, 2);
	Style.ItemInnerSpacing = ImVec2(2, 0);
	//style.IndentSpacing = 25.0f;
	Style.ScrollbarSize = 15.0f;
	Style.ScrollbarRounding = 9.0f;
	Style.FramePadding = ImVec2(2, 1);

	//borders
	Style.WindowBorderSize = 3.3f;//0.3f
	Style.ChildBorderSize = 1.3f;
	Style.FrameBorderSize = 1.3f;
	Style.PopupBorderSize = 1.3f;
	Style.TabBorderSize = 1.3f;


	// Монохромные цвета
	Color[ImGuiCol_WindowBg] = ImColor(30, 30, 30, 200);
	Color[ImGuiCol_FrameBg] = ImColor(40, 40, 40, 128);
	Color[ImGuiCol_FrameBgActive] = ImColor(50, 50, 50, 255);
	Color[ImGuiCol_FrameBgHovered] = ImColor(50, 50, 50, 255);

	Color[ImGuiCol_Button] = ImColor(70, 70, 70, 255);
	Color[ImGuiCol_ButtonActive] = ImColor(100, 100, 100, 255);
	Color[ImGuiCol_ButtonHovered] = ImColor(90, 90, 90, 255);

	Color[ImGuiCol_Tab] = ImColor(80, 80, 80, 200);
	Color[ImGuiCol_TabActive] = ImColor(150, 150, 150, 200);
	Color[ImGuiCol_TabHovered] = ImColor(130, 130, 130, 200);

	Color[ImGuiCol_Border] = ImColor(100, 100, 100, 255);
	Color[ImGuiCol_Separator] = ImColor(100, 100, 100, 255);

	Color[ImGuiCol_ResizeGrip] = ImColor(80, 80, 80, 200);
	Color[ImGuiCol_ResizeGripActive] = ImColor(50, 50, 50, 255);
	Color[ImGuiCol_ResizeGripHovered] = ImColor(50, 50, 50, 255);

	Color[ImGuiCol_ChildBg] = ImColor(40, 40, 40, 64);

	Color[ImGuiCol_ScrollbarBg] = ImColor(50, 50, 50, 255);
	Color[ImGuiCol_ScrollbarGrab] = ImColor(70, 70, 70, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(90, 90, 90, 255);
	Color[ImGuiCol_ScrollbarGrabHovered] = ImColor(70, 70, 70, 255);

	Color[ImGuiCol_Header] = ImColor(40, 40, 40, 255);
	Color[ImGuiCol_HeaderActive] = ImColor(100, 100, 100, 255);
	Color[ImGuiCol_HeaderHovered] = ImColor(100, 100, 100, 255);
	Color[ImGuiCol_CheckMark] = ImColor(255, 255, 255, 255);
	Color[ImGuiCol_PlotHistogram] = ImColor(50, 50, 50, 255);
}


void ImGuiMenuTheme_GrayRed()
{
	ImGuiStyle& Style = ImGui::GetStyle();
	auto Color = Style.Colors;

	Style.WindowMinSize = ImVec2(762, 426);
	Style.WindowTitleAlign = ImVec2(0.50f, 0.50f);
	Style.FrameRounding = 0.0;
	Style.WindowPadding = ImVec2(2, 2);
	Style.WindowRounding = 0.5f;
	Style.ItemSpacing = ImVec2(1, 2);
	Style.ItemInnerSpacing = ImVec2(2, 0);
	Style.ScrollbarSize = 15.0f;
	Style.ScrollbarRounding = 9.0f;
	Style.FramePadding = ImVec2(2, 1);

	// Borders
	Style.WindowBorderSize = 3.3f;
	Style.ChildBorderSize = 1.3f;
	Style.FrameBorderSize = 1.3f;
	Style.PopupBorderSize = 1.3f;
	Style.TabBorderSize = 1.3f;

	// Цвета
	Color[ImGuiCol_WindowBg] = ImColor(40, 40, 40, 200);
	Color[ImGuiCol_FrameBg] = ImColor(60, 60, 60, 128);
	Color[ImGuiCol_FrameBgActive] = ImColor(80, 0, 0, 255);
	Color[ImGuiCol_FrameBgHovered] = ImColor(80, 0, 0, 255);

	Color[ImGuiCol_Button] = ImColor(90, 0, 0, 255);
	Color[ImGuiCol_ButtonActive] = ImColor(120, 0, 0, 255);
	Color[ImGuiCol_ButtonHovered] = ImColor(110, 0, 0, 255);

	Color[ImGuiCol_Tab] = ImColor(100, 0, 0, 200);
	Color[ImGuiCol_Border] = ImColor(120, 0, 0, 255);
	Color[ImGuiCol_Separator] = ImColor(120, 0, 0, 255);

	Color[ImGuiCol_ResizeGrip] = ImColor(100, 0, 0, 200);
	Color[ImGuiCol_ResizeGripActive] = ImColor(80, 0, 0, 255);
	Color[ImGuiCol_ResizeGripHovered] = ImColor(80, 0, 0, 255);

	Color[ImGuiCol_ChildBg] = ImColor(60, 60, 60, 64);
	Color[ImGuiCol_ScrollbarBg] = ImColor(70, 70, 70, 255);
	Color[ImGuiCol_ScrollbarGrab] = ImColor(90, 0, 0, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(110, 0, 0, 255);
	Color[ImGuiCol_ScrollbarGrabHovered] = ImColor(90, 0, 0, 255);

	Color[ImGuiCol_Header] = ImColor(60, 60, 60, 255);
	Color[ImGuiCol_HeaderActive] = ImColor(60, 60, 60, 255);
	Color[ImGuiCol_HeaderHovered] = ImColor(60, 60, 60, 255);
	Color[ImGuiCol_CheckMark] = ImColor(255, 0, 0, 255);
	Color[ImGuiCol_PlotHistogram] = ImColor(80, 0, 0, 255);
}

void ImGuiMenuTheme_DarkAccent()
{
	ImGuiStyle& Style = ImGui::GetStyle();
	auto Color = Style.Colors;

	Style.WindowMinSize = ImVec2(762, 426);
	Style.WindowTitleAlign = ImVec2(0.50f, 0.50f);
	Style.FrameRounding = 0.0;
	Style.WindowPadding = ImVec2(2, 2);
	Style.WindowRounding = 0.5f;
	Style.ItemSpacing = ImVec2(1, 2);
	Style.ItemInnerSpacing = ImVec2(2, 0);
	Style.ScrollbarSize = 15.0f;
	Style.ScrollbarRounding = 9.0f;
	Style.FramePadding = ImVec2(2, 1);

	// Borders
	Style.WindowBorderSize = 3.3f;
	Style.ChildBorderSize = 1.3f;
	Style.FrameBorderSize = 1.3f;
	Style.PopupBorderSize = 1.3f;
	Style.TabBorderSize = 1.3f;

	// Цвета
	Color[ImGuiCol_WindowBg] = ImColor(25, 25, 25, 200);
	Color[ImGuiCol_FrameBg] = ImColor(45, 0, 0, 128);
	Color[ImGuiCol_FrameBgActive] = ImColor(70, 0, 0, 255);
	Color[ImGuiCol_FrameBgHovered] = ImColor(70, 0, 0, 255);

	Color[ImGuiCol_Button] = ImColor(80, 0, 0, 255);
	Color[ImGuiCol_ButtonActive] = ImColor(120, 0, 0, 255);
	Color[ImGuiCol_ButtonHovered] = ImColor(100, 0, 0, 255);

	Color[ImGuiCol_Tab] = ImColor(90, 0, 0, 200);
	Color[ImGuiCol_Border] = ImColor(120, 0, 0, 255);
	Color[ImGuiCol_Separator] = ImColor(120, 0, 0, 255);

	Color[ImGuiCol_ResizeGrip] = ImColor(90, 0, 0, 200);
	Color[ImGuiCol_ResizeGripActive] = ImColor(70, 0, 0, 255);
	Color[ImGuiCol_ResizeGripHovered] = ImColor(70, 0, 0, 255);

	Color[ImGuiCol_ChildBg] = ImColor(150, 50, 50, 64);
	Color[ImGuiCol_ScrollbarBg] = ImColor(60, 60, 60, 255);
	Color[ImGuiCol_ScrollbarGrab] = ImColor(80, 0, 0, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(100, 0, 0, 255);
	Color[ImGuiCol_ScrollbarGrabHovered] = ImColor(80, 0, 0, 255);

	Color[ImGuiCol_Header] = ImColor(50, 0, 0, 255);
	Color[ImGuiCol_HeaderActive] = ImColor(50, 0, 0, 255);
	Color[ImGuiCol_HeaderHovered] = ImColor(50, 0, 0, 255);
	Color[ImGuiCol_CheckMark] = ImColor(255, 0, 0, 255);
	Color[ImGuiCol_PlotHistogram] = ImColor(90, 0, 0, 255);
}

void ImGuiMenuTheme_RedBlack()
{
	ImGuiStyle& Style = ImGui::GetStyle();
	auto Color = Style.Colors;

	Style.WindowMinSize = ImVec2(762, 426);
	Style.WindowTitleAlign = ImVec2(0.50f, 0.50f);
	Style.FrameRounding = 0.0;
	Style.WindowPadding = ImVec2(2, 2);
	Style.WindowRounding = 0.5f;
	Style.ItemSpacing = ImVec2(1, 2);
	Style.ItemInnerSpacing = ImVec2(2, 0);
	Style.ScrollbarSize = 15.0f;
	Style.ScrollbarRounding = 9.0f;
	Style.FramePadding = ImVec2(2, 1);

	// Borders
	Style.WindowBorderSize = 3.3f;
	Style.ChildBorderSize = 1.3f;
	Style.FrameBorderSize = 1.3f;
	Style.PopupBorderSize = 1.3f;
	Style.TabBorderSize = 1.3f;

	// Цвета
	Color[ImGuiCol_WindowBg] = ImColor(20, 20, 20, 200);
	Color[ImGuiCol_FrameBg] = ImColor(40, 0, 0, 128);
	Color[ImGuiCol_FrameBgActive] = ImColor(60, 0, 0, 255);
	Color[ImGuiCol_FrameBgHovered] = ImColor(60, 0, 0, 255);

	Color[ImGuiCol_Button] = ImColor(70, 0, 0, 255);
	Color[ImGuiCol_ButtonActive] = ImColor(100, 0, 0, 255);
	Color[ImGuiCol_ButtonHovered] = ImColor(90, 0, 0, 255);

	Color[ImGuiCol_Tab] = ImColor(80, 0, 0, 200);
	Color[ImGuiCol_Border] = ImColor(100, 0, 0, 255);
	Color[ImGuiCol_Separator] = ImColor(100, 0, 0, 255);

	Color[ImGuiCol_ResizeGrip] = ImColor(80, 0, 0, 200);
	Color[ImGuiCol_ResizeGripActive] = ImColor(60, 0, 0, 255);
	Color[ImGuiCol_ResizeGripHovered] = ImColor(60, 0, 0, 255);

	Color[ImGuiCol_ChildBg] = ImColor(40, 0, 0, 64);
	Color[ImGuiCol_ScrollbarBg] = ImColor(50, 0, 0, 255);
	Color[ImGuiCol_ScrollbarGrab] = ImColor(70, 0, 0, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(90, 0, 0, 255);
	Color[ImGuiCol_ScrollbarGrabHovered] = ImColor(70, 0, 0, 255);

	Color[ImGuiCol_Header] = ImColor(40, 0, 0, 255);
	Color[ImGuiCol_HeaderActive] = ImColor(40, 0, 0, 255);
	Color[ImGuiCol_HeaderHovered] = ImColor(40, 0, 0, 255);
	Color[ImGuiCol_CheckMark] = ImColor(255, 0, 0, 255);
	Color[ImGuiCol_PlotHistogram] = ImColor(50, 0, 0, 255);
}

void ImGuiMenuTheme_Anomaly() {
	static float t = 0.0f;
	t += ImGui::GetIO().DeltaTime * 3.0f;

	float r = 0.4f + 0.1f * sin(t * 1.1f);
	float g = 0.1f + 0.05f * sin(t * 0.7f);
	float b = 0.1f + 0.05f * cos(t * 0.9f);
	float a = 190.0f / 255.0f; // Прозрачность: 190 из 255 ? ~0.745

	auto& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_WindowBg] = ImVec4(r, g, b, a);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(80 / 255.0f, 10 / 255.0f, 10 / 255.0f, 128 / 255.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(120 / 255.0f, 20 / 255.0f, 20 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(100 / 255.0f, 15 / 255.0f, 15 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(140 / 255.0f, 20 / 255.0f, 20 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(180 / 255.0f, 30 / 255.0f, 30 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(160 / 255.0f, 25 / 255.0f, 25 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(100 / 255.0f, 15 / 255.0f, 15 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(120 / 255.0f, 20 / 255.0f, 20 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(100 / 255.0f, 15 / 255.0f, 15 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_Text] = ImVec4(255 / 255.0f, 220 / 255.0f, 220 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(150 / 255.0f, 30 / 255.0f, 30 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(150 / 255.0f, 30 / 255.0f, 30 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(255 / 255.0f, 100 / 255.0f, 100 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(60 / 255.0f, 10 / 255.0f, 10 / 255.0f, 64 / 255.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(80 / 255.0f, 10 / 255.0f, 10 / 255.0f, 255 / 255.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(120 / 255.0f, 20 / 255.0f, 20 / 255.0f, 255 / 255.0f);

	style.WindowRounding = 4.0f;
	style.FrameRounding = 2.0f;
	style.ScrollbarRounding = 7.0f;
}


void ImGuiMenuTheme_PDA() {
	static float scan = 0.0f;
	scan += ImGui::GetIO().DeltaTime * 10.0f;
	if (scan > 100.0f) scan = 0.0f;

	auto& style = ImGui::GetStyle();

	// Зелёно-чёрная палитра
	style.Colors[ImGuiCol_WindowBg] = ImColor(0, 40, 0, 220);
	style.Colors[ImGuiCol_FrameBg] = ImColor(0, 60, 0, 160);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(0, 100, 0, 255);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(0, 80, 0, 255);

	style.Colors[ImGuiCol_Button] = ImColor(0, 70, 0, 255);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(0, 110, 0, 255);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(0, 90, 0, 255);

	style.Colors[ImGuiCol_Text] = ImColor(0, 255, 0, 255);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(0, 120, 0, 255);
	style.Colors[ImGuiCol_Border] = ImColor(0, 100, 0, 255);
	style.Colors[ImGuiCol_Separator] = ImColor(0, 80, 0, 255);
	style.Colors[ImGuiCol_CheckMark] = ImColor(0, 255, 0, 255);

	style.Colors[ImGuiCol_ChildBg] = ImColor(0, 30, 0, 64);
	style.Colors[ImGuiCol_ScrollbarBg] = ImColor(0, 50, 0, 255);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(0, 90, 0, 255);

	style.WindowRounding = 1.0f;
	style.FrameRounding = 1.0f;
	style.ScrollbarRounding = 0.0f;

	// Эффект сканирования (горизонтальная линия)
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
	if (ImGui::GetCurrentWindow()->DrawList->_CmdHeader.ClipRect.x == 0) {
		float y = fmodf(scan, ImGui::GetWindowHeight());
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(0, y),
			ImVec2(ImGui::GetWindowWidth(), y),
			ImGui::GetColorU32(ImVec4(0, 255, 0, 40)), 1.0f
		);
	}
	ImGui::PopStyleVar();
}

void ImGuiMenuTheme_Cyber() {
	static float t = 0.0f;
	t += ImGui::GetIO().DeltaTime * 2.0f;

	auto& style = ImGui::GetStyle();

	// Тёмно-синий с неоновым акцентом
	style.Colors[ImGuiCol_WindowBg] = ImColor(10, 10, 30, 210);
	style.Colors[ImGuiCol_FrameBg] = ImColor(30, 30, 60, 140);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(60, 60, 120, 255);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(50, 50, 100, 255);

	style.Colors[ImGuiCol_Button] = ImColor(50, 50, 150, 255);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(80, 80, 200, 255);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(100, 100, 255, 255);

	style.Colors[ImGuiCol_Text] = ImColor(100, 100, 255, 255);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(50, 50, 100, 180);
	style.Colors[ImGuiCol_Border] = ImColor(80, 80, 180, 200);
	style.Colors[ImGuiCol_Separator] = ImColor(70, 70, 150, 200);
	style.Colors[ImGuiCol_CheckMark] = ImColor(120, 120, 255, 255);

	style.Colors[ImGuiCol_ChildBg] = ImColor(20, 20, 50, 64);
	style.Colors[ImGuiCol_ScrollbarBg] = ImColor(40, 40, 80, 255);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(60, 60, 140, 255);

	style.WindowRounding = 6.0f;
	style.FrameRounding = 4.0f;
	style.ScrollbarRounding = 6.0f;

	// Бегущие вертикальные линии — эффект "сети"
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
	for (int x = (int)fmodf(t * 20, 20); x < ImGui::GetWindowWidth(); x += 20) {
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(x, 0),
			ImVec2(x, ImGui::GetWindowHeight()),
			ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 1.0f, 0.1f + 0.1f * sin(t + x * 0.1f))), 1.0f
		);
	}
	ImGui::PopStyleVar();
}