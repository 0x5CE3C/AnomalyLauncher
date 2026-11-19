#pragma once

void Debug_ListExternalModsV2();//Work
void Debug_CheckAndCleanInvalidHashMods();//Work
void Request_CheckAndCleanInvalidHashMods();//Work

void Debug_ExportValidModsHashes();//Work

void Debug_ConvertExternalModsToFilelist();//Work

void Debug_ExportAnomalyHashes();
void Debug_ExportMO2Hashes();

void Debug_CheckAndCleanAnomalyItems();
void Debug_CheckAndCleanMO2Items();

//Work
bool RemoveModFromDisk(const std::string& modPath);

//HASH_CHECK_MODS, // Проверка модов на валидность, удаление всех невалидных. - обновление
void Debug_CheckAndCleanInvalidHashMods_ExternalModsDelete(float* outProgress = nullptr);



// Запрос на остановку проверки хеш-сумм модпака
// Функция для запроса прерывания извне (вызывается, например, по нажатию кнопки)
void AbortCurrentDebugTask();

bool isRunningCheckHash();// проверка на запущенную задачу проверки хешей.

// Запрос на сброс флагов для следующего запуска