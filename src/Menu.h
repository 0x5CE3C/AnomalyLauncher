#pragma once

enum TabId {
    GENERAL,
    DEBUG,
    TASKS,
    MODULES,
    LISTS,
    CHANGELOG,
    SETTINGS,
    HELP,
    CONSOLE,
    CREDITS
};

void Menu_render_info();
void Menu_render_general();
void Menu_render_optimization();
void Menu_render_debug();
void Menu_render_helper();
void Menu_render_sborka();
void Menu_render_updates();
void Menu_render_modules();
void Menu_render_lists();
void Menu_render_scanner();
void Menu_render_credits();

void RenderProgressBar();

void RenderFilesList();

void Menu_render_tasks();
void Menu_render_settings();
//void Menu_RLanguageChanger();
//void Menu_RThemeEditor();


extern bool g_сhanged; // for autosave params
extern bool g_SettingsDirty;    // Флаг: были изменения
extern float g_LastSettingChangeTime; // Время последнего изменения

bool Edited();