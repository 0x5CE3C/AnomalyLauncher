#include "main.h"

char	ini_filename[512];

int Save()
{
	traceLastFunc("int Save();");

	_snprintf(ini_filename, sizeof(ini_filename), "%s\\%s\\%s", GetExePath().c_str(), PROJECT_FOLDER, INI_FILE);

	INI::File ini;
	// Loading from file	
	if (!ini.Load(ini_filename))
	{
		// Loading from stream
		std::ifstream op(ini_filename, std::ios::in);
		if (op.is_open())
			op >> ini;
	}	

		ini.GetSection("general")->SetValue("render_type", sn->set.render_type);
		ini.GetSection("general")->SetValue("smap_type", sn->set.shadow_map_type);
		//================= LAUNCH MODES ======================
		ini.GetSection("general")->SetValue("block60hz_enabled", sn->c60hz.enabled);
		ini.GetSection("general")->SetValue("bnointro_enabled", sn->nointro.enabled);
		ini.GetSection("general")->SetValue("bnoprefetch_enabled", sn->noprefetch.enabled);
		ini.GetSection("general")->SetValue("bnoramtex_enabled", sn->noramtex.enabled);
		ini.GetSection("general")->SetValue("bnosound_enabled", sn->nosound.enabled);
		ini.GetSection("general")->SetValue("bno_occq_enabled", sn->no_occq.enabled);
		ini.GetSection("general")->SetValue("bprefetch_sounds_enabled", sn->prefetch_sounds.enabled);
		ini.GetSection("general")->SetValue("bskiplogo_enabled", sn->skiplogo.enabled);
   

		//================= DEBUG MODES ======================
	   ini.GetSection("debug")->SetValue("bdbg_enabled", sn->dbg.enabled);
	   ini.GetSection("debug")->SetValue("bdbgdev_enabled", sn->devdbg.enabled);	  
	   ini.GetSection("debug")->SetValue("bdebug_ge_enabled", sn->debug_ge.enabled);
	   ini.GetSection("debug")->SetValue("bdemomode_enabled", sn->demomode.enabled);
	   ini.GetSection("debug")->SetValue("bnes_texture_storing_enabled", sn->nes_texture_storing.enabled);
      	
	   //================= MODULES MODES ======================
	   ini.GetSection("modules")->SetValue("download_system", sn->download_system.enabled);   
	   ini.GetSection("modules")->SetValue("scanhashes_enabled", sn->scanhashes.enabled);
	   ini.GetSection("modules")->SetValue("scanhashes_dirs_enabled", sn->scanhashes_dirs.enabled);
	    ini.GetSection("modules")->SetValue("modpack_version", sn->set.modpack_version);
	   
       ini.GetSection("settings")->SetValue("buseMO2_enabled", sn->buseMO2.enabled);
	   ini.GetSection("settings")->SetValue("playbackgroundtheme_enabled", sn->playbackgroundtheme.enabled);
	   ini.GetSection("settings")->SetValue("background_music_volume", sn->background_music_volume);

	   ini.GetSection("settings")->SetValue("bmenu_file_download_enabled", sn->bmenu_file_download.enabled);
	   ini.GetSection("settings")->SetValue("menu_file_download_volume", sn->menu_file_download_volume);

	    ini.GetSection("settings")->SetValue("bmenu_item_nav_enabled", sn->bmenu_item_nav.enabled);
	    ini.GetSection("settings")->SetValue("menu_item_nav_volume", sn->menu_item_nav_volume);

	    ini.GetSection("settings")->SetValue("bmenu_log_message_enabled", sn->bmenu_log_message.enabled);
	    ini.GetSection("settings")->SetValue("menu_log_message_volume", sn->menu_log_message_volume);

	    ini.GetSection("settings")->SetValue("bmenu_nav_enabled", sn->bmenu_nav.enabled);
		ini.GetSection("settings")->SetValue("menu_nav_volume", sn->menu_nav_volume);

		ini.GetSection("settings")->SetValue("bmenu_select_enabled", sn->bmenu_select.enabled);
		ini.GetSection("settings")->SetValue("menu_select_volume", sn->menu_select_volume);


	   ini.GetSection("settings")->SetValue("auto_check_launcher_update_enabled", sn->auto_check_launcher_update.enabled);
	   
	   ini.GetSection("settings")->SetValue("auto_update_filelist_enabled", sn->auto_update_filelist.enabled);
	   /*ini.GetSection("settings")->SetValue("auto_update_changelist_enabled", sn->auto_update_changelist.enabled);*/
	   
	   //Возможный Автономный режим "без интернета".
	   ini.GetSection("modpack")->SetValue("latest_temp_ver_mp", sn->latest_temp_ver_mp, "временное хранилище версии сборки");

	   ini.GetSection("modpack")->SetValue("umode.universal_mode_enabled", sn->umode.universal_mode.enabled);
	   string mpname_cp1251 = utf8_to_cp1251(sn->umode.modpack_name);
	   ini.GetSection("modpack")->SetValue("umode.modpack_name", mpname_cp1251, "название профиля вашей сборки в Mod Organizer 2");
	   ini.GetSection("modpack")->SetValue("umode.filelist_url", sn->umode.filelist_url, "ссылка на список файлов");
	   ini.GetSection("modpack")->SetValue("umode.lastupdates_url", sn->umode.lastupdates_url, "ссылка на список последних изменений");
	   ini.GetSection("modpack")->SetValue("disable_update_modpack_button_enabled", sn->disable_update_modpack_button.enabled, "Отключение кнопки \"Обновить сборку\"");
	  

	   ini.GetSection("settings")->SetValue("httpclient_limit_speed_type", sn->set.httpclient_limit_speed_type, "Тип скорости: 0 - Без лимита, 1 - 1 МБ/с, 2 - 2 МБ/с, 3 - 5 МБ/с, 4 - 10 МБ/с и т.д.");

	    /*ini.GetSection("style")->SetValue("theme_type", sn->set.theme_type);
	    ini.GetSection("style")->SetValue("font_size", sn->set.font_size);
	    ini.GetSection("style")->SetValue("custom_font", sn->set.custom_font);*/

	   ini.GetSection("filelistgenerator")->SetValue("filter_only_extractor_enabled", sn->filter_only_extractor.enabled);
	   ini.GetSection("filelistgenerator")->SetValue("filter_only_anomaly_enabled", sn->filter_only_anomaly.enabled);
	   ini.GetSection("filelistgenerator")->SetValue
	   ("filter_only_mo2_enabled", sn->filter_only_mo2.enabled);
	   ini.GetSection("filelistgenerator")->SetValue
	   ("filter_only_mp_enabled", sn->filter_only_mp.enabled);


		ini.Save(ini_filename);
		sn->con.AddLog("{AAFFFF}[Настройки]: Параметры сохранены успешно!");
	return 0;
};

int Load() 
{
	traceLastFunc("int Load()");

	_snprintf(ini_filename, sizeof(ini_filename), "%s\\%s\\%s", GetExePath().c_str(), PROJECT_FOLDER, INI_FILE);

	INI::File ini;
	// Loading from file	
	if (!ini.Load(ini_filename))
	{
		// Loading from stream
		std::ifstream op(ini_filename, std::ios::in);
		if (op.is_open())
			op >> ini;
	}
						
		sn->set.render_type = ini.GetSection("general")->GetValue("render_type", 0).AsInt();
		sn->set.shadow_map_type = ini.GetSection("general")->GetValue("smap_type", 0).AsInt();
		//================= LAUNCH MODES ======================
		sn->c60hz.enabled = ini.GetSection("general")->GetValue("block60hz_enabled", false).AsBool();
		sn->nointro.enabled = ini.GetSection("general")->GetValue("bnointro_enabled", false).AsBool();
		sn->noprefetch.enabled = ini.GetSection("general")->GetValue("bnoprefetch_enabled", false).AsBool();
		sn->noramtex.enabled = ini.GetSection("general")->GetValue("bnoramtex_enabled", false).AsBool();
		sn->nosound.enabled = ini.GetSection("general")->GetValue("bnosound_enabled", false).AsBool();
		sn->no_occq.enabled = ini.GetSection("general")->GetValue("bno_occq_enabled", false).AsBool();
		sn->prefetch_sounds.enabled = ini.GetSection("general")->GetValue("bprefetch_sounds_enabled", false).AsBool();
		sn->skiplogo.enabled = ini.GetSection("general")->GetValue("bskiplogo_enabled", false).AsBool();
		//======================================================
		sn->dbg.enabled = ini.GetSection("debug")->GetValue("bdbg_enabled", false).AsBool();

		sn->devdbg.enabled = ini.GetSection("debug")->GetValue("bdbgdev_enabled", false).AsBool();		

		sn->debug_ge.enabled = ini.GetSection("debug")->GetValue("bdebug_ge_enabled", false).AsBool();
		sn->demomode.enabled = ini.GetSection("debug")->GetValue("bdemomode_enabled", false).AsBool();
		sn->nes_texture_storing.enabled = ini.GetSection("debug")->GetValue("bnes_texture_storing_enabled", false).AsBool();

		sn->download_system.enabled = ini.GetSection("modules")->GetValue("download_system", true).AsBool();
		sn->scanhashes.enabled = ini.GetSection("modules")->GetValue("scanhashes_enabled", false).AsBool();
		sn->scanhashes_dirs.enabled = ini.GetSection("modules")->GetValue("scanhashes_dirs_enabled", false).AsBool();

		sn->set.modpack_version = ini.GetSection("modules")->GetValue("modpack_version", "0.0.0.1").AsString();

		sn->buseMO2.enabled = ini.GetSection("settings")->GetValue("buseMO2_enabled", false).AsBool();

		sn->playbackgroundtheme.enabled = ini.GetSection("settings")->GetValue("playbackgroundtheme_enabled", true).AsBool();
		sn->background_music_volume = ini.GetSection("settings")->GetValue("background_music_volume", 50).AsInt();

		sn->bmenu_file_download.enabled = ini.GetSection("settings")->GetValue("bmenu_file_download_enabled", true).AsBool();
		sn->menu_file_download_volume = ini.GetSection("settings")->GetValue("menu_file_download_volume", 50).AsInt();

		sn->bmenu_item_nav.enabled = ini.GetSection("settings")->GetValue("bmenu_item_nav_enabled", true).AsBool();
		sn->menu_item_nav_volume = ini.GetSection("settings")->GetValue("menu_item_nav_volume", 50).AsInt();

		sn->bmenu_log_message.enabled = ini.GetSection("settings")->GetValue("bmenu_log_message_enabled", true).AsBool();
		sn->menu_log_message_volume = ini.GetSection("settings")->GetValue("menu_log_message_volume", 50).AsInt();

		sn->bmenu_nav.enabled = ini.GetSection("settings")->GetValue("bmenu_nav_enabled", true).AsBool();
		sn->menu_nav_volume = ini.GetSection("settings")->GetValue("menu_nav_volume", 50).AsInt();

		sn->bmenu_select.enabled = ini.GetSection("settings")->GetValue("bmenu_select_enabled", true).AsBool();
		sn->menu_select_volume = ini.GetSection("settings")->GetValue("menu_select_volume", 50).AsInt();

		sn->auto_check_launcher_update.enabled = ini.GetSection("settings")->GetValue("auto_check_launcher_update_enabled", false).AsBool();
		sn->auto_update_filelist.enabled = ini.GetSection("settings")->GetValue("auto_update_filelist_enabled", true).AsBool();
		//sn->auto_update_changelist.enabled = ini.GetSection("settings")->GetValue("auto_update_changelist_enabled", true).AsBool();


		sn->umode.universal_mode.enabled = ini.GetSection("modpack")->GetValue("umode.universal_mode_enabled", false).AsBool();

		string mpname_utf8 = cp1251_to_utf8(ini.GetSection("modpack")->GetValue("umode.modpack_name", "Purge of Zone").AsString());

		sn->umode.modpack_name = mpname_utf8;
		sn->umode.filelist_url = ini.GetSection("modpack")->GetValue("umode.filelist_url", "URL").AsString();
		sn->umode.lastupdates_url = ini.GetSection("modpack")->GetValue("umode.lastupdates_url", "URL").AsString();

		sn->disable_update_modpack_button.enabled = ini.GetSection("modpack")->GetValue("disable_update_modpack_button_enabled", true).AsBool();

		sn->latest_temp_ver_mp = ini.GetSection("modpack")->GetValue("latest_temp_ver_mp", "unknown").AsString();

		sn->set.httpclient_limit_speed_type = ini.GetSection("settings")->GetValue("httpclient_limit_speed_type", 7).AsInt();

		/*sn->set.theme_type = ini.GetSection("style")->GetValue("theme_type", 0).AsInt();
		sn->set.font_size = ini.GetSection("style")->GetValue("font_size", 17).AsInt();
		sn->set.custom_font = ini.GetSection("style")->GetValue("custom_font", true).AsBool();*/

		sn->filter_only_extractor.enabled = ini.GetSection("filelistgenerator")->GetValue("filter_only_extractor_enabled", false).AsBool();
		sn->filter_only_anomaly.enabled = ini.GetSection("filelistgenerator")->GetValue("filter_only_anomaly_enabled", false).AsBool();
		sn->filter_only_mo2.enabled = ini.GetSection("filelistgenerator")->GetValue
		  ("filter_only_mo2_enabled", false).AsBool();
		sn->filter_only_mp.enabled = ini.GetSection("filelistgenerator")->GetValue
		  ("filter_only_mp_enabled", false).AsBool();

		if (fsm->FileExist(ini_filename) == 0)
		{
			sn->con.AddLog("{FF0000}[Настройки]: Файл AnomalyLauncher.ini не найден! создаю новый...");
			Save();
		}
		else
		{
			sn->con.AddLog("{AAFFFF}[Настройки]: Файл AnomalyLauncher.ini обнаружен, параметры загружены успешно!");
		};
	return 0;
};