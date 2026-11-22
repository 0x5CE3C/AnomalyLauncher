#include "main.h"

// Метод для создания вкладки с задачами
void TaskExecutor::RenderTaskTab() {
    std::vector<Task> tasks = executor->GetAllTasks(); // Используем глобальный экземпляр  
    float totalProgress = 0.0f;
    int completedCount = 0;

    ImGui::BeginChild(u8"Задачи");

    ImGui::Text(u8"Контроль:");

    ImGui::SameLine();

    if (ImGui::Button(u8"Удалить все задачи")) {         
        try {
            executor->ClearAllTasks();
            // Сначала останавливаем скачивание
            /*auto httpClient = HttpClient::getInstance();
            if (httpClient) {
                httpClient->ResetDownloadState();
            }*/
         }
         catch (std::exception& e) {
               Log("%s", e.what());
               sn->con.AddLog("%s", e.what());
          }
         catch (std::runtime_error& e) {
             Log("%s", e.what());
             sn->con.AddLog("%s", e.what());
         }
    }    

    // Вот тут честно показываем, почему полоска двигается редко:
    uint32_t preparing = GetPreparingTasks();      // из prepareTasks
    uint32_t running = GetRunningTasks();          // из runningTasks
    uint32_t completed = GetCompletedTasks();

    uint32_t all = GetTotalTasks();

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.60, 0.85, 0.00, 2.00), u8"Задачи > %i ", all);
    ImGui::SameLine();
    ImGui::Text(u8"Выполнено: %d", completed);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8" | ");
    ImGui::SameLine();
    ImGui::Text(u8"В очереди: %u", preparing);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(2.00, 2.00, 2.00, 2.00), u8" | ");
    ImGui::SameLine();
    ImGui::Text(u8"Запущено: %u", running);

    ImGui::Separator();

    float prog = GetProgress(); // Теперь это только (COMPLETED / TOTAL)
    if (prog >= 100.0f) { ResetCounters(); }

    if (sn->disable_update_modpack_button.enabled)
    {        
        ImGui::Text(u8"Общий прогресс выполнения: %.1f%%", prog);
        ImGui::ProgressBar(prog / 100.0f, ImVec2(-1.0f, 0.0f));
    }

    ImGui::Separator();

    // Таблица для отображения задач
    ImGui::Columns(5, "taskColumns", true);
    ImGui::Text("ID");
    ImGui::NextColumn();
    ImGui::Text(u8"Тип");
    ImGui::NextColumn();
    ImGui::Text(u8"Статус");
    ImGui::NextColumn();
    ImGui::Text(u8"Прогресс");
    ImGui::NextColumn();
    ImGui::Text(u8"Действие");
    ImGui::NextColumn();
    ImGui::Separator();

    for (const Task& task : tasks) {
        ImGui::Text("%d", task.id);
        ImGui::NextColumn();
        ImGui::Text(u8"%s", (
            task.type == TaskType::FILE_DOWNLOAD ? u8"Скачивание" :
            task.type == TaskType::UNPACK_ZIP ? u8"Распаковка" : 
            task.type == TaskType::PACK_MO2_ZIP ? u8"Запаковка MO2" : 
            task.type == TaskType::PACK_ANOMALY_ZIP ? u8"Запаковка Anomaly" : 
            task.type == TaskType::PACK_MODS_ZIP ? u8"Запаковка Модов" : 
            task.type == TaskType::HASH_CHECK_MODS ? u8"Сравнение хешей" : 
            task.type == TaskType::UPDATE_VERSION ? u8"Обновление версии" : 
            task.type == TaskType::SCANNING ? u8"Сканирование" : 
            task.type == TaskType::SCANNING_RESTORE_ALL ? u8"Восстановление модулей" : 
            task.type == TaskType::SCANNING_RESTORE_COMPLETED ? u8"Завершение восстановления" : 
            task.type == TaskType::FILE_UPDATE ? u8"Обновление файла" : u8"unknown")); // Приводим тип к int для отображения
        ImGui::NextColumn();

        ImGui::Text(u8"%s", (task.status == TaskStatus::RUNNING ? u8"Выполняется" :
            task.status == TaskStatus::COMPLETED ? u8"Завершена" :
            task.status == TaskStatus::CANCELED ? u8"Отменена" :
            u8"Ожидает"));
        ImGui::NextColumn();

        // Отображение прогресс-бара для каждой задачи
        if (task.status == TaskStatus::RUNNING)
        {
            ImGui::ProgressBar(task.progress / 100, ImVec2(0.0f, 0.0f));
        }
        ImGui::NextColumn();

        ImGui::PushID(task.id);
        if(task.status == TaskStatus::PREPARING && task.status != TaskStatus::CANCELED)
        {
            if (ImGui::Button(u8"Отмена")) {
                executor->CancelTask(task.id);
            }
        }        
        else if((task.status == TaskStatus::RUNNING && task.type == TaskType::FILE_DOWNLOAD) && task.status != TaskStatus::CANCELED)
        {
            if (ImGui::Button(u8"Отмена")) {
                executor->CancelTask(task.id);
                HttpClient::getInstance()->stopDownload();                
                sn->con.AddLog("[TaskExecutor]: Скачивание приостановлено для задачи с ID %d.", task.id);
            }            
        }
        else if ((task.status == TaskStatus::RUNNING && task.type != TaskType::FILE_DOWNLOAD) && task.status != TaskStatus::CANCELED)
        {
            if (ImGui::Button(u8"Отмена")) {
                executor->CancelTask(task.id);
            }
        }
        else if (task.status == TaskStatus::CANCELED)
        {
            if (ImGui::Button(u8"Удалить")) {
                executor->RemoveTaskById(task.id);
            }
        }
        ImGui::PopID();
        ImGui::NextColumn();
    }

    ImGui::EndChild();
}

void TaskExecutor::RenderProgress()
{    
    // Берём фиксированное общее количество задач из менеджера
    uint32_t totalTasks = executor->GetTotalTasks();
    uint32_t completedCount = executor->GetCompletedTasks(); // Если такого метода нет — см. ниже

    float totalProgress = 0.0f;

    if (totalTasks > 0) {
        totalProgress = (static_cast<float>(completedCount) / static_cast<float>(totalTasks)) * 100.0f;
    }
    else {
        totalProgress = 0.0f;
    }

};