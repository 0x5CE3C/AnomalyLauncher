#pragma once

// структура дл€ функций, имеющих только включение/вылючение и гор€чие клавиши
struct func 
{
	bool enabled;
    int active;	
};

struct stsn
{		
	Console con;

	//lparams
	func dbg;
	func no_occq;
	func noramtex;
	func nes_texture_storing;
	func noprefetch;
	func prefetch_sounds;
	func c60hz;
	func demomode;
	func nosound;
	func nointro;
	func skiplogo;
	func debug_ge;
	func no_dialog_header;
	func avx_support;
	func devdbg;

	func smap;		


	func my_radio;
	int radio_reconnect;
	int radio_volume;
	int radio_station;

	func auto_check_launcher_update;	

	func download_system;

	func dbg_mode;	

	func playbackgroundtheme;
	int background_music_volume;
	
	func bmenu_item_nav;
	func bmenu_select;
	func bmenu_nav;
	func bmenu_log_message;
	func bmenu_file_download;

	int menu_item_nav_volume;
	int menu_select_volume;
	int menu_nav_volume;
	int menu_log_message_volume;
	int menu_file_download_volume;

	func support_pz_mode; // включение режима поддержки сборки purge of zone

	func buseMO2;

	func scanhashes;
	func scanhashes_dirs;

	func auto_update_filelist;

	struct stumode // режим универсальности - дл€ поддержки других сборок. 0 - PZ, 1 - MP
	{
		func universal_mode; // вкл / выкл
		int project_mode; // номер режима
		std::string modpack_name; // название сборки
		std::string lastupdates_url; // статическа€ ссылка на скачивание файла со списком последний обновлений и изменений в сборке при запуске.
		std::string filelist_url; // статическа€ ссылка на скачивание списка файлов при запуске.		
	}umode;
	
	func auto_update_changelist;

	stsettings set;

	int currentTab;

	func filter_only_extractor;
	func filter_only_anomaly;
	func filter_only_mo2;
	func filter_only_mp;

	func disable_update_modpack_button;

	std::string latest_temp_ver_mp;

	func particles;	
};
