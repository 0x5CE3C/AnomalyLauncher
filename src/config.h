#pragma once
struct stsettings
{		
	int display_resolutions_type;
	int render_type;
	int support_AVX;
	int debug_mode;
	int shadow_map_type;
	int sound_problem_workaround;
	int prefetch_sounds;
	std::string launcher_version;

	std::string modpack_version;

	int httpclient_limit_speed_type; // limit incoming speed

	int theme_type = 0;      // 0–6: тип темы
	int font_size = 16;      // Размер шрифта
	bool custom_font = false; // Использовать кастомный шрифт
};

int Load();
int Save();

extern char	ini_filename[512];

//sn->set.theme_type = general->GetValue("theme_type", 0).AsInt();
//sn->set.font_size = general->GetValue("font_size", 16).AsInt();
//sn->set.custom_font = general->GetValue("custom_font", true).AsBool();