/*****************************************************
** File    : ThreadWrappper.cpp
** Project : Anomaly BE Engine
** Date    : Created 28-06-20, LAST UPDATED 04-12-25
** Author  : Blue Eclipse Team: 0x5CE3C 
** TELEGRAM ID : https://t.me/anomaly_blue_eclipse
******************************************************/
#include "main.h"
#include "ThreadWrapper.hpp"

void ThreadWrapper::joinAll() {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& threadPtr : threads) {
        if (threadPtr->joinable()) {
            threadPtr->join();
        }
    }
    threads.clear();
}

void ThreadWrapper::detachAll() {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& threadPtr : threads) {
        if (threadPtr->joinable()) {
            threadPtr->detach();
        }
    }
    threads.clear();
}

int ThreadWrapper::GetCountErrors() const {
    return errors;
}

std::unique_ptr<ThreadWrapper> ThreadWrapper::create() {
    return std::unique_ptr<ThreadWrapper>(new ThreadWrapper());
}

ThreadWrapper::~ThreadWrapper() {
    joinAll();    
}



// ==================================== THREAD POOL ==========================//

// Конструктор
ThPool::ThPool(size_t threads) : stop(false), isWorkerActive(threads, true) {
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back(&ThPool::worker, this);
    }
}

// Деструктор
ThPool::~ThPool() {
    stop = true;
    condition.notify_all(); // Уведомляем все потоки

    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join(); // Ждем завершения работы потоков
        }
    }
}

// Получение количества активных потоков
int ThPool::getActiveThreadsCount() {
    int count = 0;
    for (const auto& active : isWorkerActive) {
        if (active) {
            ++count; // Увеличиваем счетчик для каждого активного потока
        }
    }
    return count; // Возвращаем количество активных потоков
}

// Получение общего количества потоков
size_t ThPool::getTotalThreads() {
    return workers.size();
}

// Получение числа свободных потоков
size_t ThPool::getIdleThreads() {
    return std::count(isWorkerActive.begin(), isWorkerActive.end(), false);
}

// Метод worker для выполнения задач
void ThPool::worker() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); }); // Ожидаем задачи

            if (stop && tasks.empty()) {
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
        }

        task(); // Выполнение задачи
    }
}


// Метод для отцепления потока
void ThPool::detachWorker(size_t index) {
    if (index < workers.size() && workers[index].joinable()) {
        workers[index].detach(); // Отцепляем поток
        isWorkerActive[index] = false; // Устанавливаем состояние потока как неактивное
    }
    else {
        std::cerr << "Некорректный индекс или поток уже отцеплен." << std::endl;
    }
}

// Метод для завершения потока
void ThPool::stopWorker(size_t index) {
    if (index < workers.size()) {
        stop = true;
        workers[index].join(); // Ждем завершения потока
        isWorkerActive[index] = false; // Устанавливаем состояние потока как неактивное
    }
    else {
        std::cerr << "Некорректный индекс потока." << std::endl;
    }
}

std::vector<int> ThPool::getAllThreadsIds() {
    std::vector<int> ids;
    std::hash<std::thread::id> hasher;

    for (const auto& worker : workers) {
        ids.push_back(static_cast<int>(hasher(worker.get_id())));
    }

    return ids; // Возвращаем вектор идентификаторов потоков
}

std::vector<bool> ThPool::getWorkerStatus() {
    return isWorkerActive; // Возвращаем статус потоков
}

std::vector<int> ThPool::getActiveThreadIds() {
    std::vector<int> activeIds;
    
        const auto& isWorkerActive = getWorkerStatus(); // Предполагаем, что ThPool имеет метод getWorkerStatus
        std::hash<std::thread::id> hasher;

        for (size_t i = 0; i < isWorkerActive.size(); ++i) {
            if (isWorkerActive[i]) {
                activeIds.push_back(static_cast<int>(hasher(workers[i].get_id())));
            }
        }    
    return activeIds; // Возвращаем идентификаторы активных потоков
}

std::vector<int> ThPool::getIdleThreadIds() {
    std::vector<int> idleIds;
    
        const auto& isWorkerActive = getWorkerStatus(); // Предполагаем, что ThPool имеет метод getWorkerStatus
        std::hash<std::thread::id> hasher;

        for (size_t i = 0; i < isWorkerActive.size(); ++i) {
            if (!isWorkerActive[i]) {
                idleIds.push_back(static_cast<int>(hasher(workers[i].get_id())));
            }
        }    
    return idleIds; // Возвращаем идентификаторы бездействующих потоков
}


// =============================== THREAD POOL WRAPPER ======================//

std::unique_ptr<ThPoolWrapper> thpw;

std::unique_ptr<ThPoolWrapper> ThPoolWrapper::create() {
    return std::unique_ptr<ThPoolWrapper>(new ThPoolWrapper());
}

void ThPoolWrapper::createThreadPool(std::optional<size_t> poolSize) {
    traceLastFunc("void ThPoolWrapper::createThreadPool(size_t poolSize);");
    try {
        if (thPool) {
            std::cerr << "Пул потоков уже создан." << std::endl;
            return;  // Пул уже инициализирован, больше ничего не делаем
        }

        // Используем количество логических процессоров, если poolSize не указано
        size_t actualPoolSize = poolSize.value_or(threads_count);
        thPool = std::make_unique<ThPool>(actualPoolSize);        
        poolInitialized = true;  // Устанавливаем флаг после успешного создания

        Log("void ThPoolWrapper::createThreadPool(size_t poolSize); bthreadPool init [ %i ]", poolInitialized);
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка создания пула потоков: " << e.what() << std::endl;
    }
}

void ThPoolWrapper::destroyThreadPool() {
    thPool.reset(); // Автоматический вызов деструктора ThPool
}

bool ThPoolWrapper::init() {
    traceLastFunc("bool ThPoolWrapper::init();");
    try {
        if (!thpw) {
            thpw = ThPoolWrapper::create(); // Используем метод create для инициализации

            // Проверяем успешность создания
            if (!thpw) {
                std::cerr << "Не удалось создать экземпляр ThPoolWrapper." << std::endl;
                return false;
            }
            Log("bool ThPoolWrapper::init() Initialized");
            return true; // Инициализация прошла успешно
        }
        else {
            std::cerr << "ThPoolWrapper уже инициализирован." << std::endl;
            return true; // Если уже инициализирован, возвращаем true
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка инициализации ThPoolWrapper: " << e.what() << std::endl;
        return false; // Ошибка инициализации
    }
}

void ThPoolWrapper::detachThread(size_t index) {
    if (thPool) {
        thPool->detachWorker(index);
    }
    else {
        std::cerr << "Пул потоков не инициализирован." << std::endl;
    }
}

int ThPoolWrapper::getActiveThreadCount() {
    if (thPool) {
        return thPool->getActiveThreadsCount(); // Возвращаем количество активных потоков
    }
    return 0; // Если пул не инициализирован
}

int ThPoolWrapper::getTotalThreads() {
    if (thPool) {
        return static_cast<int>(thPool->getTotalThreads()); // Возвращаем общее количество потоков
    }
    return 0; // Если пул не инициализирован
}

int ThPoolWrapper::getIdleThreadCount() {
    if (thPool) {
        return static_cast<int>(thPool->getIdleThreads()); // Возвращаем количество свободных потоков
    }
    return 0; // Если пул не инициализирован
}

// Получение идентификаторов потоков
std::vector<int> ThPoolWrapper::getAllThreadIds() {
    if (thPool) {
        return thPool->getAllThreadsIds();
    }
    return {}; // Если пул не инициализирован
}

std::vector<int> ThPoolWrapper::getActiveThreadIds() {
    if (thPool) {
        return thPool->getActiveThreadIds();
    }
    return {}; // Если пул не инициализирован
};


std::vector<int> ThPoolWrapper::getIdleThreadIds() {
    if (thPool) {
        return thPool->getIdleThreadIds();
    }
    return {}; // Если пул не инициализирован
};


ThPoolWrapper::ThPoolWrapper() {};
ThPoolWrapper::~ThPoolWrapper() {};