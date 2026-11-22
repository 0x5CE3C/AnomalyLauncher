#include "main.h"
#include "CTaskManager.h"
#include "L_Main.h"
//#include <atomic>

std::unique_ptr<TaskExecutor> executor;

// Конструктор для инициализации объекта Task
Task::Task(uint32_t id, std::function<void()> func, TaskType type)
    : id(id), func(std::move(func)), type(type), status(TaskStatus::PREPARING) {
}

TaskExecutor::TaskExecutor() {
    traceLastFunc("TaskExecutor::TaskExecutor();");
    try {
        sn->con.AddLog("{FFFF00}[Менеджер задач]: Инициализация завершена");

        // Создаем экземпляр обертки
        twInstance = ThreadWrapper::create();

        // Запускаем цикл обработки задач через beginThread (не detach!)
        // beginThread сохраняет указатель на поток внутри twInstance
        twInstance->beginThread([this]() {
            TasksUpdateSync();
            });

    }
    catch (const std::exception& e) {
        Log("[Менеджер задач]: Ошибка инициализации > %s", e.what());
    }
}

// Метод для добавления задачи
bool TaskExecutor::AddTask(std::function<void()> func, TaskType type) {
    traceLastFunc("bool TaskExecutor::AddTask(std::function<void()> func, uint32_t priority, TaskType type);");
    try {
        if (prepareTasks.empty() && currentTasks.empty())
        {
            nextTaskId = 1;
            if (currentTaskId > 1)
            {
                currentTaskId = 0;
            }
        }

        std::lock_guard<std::mutex> lock(taskMutex);
        currentTaskId = nextTaskId++;
        prepareTasks.emplace_back(currentTaskId, std::move(func), type);
        sn->con.AddLog("[Менеджер задач]: Задача с ID %i добавлена в Ожидающие задачи.", currentTaskId);
        taskCondition.notify_one();  // Уведомляем, что появилась новая задача

        // --- ВОТ ЭТО ДОБАВИТЬ ---
        totalTasksAdded.fetch_add(1, std::memory_order_relaxed);
        // ------------------------

        return true;
    }
    catch (const std::exception& e) {
        sn->con.AddLog("Ошибка добавления задачи: %s", e.what());
        return false;
    }
}

void TaskExecutor::CreateCurrentTask(const Task& preparedTask) {
    Task newTask(preparedTask.id, preparedTask.func, preparedTask.type);

    Log("[Creating Task]: ID: %i, Func: %p, Type: %i", preparedTask.id, preparedTask.func, static_cast<uint32_t>(preparedTask.type));

    prepareTasks.erase(prepareTasks.begin()); // Удаляем из prepareTasks
    if (currentTasks.empty())
    {
        currentTasks.push_back(newTask); // Добавляем в текущую задачу    
        Log("[Task Added]: ID: %i", newTask.id);
    }
}

/*void TaskExecutor::TasksUpdateSync() {
    while (true) {

        std::unique_lock<std::mutex> lock(taskMutex);
        taskCondition.wait(lock, [&]() { return !prepareTasks.empty(); }); // Ожидаем появление задач

        if (!prepareTasks.empty() && currentTasks.empty()) {
            currentTasks.push_back(prepareTasks.front());
            prepareTasks.erase(prepareTasks.begin());
            sn->con.AddLog("[Менеджер задач]: Задача с ID %i добавлена в Активные задачи.", currentTasks.back().id);
        }

        lock.unlock(); // Освобождаем мьютекс перед выполнением задачи                    
        ExecuteTasks();

        //CompleteTaskSync();
    }
};*/

void TaskExecutor::TasksUpdateSync() {
    while (true) {
        std::unique_lock<std::mutex> lock(taskMutex);

        // Ждем: либо появилась задача, либо попросили остановиться (stopFlag)
        taskCondition.wait(lock, [this]() {
            return !prepareTasks.empty() || stopFlag;
            });

        if (stopFlag) {
            break; // Корректный выход из цикла
        }

        // Если нет активной задачи, берем первую из очереди подготовки
        if (currentTasks.empty() && !prepareTasks.empty()) {
            // Перемещаем задачу (move гарантирует отсутствие лишних копий)
            currentTasks.push_back(std::move(prepareTasks.front()));
            prepareTasks.erase(prepareTasks.begin());
            //sn->con.AddLog("[Менеджер задач]: Нет активных задач, запуск из очереди подготовки.");
            //sn->con.AddLog("[Менеджер задач]: Задача ID %i перемещена в Ожидающие задачи.", currentTasks.back().id);
            //currentTasks.back().status = TaskStatus::RUNNING;
        }

        lock.unlock(); // !!! КРИТИЧНО: Освобождаем мьютекс ПЕРЕД тяжелой работой (curl/7za)

        // Выполняем задачу прямо здесь, в этом потоке.
        // ProcessTask сам поставит статус COMPLETED или CANCELED в конце.
        if (!currentTasks.empty()) {
            ProcessTask(currentTasks.front());
        }

        lock.lock(); // Снова захватываем мьютекс для безопасного удаления

        if (!currentTasks.empty()) {
            Task& task = currentTasks.front();

            // 1. Увеличиваем глобальный счётчик завершённых
            /*if (task.status == TaskStatus::COMPLETED) {
                completedTasksCount++;
            }*/

            // Если задача завершена (успешно или с ошибкой) — удаляем её из вектора
            if (task.status == TaskStatus::COMPLETED || task.status == TaskStatus::CANCELED) {
                currentTasks.erase(currentTasks.begin());
                sn->con.AddLog("[Менеджер задач]: Задача ID %i завершена (статус: %d).", task.id, static_cast<int>(task.status));
            }
            // Если статус RUNNING (редкий случай, если ProcessTask делает что-то асинхронное внутри себя),
            // задача остается в векторе до следующей итерации.
        }
    }
}

void TaskExecutor::ProcessTask(Task& currentTask) {
    try {
        if (currentTask.status == TaskStatus::PREPARING)
        {
            currentTask.status = TaskStatus::RUNNING;
            // Обработка в зависимости от типа задачи
            switch (currentTask.type) {
            case TaskType::FILE_DOWNLOAD:
                AddFileDownload(currentTask); // Метод для загрузки файла
                break;
            case TaskType::UNPACK_ZIP:
                UnpackZipFiles(currentTask); // Метод для распаковки ZIP
                break;
            case TaskType::FILE_OPERATION:
                PerformFileOperation(currentTask); // Метод для операций с файлами
                break;
            case TaskType::DOWNLOAD_PATCHES:
                DownloadPatches(currentTask); // Метод для загрузки патчей
                break;
            case TaskType::INSTALL_PATCHES:
                InstallPatches(currentTask); // Метод для установки патчей
                break;
            case TaskType::FILE_UPDATE:
                UpdateFile(currentTask); // Метод для обновления файла
                break;
            case TaskType::HASH_CHECK_MODS:
                AddHashCheckTask(currentTask); // Метод для сравнения хешей Модов
                break;
            case TaskType::SCANNING:
                ScanningTask(currentTask);
                break;
            case TaskType::UPDATE_VERSION:
                UpdateVersionTask(currentTask);
                break;
            case TaskType::SCANNING_AND_BACKUP_EXTERNAL_MODS:
                ScanningAndBackupExternalMods(currentTask);
                break;
            case TaskType::SCANNING_RESTORE_ALL:                
                ScanningAndRestoreAllModules(currentTask);
                break;
            //case TaskType::SCANNING_RESTORE_COMPLETED:
                //Log(u8"Executing task ID: %i TASK_TYPE_SCANNING_RESTORE_ALL", currentTask.id);
                //ScanningAndRestoreAllModules(currentTask);
                //break;
            case TaskType::PACK_ANOMALY_ZIP:
                //Log(u8"Executing task ID: %i TASK_TYPE_SCANNING_RESTORE_ALL", currentTask.id);    
                AddAnomalyPackTask(currentTask);
                break;
            case TaskType::PACK_MO2_ZIP:
                //Log(u8"Executing task ID: %i TASK_TYPE_SCANNING_RESTORE_ALL", currentTask.id);    
                AddMo2TopPackTask(currentTask);
                break;
            case TaskType::PACK_MODS_ZIP:
                //Log(u8"Executing task ID: %i TASK_TYPE_SCANNING_RESTORE_ALL", currentTask.id);  
                AddMo2ModsPackTask(currentTask);
                break;


            default:
                Log("[Менеджер задач]: Неизвестный тип задачи с ID: %i | Отмена задачи", currentTask.id);
                currentTask.status = TaskStatus::CANCELED; // Устанавливаем статус CANCELED для неизвестной задачи
                return; // Выходим, не обновляя статус на COMPLETED
            }
        }        
    }
    catch (const std::exception& e) {
        sn->con.AddLog("[Менеджер задач]: Ошибка обработки задачи: %s", e.what());
        currentTask.status = TaskStatus::CANCELED; // Устанавливаем статус CANCELED в случае ошибки
    }
}


/*void TaskExecutor::CompleteTaskSync() {
    try
    {
        for (const auto& task : currentTasks) {
            if (task.status == TaskStatus::COMPLETED) {
                 switch (task.type) 
                 {
                    case TaskType::FILE_DOWNLOAD:
                    {
                        // task.status = TaskStatus::CANCELED;
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        break;
                    }
                    case TaskType::UNPACK_ZIP:
                    {
                        // task.status = TaskStatus::CANCELED;
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        break;
                    }
                    case TaskType::FILE_OPERATION:
                    {
                        //task.status = TaskStatus::CANCELED;
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        break;
                    }
                    case TaskType::DOWNLOAD_PATCHES:
                    {
                        //task.status = TaskStatus::CANCELED;
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        break;
                    }
                    case TaskType::INSTALL_PATCHES:
                    {
                        //task.status = TaskStatus::CANCELED;
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        break;
                    }
                    case TaskType::FILE_UPDATE:
                    {
                        //task.status = TaskStatus::CANCELED;
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        break;
                    }
                    case TaskType::HASH_CHECK_MODS:
                    {
                        //task.status = TaskStatus::CANCELED;
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        break;
                    }
                    case TaskType::SCANNING:
                    {
                        //task.status = TaskStatus::CANCELED;
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        break;
                    }
                    case TaskType::UPDATE_VERSION:
                    {
                        // task.status = TaskStatus::CANCELED;
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        break;
                    }
                    case TaskType::SCANNING_AND_BACKUP_EXTERNAL_MODS:
                    {
                        //  task.status = TaskStatus::CANCELED;
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        break;
                    }
                    case TaskType::SCANNING_RESTORE_ALL:
                    {
                        //   task.status = TaskStatus::CANCELED;
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Восстановление модулей.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Восстановление модулей.", task.id);
                        break;
                    }
                    case TaskType::SCANNING_RESTORE_COMPLETED:
                    {
                        Log("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        sn->con.AddLog("[Менеджер задач]: Текущая задача завершена: ID %i, Тип: Скачивание файла.", task.id);
                        break;
                    }

                    default:
                        Log("[Менеджер задач]: Неизвестный тип задачи с ID: %i | Отмена текущей задачи", task.id);
                        break; // Выходим, не обновляя статус на COMPLETED              
                 }
                 currentTasks.erase(task);
            }
        }        
    }
    catch (const std::exception&)
    {

    }
}*/

// Метод для получения типа задачи по ID
TaskType TaskExecutor::GetTaskType(uint32_t taskId) const {
    traceLastFunc("TaskType TaskExecutor::GetTaskType(uint32_t taskId);");
    try {
        for (const auto& task : currentTasks) {
            if (task.id == taskId) {
                return task.type; // Возвращаем тип задачи
            }
        }
        return TaskType::UNKNOWN; // Если задача не найдена, возвращаем UNKNOWN
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка получения типа задачи ID " << taskId << ": " << e.what() << std::endl;
        return TaskType::UNKNOWN; // Ошибка возврата UNKNOWN
    }
}

// Метод для удаления задачи по ID
bool TaskExecutor::RemoveTaskById(uint32_t taskId) {
    traceLastFunc("bool TaskExecutor::RemoveTaskById(uint32_t taskId);");
    try {
        std::lock_guard<std::mutex> lock(taskMutex);

        auto itPrepare = std::find_if(prepareTasks.begin(), prepareTasks.end(),
            [taskId](const Task& t) { return t.id == taskId; });

        if (itPrepare != prepareTasks.end()) {
            prepareTasks.erase(itPrepare);
            sn->con.AddLog("[Менеджер задач]: Задача удалена из очереди: ID = %i", taskId);
            return true;
        }

        auto itCurrent = std::find_if(currentTasks.begin(), currentTasks.end(),
            [taskId](const Task& t) { return t.id == taskId; });

        if (itCurrent != currentTasks.end()) {
            // Можно дополнительно проверить тип и сбросить флаги, если это сканирование
            if (itCurrent->type == TaskType::SCANNING_RESTORE_ALL) {
                cancellationRequested = true;
                scanner_extractor_enable = false;
                scanner_anomaly_enable = false;
                scanner_mo2_enable = false;
                scanner_be_enable = false;
                if (sn->download_system.enabled) sn->download_system.enabled = false;
                if (sn->scanhashes.enabled) sn->scanhashes.enabled = false;
                if (sn->scanhashes_dirs.enabled) sn->scanhashes_dirs.enabled = false;
            }

            currentTasks.erase(itCurrent);
            sn->con.AddLog("[Менеджер задач]: Задача удалена из активных: ID = %i", taskId);
            return true;
        }
    }
    catch (const std::exception& e) {
        Log("[Менеджер задач]: Ошибка при удалении задачи: %s", e.what());
        sn->con.AddLog("[Менеджер задач]: Ошибка при удалении задачи: %s", e.what());
        return false; // Ошибка удаления
    }
}

// Метод для очистки всех задач
void TaskExecutor::ClearAllTasks() {
    traceLastFunc("void TaskExecutor::ClearAllTasks();");

    std::lock_guard<std::mutex> lock(taskMutex);

    prepareTasks.clear();
    currentTasks.clear();


    // Сначала останавливаем скачивание
    auto httpClient = HttpClient::getInstance();
    if (httpClient) {
        httpClient->stopDownload();
    }


    // Сбрасываем флаги отмены, если они есть
    cancellationRequested = true;
    scanner_extractor_enable = false;
    scanner_anomaly_enable = false;
    scanner_mo2_enable = false;
    scanner_be_enable = false;
    AbortCurrentDebugTask();

    sn->con.AddLog("{FFFF00}[Менеджер задач]: Все задачи очищены.");
}

// Метод для получения статуса задачи по ID
TaskStatus TaskExecutor::GetTaskStatus(uint32_t taskId) const {
    traceLastFunc("TaskStatus TaskExecutor::GetTaskStatus(uint32_t taskId);");
    try {
        for (const auto& task : currentTasks) {
            if (task.id == taskId) {
                return task.status; // Возвращаем статус задачи
            }
        }
        return TaskStatus::CANCELED; // Если задача не найдена, возвращаем CANCELED
    }
    catch (const std::exception& e) {
        Log("[Менеджер задач]: Ошибка получения статуса задачи ID %s", e.what());
        return TaskStatus::CANCELED; // Ошибка возврата CANCELED
    }
}

// Метод для получения общего количества задач
uint32_t TaskExecutor::GetTotalTasks() const {
    traceLastFunc("uint32_t TaskExecutor::GetTotalTasks();");
    try {
        return currentTasks.size() + prepareTasks.size() + GetCompletedTasks(); // Общее количество задач, включая временное хранилище
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка получения общего количества задач: " << e.what() << std::endl;
        return 0; // Ошибка возврата 0
    }
}


uint32_t TaskExecutor::GetRemainingTasks() const {
    traceLastFunc("uint32_t TaskExecutor::GetTotalTasks();");
    try {
        return currentTasks.size() + prepareTasks.size(); // Общее количество задач, включая временное хранилище
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка получения общего количества задач: " << e.what() << std::endl;
        return 0; // Ошибка возврата 0
    }
}



// Метод для получения количества ожидающих задач
uint32_t TaskExecutor::GetPreparingTasks() const {
    traceLastFunc("uint32_t TaskExecutor::GetPreparingTasks();");
    try {
        int count = 0;
        for (const auto& task : prepareTasks) {
            if (task.status == TaskStatus::PREPARING) {
                count++; // Счетчик ожидающих задач
            }
        }
        return count; // Возвращаем количество ожидающих задач
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка получения количества ожидающих задач: " << e.what() << std::endl;
        return 0; // Ошибка возврата 0
    }
}

// Метод для получения количества выполняемых задач
uint32_t TaskExecutor::GetRunningTasks() const {
    traceLastFunc("uint32_t TaskExecutor::GetRunningTasks();");
    try {
        int count = 0;
        for (const auto& task : currentTasks) {
            if (task.status == TaskStatus::RUNNING) {
                count++; // Счетчик выполняемых задач
            }
        }
        return count; // Возвращаем количество выполняемых задач
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка получения количества выполняемых задач: " << e.what() << std::endl;
        return 0; // Ошибка возврата 0
    }
}


// Метод для получения количества завершенных задач
uint32_t TaskExecutor::GetCompletedTasks() const {
    traceLastFunc("uint32_t TaskExecutor::GetCompletedTasks();");
    try {
        uint32_t remaining = 0;

        {
            std::lock_guard<std::mutex> lock(taskMutex);
            remaining = static_cast<uint32_t>(prepareTasks.size() + currentTasks.size());
        }

        uint32_t total = totalTasksAdded.load(std::memory_order_relaxed);

        if (remaining >= total) return 0;
        return total - remaining;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка получения количества завершенных задач: " << e.what() << std::endl;
        return 0; // Ошибка возврата 0
    }
}


// Метод для получения ID текущей выполняемой задачи
uint32_t TaskExecutor::GetCurrentRunningTaskID() const {
    std::lock_guard<std::mutex> lock(taskMutex);

    for (const auto& task : currentTasks) {
        if (task.status == TaskStatus::RUNNING) {
            return task.id;
        }
    }
    return 0; // Нет выполняемой задачи
}

void TaskExecutor::ResetCounters() {
    totalTasksAdded.store(0, std::memory_order_release);
    completedTasksCount.store(0, std::memory_order_release);
    sn->con.AddLog("{00FF00}[Менеджер задач]: Счётчики задач сброшены.");
}

// Метод для получения прогресса выполнения задач
float TaskExecutor::GetProgress() const {
    traceLastFunc("float TaskExecutor::GetProgress();");

    uint32_t total = totalTasksAdded.load(std::memory_order_acquire);
    if (total == 0) return 0.0f;

    float completedPoints = 0.0f;

    {
        std::lock_guard<std::mutex> lock(taskMutex);

        uint32_t finishedCount = total;
        finishedCount -= static_cast<uint32_t>(prepareTasks.size());
        finishedCount -= static_cast<uint32_t>(currentTasks.size());

        // Полностью завершённые задачи дают 100% каждая
        completedPoints += static_cast<float>(finishedCount) * 100.0f;

        for (const auto& task : currentTasks) {
            if (task.status == TaskStatus::COMPLETED) {
                completedPoints += 100.0f;
            }
            else if (task.progress > 0.0f) {
                // Если задача сама сообщает прогресс (curl callback)
                completedPoints += task.progress;
            }
            else {
                // Нет прогресса:
                // - Если задача реально в работе (RUNNING) — даём небольшой бонус, чтобы полоска не стояла
                // - Если PREPARING — 0%
                if (task.status == TaskStatus::RUNNING) {
                    completedPoints += 5.0f;  // Маленький бонус, чтобы было видно движение
                }
                else {
                    // PREPARING или неизвестный статус — считаем за 0%
                    completedPoints += 0.0f;
                }
            }
        }
    }

    float progress = completedPoints / (static_cast<float>(total) * 100.0f);
    return std::clamp(progress * 100.0f, 0.0f, 100.0f);
}

std::vector<Task> TaskExecutor::GetAllTasks() const {
    std::unique_lock<std::mutex> lock(taskMutex); // Используем unique_lock
    std::vector<Task> allTasks;

    // Получаем копии текущих и ожидающих задач
    allTasks.insert(allTasks.end(), currentTasks.begin(), currentTasks.end());
    allTasks.insert(allTasks.end(), prepareTasks.begin(), prepareTasks.end());

    return allTasks; // Возвращаем все задачи
}

void TaskExecutor::CancelTask(uint32_t taskId) {
    std::lock_guard<std::mutex> lock(taskMutex);
    bool found = false;

    auto cancelInVector = [&](std::vector<Task>& vec) {
        for (auto& task : vec) {
            if (task.id == taskId) {
                found = true;
                task.status = TaskStatus::CANCELED;

                if (task.type == TaskType::SCANNING_RESTORE_ALL) {
                    cancellationRequested = true;
                    scanner_extractor_enable = false;
                    scanner_anomaly_enable = false;
                    scanner_mo2_enable = false;
                    scanner_be_enable = false;
                    AbortCurrentDebugTask();
                    if (sn->download_system.enabled) sn->download_system.enabled = false;
                    if (sn->scanhashes.enabled) sn->scanhashes.enabled = false;
                    if (sn->scanhashes_dirs.enabled) sn->scanhashes_dirs.enabled = false;
                    sn->con.AddLog("{AAFFFF}Восстановление модулей отменено.");
                }
                break;
            }
        }
    };

    cancelInVector(currentTasks);
    cancelInVector(prepareTasks);

    if (found) {
        sn->con.AddLog("[Менеджер задач]: Задача отменена: ID = %i", taskId);
    }
    else {
        sn->con.AddLog("[Менеджер задач]: Не удалось найти задачу для отмены: ID = %i", taskId);
    }
}


void TaskExecutor::UpdateTaskProgress(uint32_t taskId, float progress) {
    std::lock_guard<std::mutex> lock(taskMutex);
    for (auto& task : currentTasks) {
        if (task.id == taskId) {
            task.progress = progress;
            Log("[Менеджер задач]: Прогресс задачи ID %u обновлен: %.2f", taskId, progress);
            break;
        }
    }
}


void TaskExecutor::SetDownloadFileName(uint32_t taskId, const std::string& filename) {
    std::lock_guard<std::mutex> lock(taskMutex);
    for (auto& task : currentTasks) {
        if (task.id == taskId && task.type == TaskType::FILE_DOWNLOAD) {
            task.filename = filename;
            Log("[Менеджер задач]: Имя файла для задачи ID %u установлено: %s", taskId, filename.c_str());
            break;
        }
    }
}

std::unique_ptr<TaskExecutor> TaskExecutor::create() {
    return std::unique_ptr<TaskExecutor>(new TaskExecutor());
}

bool TaskExecutor::init() {
    traceLastFunc("bool TaskExecutor::init();");
    try {
        if (!executor) {
            executor = TaskExecutor::create(); // Используем метод create для инициализации

            // Проверяем успешность создания
            if (!executor) {
                std::cerr << "Не удалось создать экземпляр TaskExecutor." << std::endl;
                return false;
            }
            Log("bool TaskExecutor::init() Initialize");
            return true; // Инициализация прошла успешно
        }
        else {
            std::cerr << "TaskExecutor уже инициализирован." << std::endl;
            return true; // Если уже инициализирован, возвращаем true
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка инициализации TaskExecutor: " << e.what() << std::endl;
        return false; // Ошибка инициализации
    }
}

TaskExecutor::~TaskExecutor()
{
    traceLastFunc("TaskExecutor::~TaskExecutor();");
    try {
        if (!twInstance) return;

        // 1. Просим поток остановиться
        {
            std::lock_guard<std::mutex> lock(taskMutex);
            stopFlag = true;
        }

        // 2. Будим поток, если он спит на condition_variable (ждет новую задачу)
        taskCondition.notify_one();

        // 3. Ждем завершения потока
        // joinAll() дождется всех потоков, которые были созданы через этот экземпляр twInstance
        twInstance->joinAll();

        Log("[Менеджер задач]: Поток менеджера корректно остановлен.");
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка деструктора TaskExecutor: " << e.what() << std::endl;
    }
}