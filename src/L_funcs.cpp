#include "main.h"
#include "L_funcs.h"

HCHANNEL menu_theme;
static HSAMPLE smpl;

static bool func_nnabler_init = true;
float amplitude = 0.0f;
void funcs_enabler()
{
	if (func_nnabler_init)
	{		
		sn->con.AddLog("{AAFFBB}[info]: Запуск основных функций...");		
		func_nnabler_init = 0;
	}			


	GetRenderer();
	GetLaunchparams();
		

	static bool restore_extractor_by_memory = false;
	bool restore_extractor_enabled = true;

	if (restore_extractor_enabled)
	{
		if (!restore_extractor_by_memory)
		{
			sn->con.AddLog("[info]: Проверка существования экстрактора запущена");
			restore_extractor_by_memory = 1;
		}
		
		int createpfolder = 0;

		std::string pfolder = GetExePath() + "\\" + PROJECT_FOLDER;
		if (fsm->FolderExist(pfolder.c_str()))
		{
			///sn->con.AddLog(u8"{00BBBB}папка для обновлений найдена: %s", pfolder.c_str());
			if (fsm->FileExist(zipPath.c_str()) == false)
			{
				sn->con.AddLog("[info]: Экстрактор не обнаружен, восстановление из памяти Лаунчера");
				ExtractRes(IDR_EXTRACTOR1, _T("EXTRACTOR"), zipPath.c_str());
				restore_extractor_enabled = false;
			}
			else
			{
				//sn->con.AddLog("[info]: Экстрактор обнаружен, пропуск восстановления");
				restore_extractor_enabled = false;
			}
		}
		else
		{
			sn->con.AddLog("[info]:{FF0000}папка для конфигураций не найдена!");
			if ((!fsm->FolderExist(pfolder.c_str())))
			{
				fsm->CreateFolder(pfolder.c_str());
				sn->con.AddLog("[info]:{00BB00}папка для конфигураций создана!");


				if (fsm->FileExist(zipPath.c_str()) == false)
				{
					sn->con.AddLog("[info]: Экстрактор не обнаружен, восстановление из памяти Лаунчера");
					ExtractRes(IDR_EXTRACTOR1, _T("EXTRACTOR"), zipPath.c_str());
					restore_extractor_enabled = false;
				}
				else
				{
					//sn->con.AddLog("[info]: Экстрактор обнаружен, пропуск восстановления");
					restore_extractor_enabled = false;
				}
			}
		}		

		
	}
	else
	{
		if (restore_extractor_by_memory)
		{
			sn->con.AddLog("[info]: Проверка существования экстрактора выключена");
			restore_extractor_by_memory = 0;
		}
	}



	static bool dbg_a = false;
	if (sn->dbg.enabled)
	{
		if (!dbg_a)
		{
			sn->con.AddLog(u8"[info]: Debug Mode Enabled");
			dbg_a = 1;
		}
	}
	else
	{
		if (dbg_a)
		{
			sn->con.AddLog(u8"[info]: Debug Mode Disabled");
			dbg_a = 0;
		}
	}



	static bool no_occq_a = false;
	if (sn->no_occq.enabled)
	{
		if (!no_occq_a)
		{
			sn->con.AddLog(u8"[info]: no_occq Enabled");
			no_occq_a = 1;
		}
	}
	else
	{
		if (no_occq_a)
		{
			sn->con.AddLog(u8"[info]: no_occq Disabled");
			no_occq_a = 0;
		}
	}
		static bool noramtex_a = false;
	if (sn->noramtex.enabled)
	{
		if (!noramtex_a)
		{
			sn->con.AddLog(u8"[info]: noramtex Enabled");
			noramtex_a = 1;
		}
	}
	else
	{
		if (noramtex_a)
		{
			sn->con.AddLog(u8"[info]: noramtex Disabled");
			noramtex_a = 0;
		}
	}
		static bool nes_texture_storing_a = false;
	if (sn->nes_texture_storing.enabled)
	{
		if (!nes_texture_storing_a)
		{
			sn->con.AddLog(u8"[info]: nes_texture_storing Enabled");
			nes_texture_storing_a = 1;
		}
	}
	else
	{
		if (nes_texture_storing_a)
		{
			sn->con.AddLog(u8"[info]: nes_texture_storing Disabled");
			nes_texture_storing_a = 0;
		}
	}
		static bool noprefetch_a = false;
	if (sn->noprefetch.enabled)
	{
		if (!noprefetch_a)
		{
			sn->con.AddLog(u8"[info]: noprefetch Enabled");
			noprefetch_a = 1;
		}
	}
	else
	{
		if (noprefetch_a)
		{
			sn->con.AddLog(u8"[info]: noprefetch Disabled");
			noprefetch_a = 0;
		}
	}
		static bool prefetch_sounds_a = false;
	if (sn->prefetch_sounds.enabled)
	{
		if (!prefetch_sounds_a)
		{
			sn->con.AddLog(u8"[info]: prefetch_sounds Enabled");
			prefetch_sounds_a = 1;
		}
	}
	else
	{
		if (prefetch_sounds_a)
		{
			sn->con.AddLog(u8"[info]: prefetch_sounds Disabled");
			prefetch_sounds_a = 0;
		}
	}
		static bool c60hz_a = false;
	if (sn->c60hz.enabled)
	{
		if (!c60hz_a)
		{
			sn->con.AddLog(u8"[info]: c60hz Enabled");
			c60hz_a = 1;
		}
	}
	else
	{
		if (c60hz_a)
		{
			sn->con.AddLog(u8"[info]: c60hz Disabled");
			c60hz_a = 0;
		}
	}
		static bool demomode_a = false;
	if (sn->demomode.enabled)
	{
		if (!demomode_a)
		{
			sn->con.AddLog(u8"[info]: demomode Enabled");
			demomode_a = 1;
		}
	}
	else
	{
		if (demomode_a)
		{
			sn->con.AddLog(u8"[info]: demomode Disabled");
			demomode_a = 0;
		}
	}
		static bool nosound_a = false;
	if (sn->nosound.enabled)
	{
		if (!nosound_a)
		{
			sn->con.AddLog(u8"[info]: nosound Enabled");
			nosound_a = 1;
		}
	}
	else
	{
		if (nosound_a)
		{
			sn->con.AddLog(u8"[info]: nosound Disabled");
			nosound_a = 0;
		}
	}
		static bool nointro_a = false;
	if (sn->nointro.enabled)
	{
		if (!nointro_a)
		{
			sn->con.AddLog(u8"[info]: nointro Enabled");
			nointro_a = 1;
		}
	}
	else
	{
		if (nointro_a)
		{
			sn->con.AddLog(u8"[info]: nointro Disabled");
			nointro_a = 0;
		}
	}
		static bool skiplogo_a = false;
	if (sn->skiplogo.enabled)
	{
		if (!skiplogo_a)
		{
			sn->con.AddLog(u8"[info]: skiplogo Enabled");
			skiplogo_a = 1;
		}
	}
	else
	{
		if (skiplogo_a)
		{
			sn->con.AddLog(u8"[info]: skiplogo Disabled");
			skiplogo_a = 0;
		}
	}
		static bool debug_ge_a = false;
	if (sn->debug_ge.enabled)
	{
		if (!debug_ge_a)
		{
			sn->con.AddLog(u8"[info]: debug_ge Enabled");
			debug_ge_a = 1;
		}
	}
	else
	{
		if (debug_ge_a)
		{
			sn->con.AddLog(u8"[info]: debug_ge Disabled");
			debug_ge_a = 0;
		}
	}

	static bool devmode = false;
	if (sn->dbg_mode.enabled)
	{
		if (!devmode)
		{
			sn->con.AddLog(u8"[info]: devmode Enabled");
			devmode = 1;
		}
	}
	else
	{
		if (devmode)
		{
			sn->con.AddLog(u8"[info]: devmode Disabled");
			devmode = 0;
		}
	}	

	static bool key_update_modpack_disable = false;
	if (sn->disable_update_modpack_button.enabled)
	{
		if (!key_update_modpack_disable)
		{
			//sn->con.AddLog(u8"[info]: disable_update_modpack_button Enabled");
			sn->con.AddLog("{FFFF00}[info]: Кнопка \"Обновление сборки\" Отключена");
			key_update_modpack_disable = 1;
		}

		/*if (modpack_up_unlocked == true)
		{
			sn->con.AddLog("{FFFF00}[info]: Кнопка \"Обновление сборки\" пропущена");
			sn->latest_temp_ver_mp = modpack_latest_ver;
			modpack_latest_ver = sn->set.modpack_version;
			modpack_up_unlocked = 0;
		}*/
			
	}
	else
	{
		if (key_update_modpack_disable)
		{
			//sn->con.AddLog(u8"[info]: disable_update_modpack_button Disabled");
			sn->con.AddLog("{FFFF00}[info]: Кнопка \"Обновление сборки\" Включена");
			/*modpack_up_unlocked = 1;
			modpack_latest_ver = sn->latest_temp_ver_mp;*/
			key_update_modpack_disable = 0;
		}
	}

	


	static bool playbackground_a = false;
	if (sn->playbackgroundtheme.enabled)
	{
		BASS_Init(-1, 44100, BASS_DEVICE_CPSPEAKERS, GetActiveWindow(), NULL);
		if (!playbackground_a)
		{
			sn->con.AddLog("[info]: menu background music Enabled");
			char	filename[512];
			_snprintf(filename, sizeof(filename), "%s\\%s\\%s", GetExePath().c_str(), PROJECT_FOLDER, BE_MENU_MAINTHEME);
			smpl = BASS_SampleLoad(false, filename, 0, 0, 1, BASS_SAMPLE_LOOP);
			menu_theme = BASS_SampleGetChannel(smpl, FALSE);
			BASS_ChannelPlay(menu_theme, FALSE);
			QWORD len = BASS_ChannelGetLength(menu_theme, BASS_POS_BYTE);
			DWORD time = BASS_ChannelBytes2Seconds(menu_theme, len);
			playbackground_a = 1;
		}

		if (BASS_ChannelIsActive(menu_theme) == BASS_ACTIVE_PLAYING)
		{
			BASS_ChannelSetAttribute(menu_theme, BASS_ATTRIB_VOL, (float)sn->background_music_volume / 100);
			
			BASS_ChannelGetAttribute(menu_theme, BASS_ATTRIB_VOL, &amplitude);
		}
	}
	else
	{
		if (playbackground_a)
		{
			sn->con.AddLog("[info]:  menu background music Disabled");
			amplitude = 0.0f;
			playbackground_a = 0;
		}

		if (playbackground_a == 0)
		{
			if (BASS_ChannelIsActive(menu_theme) == BASS_ACTIVE_PLAYING)
			{
				BASS_SampleFree(smpl);
				BASS_ChannelPause(menu_theme);
				
				//sn->con.AddLog("BASS_ChannelStop");
			}
		}
	}		

	static bool unim_a = false;
	if (sn->umode.universal_mode.enabled)
	{
		if (!unim_a)
		{
			sn->con.AddLog(u8"[info]: Universal Mode Enabled");			
			sn->umode.project_mode = PROJECTMODES::PM_Anomaly_MP;
			checkmodpacklatestchangelist();
			filelistupdate();			
			unim_a = 1;
		}
	}
	else
	{
		if (unim_a)
		{
			sn->con.AddLog(u8"[info]: Universal Mode Disabled");
			sn->umode.project_mode = PROJECTMODES::PM_Anomaly_PZ;
			checkmodpacklatestchangelist();
			filelistupdate();			
			unim_a = 0;
		}
	}



	if (IsAppRunningAsAdminMode() == 1)
	{
		auto thw = ThreadWrapper::create();
		thw->beginAsync(CheckDirectPlayStatus);
		thw->detachAll();
	}

}


bool IsCheckAVXSupport()
{
	bool can_use_AVX = false;
	if (SupportCPU_AVX_check() == 1)
	{
		can_use_AVX = true;
		
	}
	else if (SupportCPU_AVX_check() == 0)
	{
		can_use_AVX = false;		
	}
	return can_use_AVX;
};


std::string GetLUFolder()
{
	std::string pathtoupdates = GetExePath() + "\\" + PROJECT_FOLDER + "\\" + PROJECT_UPDATES_FOLDER;
	if (fsm->FolderExist(pathtoupdates.c_str()))
	{
		///sn->con.AddLog(u8"{00BBBB}папка для обновлений найдена: %s", pathtoupdates.c_str());
		//createlauncherfolder = 0;
		return pathtoupdates;
	}
	else 
	{
		sn->con.AddLog(u8"{FF0000}папка для обновлений не найдена!");
		if ((!fsm->FolderExist(pathtoupdates.c_str())))
		{			
			fsm->CreateFolder(pathtoupdates.c_str());
			sn->con.AddLog(u8"{00BB00}папка для обновлений создана!");
		}		
		return pathtoupdates = "N\\A";
	}	
};

void CheckAnomalyLaucherFolder()
{
	std::string pathtoupdates = GetExePath() + "\\" + PROJECT_FOLDER;
	if (fsm->FolderExist(pathtoupdates.c_str()))
	{
		///sn->con.AddLog(u8"{00BBBB}папка для обновлений найдена: %s", pathtoupdates.c_str());
		createlauncherfolder = 0;		
	}
	else
	{
		sn->con.AddLog("{FF0000}папка для конфигурации не найдена!");
		if ((!fsm->FolderExist(pathtoupdates.c_str())))
		{
			fsm->CreateFolder(pathtoupdates.c_str());
			sn->con.AddLog("{00BB00}папка для конфигурации создана!");
		}		
	}
};


std::string GetMO2Folder() 
{
	std::string pathtomo2 = GetExePath() + "\\" + "MO2";
	if (fsm->FolderExist(pathtomo2.c_str()))
	{
		///sn->con.AddLog(u8"{00BBBB}папка для обновлений найдена: %s", pathtoupdates.c_str());		
		return pathtomo2;
	}
	else
	{
		return pathtomo2 = "N\\A";
	}
};

std::string GetBEFolder()
{
	std::string pathtobe = GetExePath() + "\\" + "MO2" + "\\" + "mods";
	if (fsm->FolderExist(pathtobe.c_str()))
	{
		///sn->con.AddLog(u8"{00BBBB}папка для обновлений найдена: %s", pathtoupdates.c_str());		
		return pathtobe;
	}
	else
	{
		return pathtobe = "N\\A";
	}
};

bool unlockedlauncherupdate(bool unlock)
{
	return unlock;
};

bool unlockedmodpackupdate(bool unlock)
{
	return unlock;
};

bool DetectLaunchbyModOrganizer()
{	
		std::string parent_proc = "ModOrganizer.exe";
		std::string detect_proc = GetParentProcessName();
		if (strcmp(parent_proc.c_str(), detect_proc.c_str()) == 0)
		{
			//Log("[DetectParent]: parent_proc: %s detect_proc: %s", parent_proc.c_str(), detect_proc.c_str());
			return true;
		}
		//Log("[DetectParent]: parent_proc: %s detect_proc: %s", parent_proc.c_str(), detect_proc.c_str());
		return false;	
}


