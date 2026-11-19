#include "main.h"

void TaskExecutor::AddFileDownload(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        currentTask.progress = 0.0f;
        currentTask.func();
        sn->con.AddLog("[TaskExecutor]: Executing task ID: %i TASK_TYPE_FILE_DOWNLOAD", currentTask.id);

        auto httpc = HttpClient::getInstance();

        while (currentTask.progress <= 100.0f) {
            sleep(100);
            currentTask.progress = progress_; // Увеличиваем прогресс
            //sn->con.AddLog("Download Progress: %0.1f%% | %d", currentTask.progress, isDownloading);           

            if (currentTask.progress == 100.0f && !httpc->IsDownloading())
            {
                currentTask.status = TaskStatus::COMPLETED;
                //currentTask.progress = 0.0f;
                //sn->con.AddLog("[TaskExecutor]: Задача с ID %i завершена", currentTasks.front().id);                     
                //currentTasks.erase(currentTasks.begin()); // Удаляем после обработки          
                break;
            }
        }
    }
}

void TaskExecutor::UpdateFile(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {

        Log("[Менеджер задач]: Выполнение задачи: %i, Тип: Обновление файла.", currentTask.id);
        sn->con.AddLog("[Менеджер задач]: Выполнение задачи: %i, Тип: Обновление файла.", currentTask.id);

        currentTask.progress = 0.0f;
        currentTask.func();

        currentTask.progress = 100.0f;
        currentTask.status = TaskStatus::COMPLETED;

    }
}

void TaskExecutor::UnpackZipFiles(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        Log("Распаковка ZIP файлов для задачи ID: %i", currentTask.id);
        currentTask.func();
        if (zip_extracted == 0 && unpacked_status == 0)
        {
            sn->con.AddLog("[Unpacking]: 1");
            if (is_extracting == false)
            {
                currentTask.progress = 100.0f;
                currentTask.status = TaskStatus::COMPLETED; // Завершение задачи
                sn->con.AddLog("Zip файлы для задачи ID %i распакованы.", currentTask.id);
            }
        }
    }
}

void TaskExecutor::PerformFileOperation(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        Log("Выполнение файловой операции для задачи ID: %i", currentTask.id);
        currentTask.func();

        sn->con.AddLog("[TaskExecutor]: Executing task ID: %i  TASK_TYPE_FILE_OPERATION", currentTask.id);

        if (fileop_move || fileop_remove || fileop_rename || dirop_move || dirop_remove || dirop_rename)
        {
            currentTask.progress = 100.0f;
            currentTask.status = TaskStatus::COMPLETED; // Завершение задачи
            sn->con.AddLog("Файловая операция для задачи ID %i выполнена.", currentTask.id);
        }
        else {
            // Можно добавить обработку ошибок или изменение статуса задачи
            sn->con.AddLog("Файловая операция для задачи ID %i не была выполнена.", currentTask.id);
        }
    }
}

void Debug_CheckAndBackupExternalMods(float* outProgress = nullptr) {
    if (outProgress) *outProgress = 0.0f;

    sn->con.AddLog("Диагностика: проверка сторонних модов %s",
        (sn->umode.universal_mode.enabled ? cp1251_to_utf8(sn->umode.modpack_name).c_str() : u8"Purge of Zone"));

    bool foundInvalid = false;
    bool foundExternal = false;
    int totalChecked = 0;
    int totalMods = 0;

    // === ШАГ 1: Подсчёт валидных модов ===
    for (int i = 0; i < iFileListCount; ++i) {
        if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE) {
            totalMods++;
        }
    }

    if (totalMods == 0) {
        sn->con.AddLog("Не найдено ни одного мода для проверки.");
    }

    // === ШАГ 2: Проверка хэшей валидных модов ===
    for (int i = 0; i < iFileListCount; ++i) {
        if (filelist.fileList[i].m_type != SUBMODULE_TYPE_BE) continue;

        const char* modName = filelist.fileList[i].name;
        std::string pathToCheck = GetDirectoryPath(i);

        if (!fsm->DirectoryExists(pathToCheck.c_str())) {
            sn->con.AddLog(" Пропущен: папка не найдена — %s", modName);
            totalChecked++;
            if (outProgress) *outProgress = (float)(totalChecked * 100) / (totalMods + 1);
            continue;
        }

        totalChecked++;
        if (outProgress) {
            *outProgress = (float)(totalChecked * 100) / (totalMods + 1);
        }
    }

    // === ШАГ 3: Поиск и перемещение сторонних модов ===
    std::string baseDirectory = GetMO2Folder() + "\\mods";
    std::string backupRoot = GetMO2Folder() + "\\backup\\external_mods";

    // Создаём имя папки с датой и временем: 2026-02-09_14-21
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = {};
#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    tm = *std::localtime(&time_t);
#endif
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d_%H-%M");
    std::string timestamp = ss.str();
    std::string backupDir = backupRoot + "\\" + timestamp;

    sn->con.AddLog("Начинаю проверку сторонних модов. Резервная копия — %s", backupDir.c_str());

    if (!fsm->FolderExist(baseDirectory.c_str())) {
        sn->con.AddLog("{AAFFFF}Папка mods не найдена — пропуск.");
    }
    else {
        // Создаём папку бэкапа
        if (!fsm->FolderExist(backupDir.c_str())) {
            fsm->CreatefolderRecursive(backupDir.c_str());
            sn->con.AddLog("Создана папка резервного копирования: %s", backupDir.c_str());
        }

        std::set<std::string> allowedMods;
        for (int i = 0; i < iFileListCount; ++i) {
            if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE) {
                allowedMods.insert(filelist.fileList[i].name);
            }
        }

        // Список системных папок MO2, которые НЕ нужно трогать
        std::set<std::string> protectedFolders = {
            "profiles", "downloads", "mods", "backup", "config", "overwrite"
        };

        for (const auto& entry : std::filesystem::directory_iterator(
            baseDirectory,
            std::filesystem::directory_options::skip_permission_denied)) {
            if (entry.is_directory()) {
                std::string folderName = entry.path().filename().string();
                std::string fullPath = entry.path().string();

                // Пропускаем защищённые папки
                if (protectedFolders.find(folderName) != protectedFolders.end()) {
                    sn->con.AddLog(" Пропущена защищенная папка: %s", folderName.c_str());
                    continue;
                }

                if (allowedMods.find(folderName) == allowedMods.end()) {
                    std::string destPath = backupDir + "\\" + folderName;
                    sn->con.AddLog(" Сторонний мод: %s — перемещаю в %s", folderName.c_str(), destPath.c_str());

                    try {
                        std::filesystem::rename(fullPath, destPath);
                        sn->con.AddLog("   Успешно перемещён.");
                        foundExternal = true;
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        sn->con.AddLog("   Ошибка перемещения: %s", e.what());
                        // Если не получилось переместить — попробуем удалить?
                        // Или оставить на месте с предупреждением
                    }
                }
            }
        }
    }

    // === Завершение ===
    if (!foundExternal) {
        sn->con.AddLog("Всё в порядке: сторонних модов нет.");
    }
    else {
        if (foundExternal) sn->con.AddLog("Очистка: сторонние моды перемещены в резервную папку.");
    }

    //sn->con.AddLog("Диагностика завершена.");
    if (outProgress) *outProgress = 100.0f;
}

void TaskExecutor::AddHashCheckTask(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        currentTask.progress = 0.0f;
        sn->con.AddLog("[TaskExecutor]: Executing task ID: %i TASK_TYPE_HASH_CHECK_MODS", currentTask.id);

        auto thw = ThreadWrapper::create();
        thw->beginAsync([this, &currentTask]() {
            Debug_CheckAndCleanInvalidHashMods_ExternalModsDelete(&currentTask.progress);
            currentTask.status = TaskStatus::COMPLETED;
            });
        //thw->detachAll();
    }
}

void TaskExecutor::ScanningAndBackupExternalMods(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        currentTask.progress = 0.0f;
        sn->con.AddLog("[TaskExecutor]: Executing task ID: %i TASK_TYPE_SCANNING_AND_BACKUP_EXTERNAL_MODS", currentTask.id);

        auto thw = ThreadWrapper::create();
        thw->beginAsync([this, &currentTask]() {
            Debug_CheckAndBackupExternalMods(&currentTask.progress);
            currentTask.status = TaskStatus::COMPLETED;
            });
    }
}

void TaskExecutor::ScanningTask(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        currentTask.progress = 0.0f;
        sn->con.AddLog("[TaskExecutor]: Запуск задачи сканирования — последовательная проверка модулей");

        const int modules[4] = {
            SCAN_MODULE_TYPE_EXTRACTOR,
            SCAN_MODULE_TYPE_ANOMALY,
            SCAN_MODULE_TYPE_MO2,
            SCAN_MODULE_TYPE_BE
        };
        const int total = 4;

        auto thw = ThreadWrapper::create();
        thw->beginAsync([this, &currentTask, modules, total]() {
            float progressPerModule = 1.0f / total;

            for (int i = 0; i < total; ++i) {
                int moduleType = modules[i];

                // Отключаем хеширование ТОЛЬКО для BE ДО запуска модуля
                if (moduleType == SCAN_MODULE_TYPE_BE) {
                    sn->scanhashes.enabled = false;
                    sn->scanhashes_dirs.enabled = false;
                }

                // Устанавливаем флаг, что сканирование запущено
                switch (moduleType) {
                case SCAN_MODULE_TYPE_EXTRACTOR: scanner_extractor_enable = true; break;
                case SCAN_MODULE_TYPE_ANOMALY:   scanner_anomaly_enable = true;   break;
                case SCAN_MODULE_TYPE_MO2:       scanner_mo2_enable = true;       break;
                case SCAN_MODULE_TYPE_BE:        scanner_be_enable = true;        break;
                }

                // Запускаем сканирование   
                ScanModule(moduleType);  // ? Только один аргумент

                if (scanner_extractor_enable == true)
                {
                    scanner_extractor_enable = false;
                }

                // Ждём завершения — опрашиваем флаг
                while (true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));

                    bool isCompleted = false;
                    switch (moduleType) {
                    case SCAN_MODULE_TYPE_EXTRACTOR: isCompleted = !scanner_extractor_enable; break;
                    case SCAN_MODULE_TYPE_ANOMALY:   isCompleted = !scanner_anomaly_enable;   break;
                    case SCAN_MODULE_TYPE_MO2: isCompleted = !scanner_mo2_enable;  break;
                    case SCAN_MODULE_TYPE_BE: isCompleted = !scanner_be_enable; break;

                    }

                    if (isCompleted) break;
                }

                // Обновляем прогресс после завершения модуля
                currentTask.progress = (i + 1) * progressPerModule;
            }

            // Задача завершена
            currentTask.progress = 1.0f;
            currentTask.status = TaskStatus::COMPLETED;

            sn->con.AddLog("[TaskExecutor]: Все модули проверены — сканирование завершено");
            //executor->AddTask([]() {}, TaskType::HASH_CHECK_MODS);

            //if(module_anomaly_damaged == false && module_mo2_damaged == false && //module_be_damaged == false)
            //{
                executor->AddTask([]() {}, TaskType::UPDATE_VERSION);
                PatchModOrganizerbtn();
            //}


        });
    }
}


void TaskExecutor::UpdateVersionTask(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        currentTask.progress = 0.0f;
        sn->con.AddLog("[TaskExecutor]: Executing task ID: %i TASK_TYPE_UPDATE_VERSION", currentTask.id);


        const int modules[4] = {
            SCAN_MODULE_TYPE_EXTRACTOR,
            SCAN_MODULE_TYPE_ANOMALY,
            SCAN_MODULE_TYPE_MO2,
            SCAN_MODULE_TYPE_BE
        };
        const int total = 4;

        auto thw = ThreadWrapper::create();
        thw->beginAsync([this, &currentTask, modules, total]() {
            float progressPerModule = 1.0f / total;

            for (int i = 0; i < total; ++i) {
                int moduleType = modules[i];

                // Отключаем хеширование ТОЛЬКО для BE ДО запуска модуля
                if (moduleType == SCAN_MODULE_TYPE_BE) {
                    sn->scanhashes.enabled = false;
                    sn->scanhashes_dirs.enabled = false;
                }

                if (moduleType == SCAN_MODULE_TYPE_ANOMALY) {
                    sn->scanhashes.enabled = false;
                    sn->scanhashes_dirs.enabled = false;
                }

                if (moduleType == SCAN_MODULE_TYPE_MO2) {
                    sn->scanhashes.enabled = false;
                    sn->scanhashes_dirs.enabled = false;
                }

                // Устанавливаем флаг, что сканирование запущено
                switch (moduleType) {
                case SCAN_MODULE_TYPE_EXTRACTOR: scanner_extractor_enable = true; break;
                case SCAN_MODULE_TYPE_ANOMALY:   scanner_anomaly_enable = true;   break;
                case SCAN_MODULE_TYPE_MO2:       scanner_mo2_enable = true;       break;
                case SCAN_MODULE_TYPE_BE:        scanner_be_enable = true;        break;
                }

                // Запускаем сканирование   
                ScanModule(moduleType);  // ? Только один аргумент

                if (scanner_extractor_enable == true)
                {
                    scanner_extractor_enable = false;
                }

                // Ждём завершения — опрашиваем флаг
                while (true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));

                    bool isCompleted = false;
                    switch (moduleType) {
                    case SCAN_MODULE_TYPE_EXTRACTOR: isCompleted = !scanner_extractor_enable; break;
                    case SCAN_MODULE_TYPE_ANOMALY:   isCompleted = !scanner_anomaly_enable;   break;
                    case SCAN_MODULE_TYPE_MO2: isCompleted = !scanner_mo2_enable;  break;
                    case SCAN_MODULE_TYPE_BE: isCompleted = !scanner_be_enable; break;

                    }

                    if (isCompleted) break;
                }

                // Обновляем прогресс после завершения модуля
                currentTask.progress = (i + 1) * progressPerModule;
            }

            if (module_anomaly_damaged == false && module_mo2_damaged == false && module_be_damaged == false)
            {

                if (strcmp(modpack_latest_ver.c_str(), sn->set.modpack_version.c_str()) != 0)
                {
                    sn->set.modpack_version = modpack_latest_ver;
                    if (sn->download_system.enabled == true) sn->download_system.enabled = false;
                    Save();
                    currentTask.progress = 100.0f;
                    currentTask.status = TaskStatus::COMPLETED;
                    modpack_up_unlocked = 0;
                    sn->con.AddLog("{AAFFFF}Обновление завершено.");
                    Log("[Обновление] > Обновление завершено");
                }
            }
        });
       
    }
}

//SCANNING_RESTORE_ALL
void TaskExecutor::ScanningAndRestoreAllModules(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        cancellationRequested = false; // Сбрасываем флаг при запуске
        currentTask.progress = 0.0f;
        sn->con.AddLog("[Менеджер задач]: Запуск задачи сканирования — последовательная проверка и восстановление ВСЕХ модулей");

        const int modules[4] = {
            SCAN_MODULE_TYPE_EXTRACTOR,
            SCAN_MODULE_TYPE_ANOMALY,
            SCAN_MODULE_TYPE_MO2,
            SCAN_MODULE_TYPE_BE
        };
        const int total = 4;

        auto thw = ThreadWrapper::create();
        thw->beginAsync([this, &currentTask, modules, total]() {
            float progressPerModule = 1.0f / total;

            for (int i = 0; i < total; ++i) {
                // Проверка на отмену ПЕРЕД запуском каждого модуля
                if (cancellationRequested) {
                    break;
                }

                int moduleType = modules[i];

                // Отключаем хеширование ТОЛЬКО для BE ДО запуска модуля
                if (moduleType == SCAN_MODULE_TYPE_BE) {
                    sn->scanhashes.enabled = false;
                    sn->scanhashes_dirs.enabled = false;
                }

                // Устанавливаем флаг, что сканирование запущено
                switch (moduleType) {
                case SCAN_MODULE_TYPE_EXTRACTOR: scanner_extractor_enable = true; break;
                case SCAN_MODULE_TYPE_ANOMALY:   scanner_anomaly_enable = true;   break;
                case SCAN_MODULE_TYPE_MO2:       scanner_mo2_enable = true;       break;
                case SCAN_MODULE_TYPE_BE:        scanner_be_enable = true;        break;
                }

                // Запускаем сканирование   
                ScanModule(moduleType);  // ? Только один аргумент

                if (scanner_extractor_enable == true)
                {
                    scanner_extractor_enable = false;
                }

                // Ждём завершения — опрашиваем флаг
                while (true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));

                    // Быстрая проверка на отмену
                    if (cancellationRequested) {
                        break;
                    }

                    bool isCompleted = false;
                    switch (moduleType) {
                    case SCAN_MODULE_TYPE_EXTRACTOR: isCompleted = !scanner_extractor_enable; break;
                    case SCAN_MODULE_TYPE_ANOMALY:   isCompleted = !scanner_anomaly_enable;   break;
                    case SCAN_MODULE_TYPE_MO2: isCompleted = !scanner_mo2_enable;  break;
                    case SCAN_MODULE_TYPE_BE: isCompleted = !scanner_be_enable; break;

                    }

                    if (isCompleted) break;
                }

                // Если отмена была запрошена, прерываем цикл
                if (cancellationRequested) {
                    break;
                }

                // Обновляем прогресс после завершения модуля
                currentTask.progress = (i + 1) * progressPerModule;
            }

            // Обработка результата: завершение или отмена
            if (cancellationRequested) {
                currentTask.status = TaskStatus::CANCELED;
                sn->con.AddLog("[Менеджер задач]: Сканирование отменено пользователем");
                Log("[Восстановление] > Сканирование отменено");
            }
            else {
                // Задача завершена нормально
                currentTask.progress = 1.0f;
                currentTask.status = TaskStatus::COMPLETED;

                sn->con.AddLog("[Менеджер задач]: Все модули проверены — сканирование завершено");
                PatchModOrganizerbtn();

                if (strcmp(modpack_latest_ver.c_str(), sn->set.modpack_version.c_str()) == 0) {

                    // отключение функций
                    if (scanner_extractor_enable == true) scanner_extractor_enable = false;
                    if (scanner_anomaly_enable == true) scanner_anomaly_enable = false;
                    if (scanner_mo2_enable == true) scanner_mo2_enable = false;
                    if (scanner_be_enable == true) scanner_be_enable = false;
                    if (sn->download_system.enabled == true) sn->download_system.enabled = false;
                    if (sn->scanhashes.enabled == true) sn->scanhashes.enabled = false;
                    if (sn->scanhashes_dirs.enabled == true) sn->scanhashes_dirs.enabled = false;

                    modpack_up_unlocked = 0;

                    sn->con.AddLog("{AAFFFF}Восстановление завершено.");
                    Log("[Восстановление] > Восстановление завершено");

                }
            }

            });
    }
}

//SCANNING_RESTORE_COMPLETED
void TaskExecutor::ScanningAndRestoreAll_Completed(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        currentTask.progress = 0.0f;
        sn->con.AddLog("[Менеджер задач]: Выполнение задачи ID: %i, Тип: Завершение восстановления", currentTask.id);

        if (strcmp(modpack_latest_ver.c_str(), sn->set.modpack_version.c_str()) == 0)
        {
            if (scanner_extractor_enable == true) scanner_extractor_enable = false;
            if (scanner_anomaly_enable == true) scanner_anomaly_enable = false;
            if (scanner_mo2_enable == true) scanner_mo2_enable = false;
            if (scanner_be_enable == true) scanner_be_enable = false;
            if (sn->download_system.enabled == true) sn->download_system.enabled = false;
            if (sn->scanhashes.enabled == true) sn->scanhashes.enabled = false;
            if (sn->scanhashes_dirs.enabled == true) sn->scanhashes_dirs.enabled = false;
            //Save();
            currentTask.progress = 100.0f;
            currentTask.status = TaskStatus::COMPLETED;
            modpack_up_unlocked = 0;
            sn->con.AddLog("{AAFFFF}Восстановление завершено.");
            Log("[Восстановление] > Восстановление завершено");
        }
    }
}



void TaskExecutor::DownloadPatches(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        Log(u8"Скачивание патчей для задачи ID: %i", currentTask.id);

        // Здесь добавьте логику скачивания патчей
        // Например: currentTask.progress = 100.0f; (завершено)

        currentTask.status = TaskStatus::COMPLETED; // Завершение задачи
        sn->con.AddLog("Патчи для задачи ID %i скачаны.", currentTask.id);
    }
}

void TaskExecutor::InstallPatches(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        Log(u8"Установка патчей для задачи ID: %i", currentTask.id);

        // Здесь добавьте логику установки патчей
        // Например: currentTask.progress = 100.0f; (завершено)

        currentTask.status = TaskStatus::COMPLETED; // Завершение задачи
        sn->con.AddLog("Патчи для задачи ID %i установлены.", currentTask.id);
    }
}

void TaskExecutor::DownloadUserFiles(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        Log(u8"Скачивание пользовательских файлов для задачи ID: %i", currentTask.id);

        // Здесь добавьте логику скачивания пользовательских файлов
        // Например: currentTask.progress = 100.0f; (завершено)

        currentTask.status = TaskStatus::COMPLETED; // Завершение задачи
        sn->con.AddLog("Пользовательские файлы для задачи ID %i скачаны.", currentTask.id);
    }
}

void TaskExecutor::PerformUserFileOperation(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING) {
        Log(u8"Выполнение пользовательской операции над файлом для задачи ID: %i", currentTask.id);

        // Здесь добавьте логику выполнения пользовательской операции
        // Например: currentTask.progress = 100.0f; (завершено)

        currentTask.status = TaskStatus::COMPLETED; // Завершение задачи
        sn->con.AddLog("Пользовательская операция над файлом для задачи ID %i выполнена.", currentTask.id);
    }
}





/// УПАКОВКА НАХУЙ!!!!!!!!!!!!!!!!!!!!!!!!ААААААААААААААААААААААААА!!!!!!!!!!!!!!
void TaskExecutor::AddAnomalyPackTask(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING){

    currentTask.progress = 0.0f;
    currentTask.status = TaskStatus::RUNNING;

    sn->con.AddLog("[TaskExecutor]: Starting Anomaly pack task ID: %i", currentTask.id);

    auto thw = ThreadWrapper::create();
    thw->beginAsync([this, &currentTask]() {
        PackAnomalyTaskHandler(&currentTask.progress);
        currentTask.status = TaskStatus::COMPLETED;
        // Если задача была отменена, статус останется RUNNING или ты можешь ставить CANCELLED
        // в самой функции-обработчике, если хочешь более точную логику.
        });
    // thw->detachAll(); // как у тебя
    }
}

void TaskExecutor::AddMo2ModsPackTask(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING){

        currentTask.progress = 0.0f;
        currentTask.status = TaskStatus::RUNNING;

        sn->con.AddLog("[TaskExecutor]: Starting MO2 Mods pack task ID: %i", currentTask.id);

        //auto thw = ThreadWrapper::create();
        //thw->beginAsync(
          //  [this, &currentTask]() {
            PackMo2ModsTaskHandler(&currentTask.progress);
            currentTask.status = TaskStatus::COMPLETED;
        //    }
        //);
    }
}

void TaskExecutor::AddMo2TopPackTask(Task& currentTask) {
    if (currentTask.status == TaskStatus::RUNNING){

        currentTask.progress = 0.0f;
        currentTask.status = TaskStatus::RUNNING;

        sn->con.AddLog("[TaskExecutor]: Starting MO2 Top pack task ID: %i", currentTask.id);

        //auto thw = ThreadWrapper::create();
        //thw->beginAsync(
            //[this, &currentTask]() {
            PackMo2TopTaskHandler(&currentTask.progress);
            currentTask.status = TaskStatus::COMPLETED;
            //}
        //);
    }
}