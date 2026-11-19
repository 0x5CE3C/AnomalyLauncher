#pragma once


// ТОЛЬКО объявления (extern), без значений!
extern std::atomic<bool> g_stopAnomalyPack;
extern std::atomic<bool> g_isRunningAnomalyPack;

extern std::atomic<bool> g_stopMo2ModsPack;
extern std::atomic<bool> g_isRunningMo2Mods;

extern std::atomic<bool> g_stopMo2TopPack;
extern std::atomic<bool> g_isRunningMo2Top;



bool IsRunningAnomalyPack();
bool IsRunningMo2Mods();
bool IsRunningMo2Top();

void AbortMo2TopPack();
void AbortMo2ModsPack();
void AbortAnomalyPack();

void PackAnomalyTaskHandler(float* outProgress);
void PackMo2ModsTaskHandler(float* outProgress);
void PackMo2TopTaskHandler(float* outProgress);