#include "main.h"

void CALLBACK StatusProc(const void* buffer, DWORD length, void* user)
{
	if (buffer && !length && (DWORD)user == 0) // got HTTP/ICY tags, and this is still the current request
		sn->con.AddLog(u8"radio is loading..");
}
HCHANNEL mp3_channel;
static HSAMPLE sample;
HSTREAM my_radio;

char* current_track;

void radio_reconnect()
{
	if (sn->radio_reconnect == 1 && sn->my_radio.enabled == true)
	{
		sn->my_radio.enabled = false;
	}
	else if (sn->radio_reconnect == 1 && sn->my_radio.enabled == false)
	{
		static DWORD timer = 0;
		if (GetTickCount() - timer > 1)
		{
			timer = GetTickCount();
			sn->my_radio.enabled = true;
			sn->radio_reconnect = 0;
		}
	}
}

void radio_control(void) {

	static bool radio = false;
	
	auto thw = ThreadWrapper::create();
	thw->beginThread(radio_reconnect);
	if (sn->my_radio.enabled)
	{
		//sn->con.AddLog(u8"sn->my_radio.active");
		BASS_Init(-1, 44100, 0, GetActiveWindow(), NULL);

		if (!radio)
		{
			DWORD r;//
			my_radio = BASS_StreamCreateURL(sradio[sn->radio_station].url, 0, BASS_STREAM_BLOCK | BASS_STREAM_STATUS | BASS_STREAM_AUTOFREE, StatusProc, (void*)&r); // open URL
			BASS_ChannelPlay(my_radio, FALSE);
			sn->con.AddLog(u8"Radio started");
			radio = 1;
		}

		if (BASS_ChannelIsActive(my_radio) == BASS_ACTIVE_PLAYING)
		{
			//float fvolume = ;
			BASS_ChannelSetAttribute(my_radio, BASS_ATTRIB_VOL, (float)sn->radio_volume / 100);


			const char* meta = BASS_ChannelGetTags(my_radio, BASS_TAG_META);
			if (meta)
			{
				const char* p = strstr(meta, "StreamTitle='");
				if (p)
				{
					const char* p2 = strstr(p, "';");
					if (p2)
					{
						char* t = strdup(p + 13);
						t[p2 - (p + 13)] = 0;

						current_track = t;

					}
				}
			}
		}

	}
	else
	{
		if (radio)
		{
			if (BASS_ChannelIsActive(my_radio) == BASS_ACTIVE_PLAYING)
			{
				BASS_StreamFree(my_radio);
				BASS_ChannelPause(my_radio);
				//BASS_Stop();				
			}
			radio = 0;
			sn->con.AddLog(u8"Radio stopped");
		}
	}
}