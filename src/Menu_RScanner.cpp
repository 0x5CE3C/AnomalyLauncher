#include "main.h"
using namespace ImGui;
void Menu_render_scanner() 
{
	auto thw = ThreadWrapper::create();
	Separator();
	pMenu->AddMenuText(u8"Проверяемые Модули:");
	Separator();
	TextWithColors(u8"{00FF00}Аномали: ");
	SameLine();
	if (scanner_anomaly_enable)
	{
		TextWithColors(u8"{AAFFFF} Выполняется Сканирование");
		SameLine();
		if (pMenu->AddStaticButton(u8"Отменить Сканирование"))
		{
			scanner_anomaly_enable = false;
		};
	}
	else
	{
		if (pMenu->AddStaticButton(u8"Проверить целостность файлов"))
		{
			//sn->con.AddLog(u8"Нажата клавиша > {FF0000} 'Проверить целостность файлов'");
			scanner_anomaly_enable = true;
			//thw->beginThreadex(ScanModule, 5, SCAN_MODULE_TYPE_ANOMALY);
			thw->beginAsync(ScanModule, SCAN_MODULE_TYPE_ANOMALY);
			thw->detachAll();
		};
	}	
	Separator();
	TextWithColors(u8"{00FF00}Mod Organizer 2: ");
	/*SameLine();
	if (scanner_mo2_enable)
	{
		TextWithColors(u8"{AAFFFF} Выполняется Сканирование");
		SameLine();
		if (pMenu->AddStaticButton(u8"Отменить Сканирование"))
		{
			scanner_mo2_enable = false;
		};
	}
	else
	{
		if (pMenu->AddStaticButton(u8"Проверить целостность файлов "))
		{
			sn->con.AddLog(u8"Нажата клавиша > {FF0000} 'Проверить целостность файлов'");
			scanner_mo2_enable = true;
			thw->beginThreadex(ScanModule, SCAN_MODULE_TYPE_MO2);
		};
	}*/
	Separator();
	TextWithColors(u8"{00FF00}сборка Blue Eclipse: ");
	/*SameLine();
	if (scanner_be_enable)
	{
		TextWithColors(u8"{AAFFFF} Выполняется Сканирование");
		SameLine();
		if (pMenu->AddStaticButton(u8"Отменить Сканирование"))
		{
			scanner_be_enable = false;
		};
	}
	else
	{
		if (pMenu->AddStaticButton(u8"Проверить целостность файлов  "))
		{
			sn->con.AddLog(u8"Нажата клавиша > {FF0000} 'Проверить целостность файлов'");
			scanner_be_enable = true;
			thw->beginThreadex(ScanModule, SCAN_MODULE_TYPE_BE);
		};
	}*/
	Separator();
	//pMenu->AddMenuText(FSManager->SetText(u8"Обновления Лаунчера %d"));			

};