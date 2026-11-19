#pragma once

#ifndef CTASKMANAGER_H
#define CTASKMANAGER_H

#include <iostream>
#include <map>
#include <functional>
#include <string>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <mutex>

// Перечисление статусов задач
enum class TaskStatus {
    PREPARING,   // Ожидает выполнения
    RUNNING,   // Исполняется
    COMPLETED, // Завершена
    CANCELED   // Отменена
};

// Перечисление типов задач
enum class TaskType {
    UNKNOWN,            // Неизвестный тип
    FILE_DOWNLOAD,      // Загрузка файла
    UNPACK_ZIP,        // Распаковка ZIP-архива
    FILE_OPERATION,    // Операция с файлом
    DOWNLOAD_PATCHES,  // Загрузка патчей
    INSTALL_PATCHES,    // Установка патчей
    FILE_UPDATE, // Обновление файла
    HASH_CHECK_MODS, // Проверка модов на валидность, удаление всех невалидных. - обновление
    SCANNING, // Сканирование - обновление
    UPDATE_VERSION,
    SCANNING_RESTORE_ALL, // Восстановление всех модулей.
    SCANNING_RESTORE_COMPLETED, // Завершение задачи выше.    
    SCANNING_AND_BACKUP_EXTERNAL_MODS,
    PACK_ANOMALY_ZIP,
    PACK_MO2_ZIP,
    PACK_MODS_ZIP
};

// Класс, представляющий задачу
class Task {
public:
    uint32_t id;                         // Идентификатор задачи
    TaskStatus status;              // Статус задачи    
    TaskType type;                  // Тип задачи
    float progress; // Прогресс выполнения задачи
    std::function<void()> func;     // Функция, которая будет выполнена    

    std::string filename = "N\\A"; // Имя загружаемого файла

    // Конструктор для инициализации задачи
    Task(uint32_t id, std::function<void()> func, TaskType type);

    Task() = default;
};

// Класс для управления задачами
class TaskExecutor {
public:
    bool init(); // Инициализация модуля

    bool AddTask(std::function<void()> func, TaskType type); // Добавление задачи

    TaskType GetTaskType(uint32_t taskId) const; // Получение типа задачи    
    bool RemoveTaskById(uint32_t taskId); // Удаление задачи по ID
    void ClearAllTasks(); // Очистка всех задач
    TaskStatus GetTaskStatus(uint32_t taskId) const; // Получение статуса задачи
    uint32_t GetTotalTasks() const; // Общее количество задач
    uint32_t GetRemainingTasks() const; // Общее количество оставшихся задач
    uint32_t GetPreparingTasks() const; // Количество ожидающих задач
    uint32_t GetRunningTasks() const; // Количество выполняющихся задач
    uint32_t GetCurrentRunningTaskID() const; // ID текущей выполняемой задачи
    uint32_t GetCompletedTasks() const; // Количество завершенных задач
    float GetProgress() const; // Прогресс выполнения задач

    std::vector<Task> GetAllTasks() const; // Получение всех задач

    void CancelTask(uint32_t taskId); // Отмена задачи по ID
    void UpdateTaskProgress(uint32_t taskId, float progress); // Обновление прогресса задачи

    void SetDownloadFileName(uint32_t taskId, const std::string& filename);

    void RenderTaskTab();
    void RenderProgress();
    void ProcessTask(Task& currentTask);

    // Конструктор
    TaskExecutor();

    // Деструктор
    ~TaskExecutor();

    // Варианты задач
    void AddFileDownload(Task& currentTask); // Метод для добавления задачи на скачивание файла
    void UnpackZipFiles(Task& currentTask); // Метод для распаковки ZIP файлов
    void PerformFileOperation(Task& currentTask); // Метод для выполнения операций с файлами
    void DownloadPatches(Task& currentTask); // Метод для скачивания патчей
    void InstallPatches(Task& currentTask); // Метод для установки патчей
    void DownloadUserFiles(Task& currentTask); // Метод для скачивания пользовательских файлов
    void PerformUserFileOperation(Task& currentTask); // Метод для выполнения пользовательских операций с файлами
    void UpdateFile(Task& currentTask); // Метод для обновления файлов после скачивания

    void AddHashCheckTask(Task& currentTask);//Метод для проверки хешей перед обновлением

    void ScanningTask(Task& currentTask);//Метод для сканирования и скачивания - обновление
    void UpdateVersionTask(Task& currentTask);

    void ScanningAndBackupExternalMods(Task& currentTask);

    void ScanningAndRestoreAllModules(Task& currentTask);
    void ScanningAndRestoreAll_Completed(Task& currentTask);

    void AddAnomalyPackTask(Task& currentTask);
    void AddMo2ModsPackTask(Task& currentTask);
    void AddMo2TopPackTask(Task& currentTask);

    //Синхронизатор
    void TasksUpdateSync();

    void ResetCounters(); // сброс счетчиков

private:
    uint32_t nextTaskId{ 1 };               // Инициализация счетчика задач - 0 -- disabled
    uint32_t currentTaskId;                 // текущий номер задачи
    std::vector<Task> currentTasks; // Вектор для текущих задач
    std::vector<Task> prepareTasks; // Вектор для задач к подготовке    
    const uint32_t MAX_TASKS = 1;  // Максимальное количество задач    

    mutable std::mutex taskMutex;

    bool TaskisExecuting = false; // Флаг выполнения задачи

    std::condition_variable taskCondition;

    static std::unique_ptr<TaskExecutor> create();
    void ProcessPrepareTasks();
    void CreateCurrentTask(const Task& preparedTask);

    std::atomic<bool> cancellationRequested{ false };

    bool stopFlag = false;                 // Флаг остановки цикла TasksUpdateSync
    std::unique_ptr<ThreadWrapper> twInstance; // Храним экземпляр обертки, чтобы контролировать поток

    std::atomic<uint32_t> completedTasksCount{ 0 };
    std::atomic<uint32_t> totalTasksAdded{ 0 }; // Сколько задач мы вообще добавили через AddTask
    //void CompleteTaskSync();// Синхронизация удаления

};

extern std::unique_ptr<TaskExecutor> executor;
#endif // CTASKMANAGER_H