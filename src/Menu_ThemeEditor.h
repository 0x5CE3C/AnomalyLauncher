#pragma once


//Тема	Эффект	Атмосфера
//Default	Стандартный ImGui	Нейтрально
//GrayRed	Серо - красная палитра	Техно
//DarkAccent	Тёмная с ярким акцентом	Современно
//RedBlack	Контрастный чёрно - красный	Агрессивно
//Anomaly	Пульсирующие цвета	Живая, как аномалия
//PDA	Монохром + сканирование	Сталкер, ретро
//Cyber	Бегущие линии, неон	Киберпанк, сеть

void ApplyCurrentTheme();
void SetThemeWithAnimation(int themeId);

void RenderThemeAndFontMenu();
extern float themeFade;
extern bool isFading;
extern int pendingTheme;