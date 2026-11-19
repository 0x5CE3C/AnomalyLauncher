#pragma once

#include "cmds_funcs.h"
#include "console.h"
#include "structs.h"

extern struct stsn* sn;

void sn_init();

void radio_control();

int check();

void Save(const char* fmt, ...);

extern char* current_track;


extern bool createlauncherfolder;

//using 
//ExtractRes(IDR_TXT1, _T("TXT"), "prikol.txt");
void ExtractRes(WORD wResId, LPCSTR ResType, LPCSTR lpszOutputPath);

/// misc funcs
BOOL IsAppRunningAsAdminMode();

void RenderImage(IDirect3DDevice9* device);


//const int ROTATION_DELAY = 50; // Задержка в миллисекундах
//const char* textToAnimate = ""; // Текст для анимации
//animateString(textToAnimate, ROTATION_DELAY); // Запуск анимации

struct stradio
{
	int id;
	const char* stationname;
	const char* url;
};

#define MAX_RADIO_STATIONS 69
const struct stradio sradio[MAX_RADIO_STATIONS] =
{
	{0, "{FFFFFF}[Anomaly BE Radio]{00FFFF}", "https://stream.zeno.fm/ovdg4ddp09svv.mp3"},
	{1, "{FFFFFF}[Record Phonk]{00FFFF}", "https://radiorecord.hostingradio.ru/phonk96.aacp"},
	{2, "{FFFFFF}[Record Main]{00FFFF}", "https://radiorecord.hostingradio.ru/rr_main96.aacp"},
	{3, "{FFFFFF}[Record Tiesto]{00FFFF}", "https://radiorecord.hostingradio.ru/tiesto96.aacp" },
	{ 4, "{FFFFFF}[Record Rock]{00FFFF}", "https://radiorecord.hostingradio.ru/rock96.aacp" },
	{ 5, "{FFFFFF}[Record Innocence]{00FFFF}", "https://radiorecord.hostingradio.ru/ibiza96.aacp" },
	{ 6, u8"{FFFFFF}[Record Гастарбайтер FM]{00FFFF}", "https://radiorecord.hostingradio.ru/gast96.aacp" },
	{ 7, "{FFFFFF}[Record David Guetta]{00FFFF}", "https://radiorecord.hostingradio.ru/guetta96.aacp" },
	{ 8, "{FFFFFF}[Record Disco/Funk]{00FFFF}", "https://radiorecord.hostingradio.ru/discofunk96.aacp" },
	{ 9, "{FFFFFF}[Record Party 24/7]{00FFFF}", "https://radiorecord.hostingradio.ru/party96.aacp" },
	{ 10, "{FFFFFF}[Record Trap]{00FFFF}", "https://radiorecord.hostingradio.ru/trap96.aacp" },
	{ 11, "{FFFFFF}[Record Neurofunk]{00FFFF}", "https://radiorecord.hostingradio.ru/neurofunk96.aacp" },
	{ 12, "{FFFFFF}[Record Hardstyle]{00FFFF}", "https://radiorecord.hostingradio.ru/teo96.aacp" },
	{ 13, "{FFFFFF}[Record Darkside]{00FFFF}", "https://radiorecord.hostingradio.ru/darkside96.aacp" },
	{ 14, "{FFFFFF}[Record Ambient]{00FFFF}", "https://radiorecord.hostingradio.ru/ambient96.aacp" },
	{ 15, "{FFFFFF}[Record Armin van Buuren]{00FFFF}", "https://radiorecord.hostingradio.ru/armin96.aacp" },
	{ 16, "{FFFFFF}[Record Russian Mix]{00FFFF}", "https://radiorecord.hostingradio.ru/rus96.aacp" },
	{ 17, u8"{FFFFFF}[Record Супердискотека 90-х]{00FFFF}", "https://radiorecord.hostingradio.ru/sd9096.aacp" },
	{ 18, "{FFFFFF}[Record Russian Hits]{00FFFF}", "https://radiorecord.hostingradio.ru/russianhits64.aacp" },
	{ 19, "{FFFFFF}[Record Chill-Out]{00FFFF}", "https://radiorecord.hostingradio.ru/chil96.aacp" },
	{ 20, "{FFFFFF}[Record Deep]{00FFFF}", "https://radiorecord.hostingradio.ru/deep96.aacp" },
	{ 21, "{FFFFFF}[Record Megamix]{00FFFF}", "https://radiorecord.hostingradio.ru/mix96.aacp" },
	{ 22, "{FFFFFF}[Record Remix]{00FFFF}", "https://radiorecord.hostingradio.ru/rmx96.aacp" },
	{ 23, u8"{FFFFFF}[Record Рекорд 00-х]{00FFFF}", "https://radiorecord.hostingradio.ru/200096.aacp" },
	{ 24, "{FFFFFF}[Record Big Hits]{00FFFF}", "https://radiorecord.hostingradio.ru/bighits96.aacp" },
	{ 25, u8"{FFFFFF}[Record Маятник Фуко]{00FFFF}", "https://radiorecord.hostingradio.ru/mf96.aacp" },
	{ 26, "{FFFFFF}[Record Chill House]{00FFFF}", "https://radiorecord.hostingradio.ru/chillhouse96.aacp" },
	{ 27, "{FFFFFF}[Record Trancemission]{00FFFF}", "https://radiorecord.hostingradio.ru/tm96.aacp" },
	{ 28, "{FFFFFF}[Record Record 80-х]{00FFFF}", "https://radiorecord.hostingradio.ru/198096.aacp" },
	{ 29, "{FFFFFF}[Record Russian Gold]{00FFFF}", "https://radiorecord.hostingradio.ru/russiangold96.aacp" },
	{ 30, "{FFFFFF}[Record Pirate Station]{00FFFF}", "https://radiorecord.hostingradio.ru/ps96.aacp" },
	{ 31, "{FFFFFF}[Record Record Gold]{00FFFF}", "https://radiorecord.hostingradio.ru/gold96.aacp" },
	{ 32, "{FFFFFF}[Record Summer Dance]{00FFFF}", "https://radiorecord.hostingradio.ru/summerparty96.aacp" },
	{ 33, u8"{FFFFFF}[Record На Хайпе]{00FFFF}", "https://radiorecord.hostingradio.ru/hype96.aacp" },
	{ 34, u8"{FFFFFF}[Record Руки Вверх!]{00FFFF}", "https://radiorecord.hostingradio.ru/rv96.aacp" },
	{ 35, "{FFFFFF}[Record Rap Hits]{00FFFF}", "https://radiorecord.hostingradio.ru/rap96.aacp" },
	{ 36, "{FFFFFF}[Record Rap Classics]{00FFFF}", "https://radiorecord.hostingradio.ru/rapclassics96.aacp" },
	{ 37, "{FFFFFF}[Record Trance Classics]{00FFFF}", "https://radiorecord.hostingradio.ru/trancehits96.aacp" },
	{ 38, "{FFFFFF}[Record VIP House]{00FFFF}", "https://radiorecord.hostingradio.ru/vip96.aacp" },
	{ 39, "{FFFFFF}[Record Bass House]{00FFFF}", "https://radiorecord.hostingradio.ru/jackin96.aacp" },
	{ 40, "{FFFFFF}[Record Organic]{00FFFF}", "https://radiorecord.hostingradio.ru/organic96.aacp" },
	{ 41, u8"{FFFFFF}[Record Русская Зима]{00FFFF}", "https://radiorecord.hostingradio.ru/ruszima96.aacp" },
	{ 42, "{FFFFFF}[Record Christmas]{00FFFF}", "https://radiorecord.hostingradio.ru/christmas96.aacp" },
	{ 43, "{FFFFFF}[Record Christmas Chill]{00FFFF}", "https://radiorecord.hostingradio.ru/christmaschill96.aacp" },
	{ 44, "{FFFFFF}[Record Complextro]{00FFFF}", "https://radiorecord.hostingradio.ru/complextro96.aacp" },
	{ 45, u8"{FFFFFF}[Record Симфония FM]{00FFFF}", "https://radiorecord.hostingradio.ru/symph96.aacp" },
	{ 46, u8"{FFFFFF}[Record Нафталин FM]{00FFFF}", "https://radiorecord.hostingradio.ru/naft96.aacp" },
	{ 47, "{FFFFFF}[Record Groove/Tribal]{00FFFF}", "https://radiorecord.hostingradio.ru/groovetribal96.aacp" },
	{ 48, u8"{FFFFFF}[Record Веснушка FM]{00FFFF}", "https://radiorecord.hostingradio.ru/deti96.aacp" },
	{ 49, u8"{FFFFFF}[Record Медляк FM]{00FFFF}", "https://radiorecord.hostingradio.ru/mdl96.aacp" },
	{ 50, "{FFFFFF}[Record 2-step]{00FFFF}", "https://radiorecord.hostingradio.ru/2step96.aacp" },
	{ 51, "{FFFFFF}[Record Moombahton]{00FFFF}", "https://radiorecord.hostingradio.ru/mmbt96.aacp" },
	{ 52, "{FFFFFF}[Record Rave FM]{00FFFF}", "https://radiorecord.hostingradio.ru/rave96.aacp" },
	{ 53, "{FFFFFF}[Record Hypnotic]{00FFFF}", "https://radiorecord.hostingradio.ru/hypno96.aacp" },
	{ 54, "{FFFFFF}[Record Jungle]{00FFFF}", "https://radiorecord.hostingradio.ru/jungle96.aacp" },
	{ 55, "{FFFFFF}[Record Midtempo]{00FFFF}", "https://radiorecord.hostingradio.ru/mt96.aacp" },
	{ 56, u8"{FFFFFF}[Record Гоп FM]{00FFFF}", "https://radiorecord.hostingradio.ru/gop96.aacp" },
	{ 57, "{FFFFFF}[Record Future Bass]{00FFFF}", "https://radiorecord.hostingradio.ru/fbass96.aacp" },
	{ 58, "{FFFFFF}[Record Tecktonik]{00FFFF}", "https://radiorecord.hostingradio.ru/tecktonik96.aacp" },
	{ 59, "{FFFFFF}[Record Latina Dance]{00FFFF}", "https://radiorecord.hostingradio.ru/latina96.aacp" },
	{ 60, "{FFFFFF}[Record Synthwave]{00FFFF}", "https://radiorecord.hostingradio.ru/synth96.aacp" },
	{ 61, "{FFFFFF}[Record EDM Classics]{00FFFF}", "https://radiorecord.hostingradio.ru/edmhits96.aacp" },
	{ 62, "{FFFFFF}[Record Progressive]{00FFFF}", "https://radiorecord.hostingradio.ru/progr96.aacp" },
	{ 63, "{FFFFFF}[Record Old School]{00FFFF}", "https://radiorecord.hostingradio.ru/pump96.aacp" },
	{ 64, "{FFFFFF}[Record Electro]{00FFFF}", "https://radiorecord.hostingradio.ru/elect96.aacp" },
	{ 65, "{FFFFFF}[Record Hard Bass]{00FFFF}", "https://radiorecord.hostingradio.ru/hbass96.aacp" },
	{ 66, "{FFFFFF}[Record Dancecore]{00FFFF}", "https://radiorecord.hostingradio.ru/dc96.aacp" },
	{ 67, "{FFFFFF}[Record Dubstep]{00FFFF}", "https://radiorecord.hostingradio.ru/dub96.aacp" },
	{ 68, "{FFFFFF}[Radio Ultra]{00FFFF}", "http://nashe2.hostingradio.ru:80/ultra-128.mp3"},
};

void radio_control();
void radio_reconnect();
extern char* current_track;