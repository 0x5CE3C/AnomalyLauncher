#include "main.h"

void Menu_render_optimization() 
{
	
		//ImGui::TreePush(u8"Оптимизация");
		pMenu->AddHudItem2(&sn->c60hz.enabled, u8"Lock 60 Hz", true, u8"{FF0000}установить постоянное ограничение 60 Герц для монитора");
		//ImFuncs::FastDescr(u8"{FF0008} установить постоянное ограничение 60 Герц для монитора");/// R G B A
		//ImGui::Separator();
		//pMenu->AddButton(&sn->nointro.enabled, u8"No Intro");			
		//ImFuncs::FastDescr(u8"{FF0008} пропуск интро");/// R G B A
		//pMenu->AddButton(&sn->noprefetch.enabled, u8"No Prefetch");
		ImGui::SameLine();
		pMenu->AddHudItem2(&sn->noprefetch.enabled, u8"No Prefetch", true, u8"{FF0008}предзагрузка моделей и текстур");		
		ImGui::SameLine();		
		pMenu->AddHudItem2(&sn->noramtex.enabled, u8"No Ram Textures", true, u8"{FF0008}удержание текстур в памяти, сильно ускоряет загрузку локаций, но может вызывать сбои и ошибки на больших локациях. После включения данной опции будет работать DirectX 9");		
		ImGui::SameLine();
		//ImGui::Separator();
		pMenu->AddHudItem2(&sn->nosound.enabled, u8"No Sound", true, u8"{FF0008}режим без звука");		
		ImGui::SameLine();
		//ImGui::Separator();
		//pMenu->AddButton(&sn->no_dialog_header.enabled, "No Dialog Header");
		pMenu->AddHudItem2(&sn->no_occq.enabled, u8"No Occlusion", true, u8"{FF0008}ограничение видимых объектов на экране");		
		ImGui::SameLine();
		//ImGui::Separator();
		pMenu->AddHudItem2(&sn->prefetch_sounds.enabled, u8"Prefetch Sounds", true, u8"{FF0008}предзагрузка звука");		
		//pMenu->AddButton(&sn->skiplogo.enabled, u8"Skip Logo");			
		//ImFuncs::FastDescr(u8"{FF0008} пропуск логотипа");/// R G B A	
	
}
