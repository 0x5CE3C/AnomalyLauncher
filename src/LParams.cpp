#include "main.h"

//0 - "\\bin\\AnomalyDX8.exe",
//1 - "\\bin\\AnomalyDX9.exe",
//2 - "\\bin\\AnomalyDX10.exe",
//3 - "\\bin\\AnomalyDX11.exe",
//4 - "\\bin\\AnomalyDX8AVX.exe",
//5 - "\\bin\\AnomalyDX9AVX.exe",	
//6 - "\\bin\\AnomalyDX10AVX.exe",
//7 - "\\bin\\AnomalyDX11AVX.exe"	
const char* lp_renderer[] =
{
	"\\bin\\AnomalyDX8.exe",
	"\\bin\\AnomalyDX9.exe",
	"\\bin\\AnomalyDX10.exe",
	"\\bin\\AnomalyDX11.exe",
	"\\bin\\AnomalyDX8AVX.exe",
	"\\bin\\AnomalyDX9AVX.exe",
	"\\bin\\AnomalyDX10AVX.exe",
	"\\bin\\AnomalyDX11AVX.exe"
};

//0 - " -smap1536",
//1 - " -smap2048",
//2 - " -smap2560",
//3 - " -smap3072",
//4 - " -smap4096"
const char* lp_smap[] =
{
	"-smap1536 ",
	"-smap2048 ",
	"-smap2560 ",
	"-smap3072 ",
	"-smap4096 "
};

//0 - " -60hz",
//1 - " -no_dialog_header",
//2 - " -no_occq",
//3 - " -noramtex",
//4 - " -dbg",
//5 - " -nes_texture_storing",
//6 - " -noprefetch",
//7 - " -prefetch_sounds",
//8 - " -demomode",
//9 - " -nosound",
//10 - " -nointro",
//11 - " -skiplogo",
//12 - " -debug_ge"
//13 - " -devdbg"
const char* misc_lp[] =
{
	"-60hz ",
	"-no_dialog_header ",
	"-no_occq ",
	"-noramtex ",
	"-dbg ",
	"-nes_texture_storing ",
	"-noprefetch ",
	"-prefetch_sounds ",
	"-demomode ",
	"-nosound ",
	"-nointro ",
	"-skiplogo ",
	"-debug_ge ",
	"-dbgdev "
};

std::string GetRenderer()
{
	std::string target = "N/A";

	if (sn->set.render_type == 0)
	{
		target = GetExePath() + lp_renderer[0];
	}
	else if (sn->set.render_type == 1)
	{
		target = GetExePath() + lp_renderer[1];
	}
	else if (sn->set.render_type == 2)
	{
		target = GetExePath() + lp_renderer[2];
	}
	else if (sn->set.render_type == 3)
	{
		target = GetExePath() + lp_renderer[3];
	}

	if (sn->set.render_type == 0 && sn->avx_support.enabled == true)
	{
		target = GetExePath() + lp_renderer[4];
	}
	else if (sn->set.render_type == 1 && sn->avx_support.enabled == true)
	{
		target = GetExePath() + lp_renderer[5];
	}
	else if (sn->set.render_type == 2 && sn->avx_support.enabled == true)
	{
		target = GetExePath() + lp_renderer[6];
	}
	else if (sn->set.render_type == 3 && sn->avx_support.enabled == true)
	{
		target = GetExePath() + lp_renderer[7];
	}

	return target;
};

std::string GetLaunchparams()
{
	std::string target = "";
	if (sn->set.render_type == 0)
	{
		if (sn->set.shadow_map_type == 0)
		{
			target = target.append(lp_smap[0]);
		}
		else if (sn->set.shadow_map_type == 1)
		{
			target = target.append(lp_smap[1]);
		}
		else if (sn->set.shadow_map_type == 2)
		{
			target = target.append(lp_smap[2]);
		}
		else if (sn->set.shadow_map_type == 3)
		{
			target = target.append(lp_smap[3]);
		}
		else if (sn->set.shadow_map_type == 4)
		{
			target = target.append(lp_smap[4]);
		}

	}
	if (sn->set.render_type == 1)
	{
		if (sn->set.shadow_map_type == 0)
		{
			target = target.append(lp_smap[0]);
		}
		else if (sn->set.shadow_map_type == 1)
		{
			target = target.append(lp_smap[1]);
		}
		else if (sn->set.shadow_map_type == 2)
		{
			target = target.append(lp_smap[2]);
		}
		else if (sn->set.shadow_map_type == 3)
		{
			target = target.append(lp_smap[3]);
		}
		else if (sn->set.shadow_map_type == 4)
		{
			target = target.append(lp_smap[4]);
		}
	}
	if (sn->set.render_type == 2)
	{
		if (sn->set.shadow_map_type == 0)
		{
			target = target.append(lp_smap[0]);
		}
		else if (sn->set.shadow_map_type == 1)
		{
			target = target.append(lp_smap[1]);
		}
		else if (sn->set.shadow_map_type == 2)
		{
			target = target.append(lp_smap[2]);
		}
		else if (sn->set.shadow_map_type == 3)
		{
			target = target.append(lp_smap[3]);
		}
		else if (sn->set.shadow_map_type == 4)
		{
			target = target.append(lp_smap[4]);
		}
	}
	if (sn->set.render_type == 3)
	{
		if (sn->set.shadow_map_type == 0)
		{
			target = target.append(lp_smap[0]);
		}
		else if (sn->set.shadow_map_type == 1)
		{
			target = target.append(lp_smap[1]);
		}
		else if (sn->set.shadow_map_type == 2)
		{
			target = target.append(lp_smap[2]);
		}
		else if (sn->set.shadow_map_type == 3)
		{
			target = target.append(lp_smap[3]);
		}
		else if (sn->set.shadow_map_type == 4)
		{
			target = target.append(lp_smap[4]);
		}
	}
	if (sn->dbg.enabled)
	{
		target = target.append(misc_lp[4]);
	}
	if (sn->c60hz.enabled)
	{
		target = target.append(misc_lp[0]);
	}
	if (sn->debug_ge.enabled)
	{
		target = target.append(misc_lp[12]);
	}
	if (sn->demomode.enabled)
	{
		target = target.append(misc_lp[8]);
	}
	if (sn->nes_texture_storing.enabled)
	{
		target = target.append(misc_lp[5]);
	}
	if (sn->nointro.enabled)
	{
		target = target.append(misc_lp[10]);
	}
	if (sn->noprefetch.enabled)
	{
		target = target.append(misc_lp[6]);
	}
	if (sn->noramtex.enabled)
	{
		target = target.append(misc_lp[3]);
	}
	if (sn->nosound.enabled)
	{
		target = target.append(misc_lp[9]);
	}
	if (sn->no_dialog_header.enabled)
	{
		target = target.append(misc_lp[1]);
	}
	if (sn->no_occq.enabled)
	{
		target = target.append(misc_lp[2]);
	}
	if (sn->prefetch_sounds.enabled)
	{
		target = target.append(misc_lp[7]);
	}
	if (sn->skiplogo.enabled)
	{
		target = target.append(misc_lp[11]);
	}
	if (sn->devdbg.enabled)
	{
		target = target.append(misc_lp[13]);
	}

	return target;
};