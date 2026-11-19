/*****************************************************
** File    : ThreadWrappper.hpp
** Project : Anomaly BE Engine
** Date    : Created 28-06-20, LAST UPDATED 04-12-25
** Author  : Blue Eclipse Team: 0x5CE3C 
** TELEGRAM ID : https://t.me/anomaly_blue_eclipse
******************************************************/
#ifndef THREADWRAPPER_HPP
#define THREADWRAPPER_HPP

#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <chrono>
#include <future>
#include <queue>
#include <optional>
#include <condition_variable>
#include <atomic>

class ThPool {
public:
    explicit ThPool(size_t threads);
    ~ThPool();

    template<class F>
    void enqueue(F&& f);
    
    void detachWorker(size_t index); // Новый метод для отцепления потока
    void stopWorker(size_t index); // Метод для завершения потока

    int getActiveThreadsCount(); // Получение количества активных потоков
    size_t getTotalThreads(); // Получение общего числа потоков
    size_t getIdleThreads(); // Получение числа свободных потоков

    std::vector<int> getAllThreadsIds(); // Получение идентификаторов всех потоков
    std::vector<bool> getWorkerStatus(); // Получение статуса потока
    std::vector<int> getActiveThreadIds(); 
    std::vector<int> getIdleThreadIds(); 

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

    void worker();

    std::vector<bool> isWorkerActive; // Состояние активных потоков
};

// Метод для добавления задач в очередь
template<typename F>
void ThPool::enqueue(F&& f) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(std::forward<F>(f)); // Перемещаем задачу в очередь        
    }
    condition.notify_one(); // Уведомляем один поток
}

class ThPoolWrapper {
public:
    ThPoolWrapper();
    ~ThPoolWrapper();

    uint32_t threads_count = std::thread::hardware_concurrency();

    bool init(); // Инициализация модуля
    void createThreadPool(std::optional<size_t> poolSize = std::nullopt);
    void destroyThreadPool();

    template<typename func>
    void CreateTask(func&& task);

    //options
    void detachThread(size_t index); // Новый метод для отцепления потока

    int getActiveThreadCount(); // Получение количества активных потоков
    int getTotalThreads(); // Получение общего числа потоков
    int getIdleThreadCount(); // Получение числа свободных потоков    

    std::vector<int> getAllThreadIds(); // Получение идентификаторов потоков
    std::vector<int> getActiveThreadIds();
    std::vector<int> getIdleThreadIds();

private:
    std::unique_ptr<ThPool> thPool; // Указатель на пул потоков
    bool poolInitialized = false; // Для отслеживания инициализации пула потоков
    static std::unique_ptr<ThPoolWrapper> create();
};

template<typename func>
void ThPoolWrapper::CreateTask(func&& task) {
    thPool->enqueue(std::forward<func>(task)); // Вызываем метод enqueue из ThPool
}

extern std::unique_ptr<ThPoolWrapper> thpw;

class ThreadWrapper{
public:    

    static std::unique_ptr<ThreadWrapper> create();   

    template<typename func>
    void beginThread(func f, int delayMilliseconds = 5);

    template<typename func, class ... args_types>
    void beginThreadex(func f, int delayMilliseconds = 5, args_types ... args);

    template<typename func, class ... args_types>
    auto beginAsync(func f, args_types ... args) -> std::future<decltype(f(args...))>;

    template<typename func>
    auto beginAsync(func f) -> std::future<decltype(f())>;

    void joinAll();
    void detachAll();
    int GetCountErrors() const;   

    ~ThreadWrapper();

private:
    std::vector<std::unique_ptr<std::thread>> threads;
    std::mutex mtx;
    int errors = 0;    
};

template<typename func>
void ThreadWrapper::beginThread(func f, int delayMilliseconds) {
    try {
        auto threadPtr = std::make_unique<std::thread>(f);
        std::lock_guard<std::mutex> lock(mtx);
        threads.push_back(std::move(threadPtr));
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMilliseconds));
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating thread: " << e.what() << std::endl;
        errors++;
    }
}

template<typename func, class ... args_types>
void ThreadWrapper::beginThreadex(func f, int delayMilliseconds, args_types ... args) {
    try {
        auto threadPtr = std::make_unique<std::thread>(std::bind(f, args...));
        std::lock_guard<std::mutex> lock(mtx);
        threads.push_back(std::move(threadPtr));
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMilliseconds));
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating thread: " << e.what() << std::endl;
        errors++;
    }
}

template<typename func, class ... args_types>
auto ThreadWrapper::beginAsync(func f, args_types ... args) -> std::future<decltype(f(args...))> {
    try {
        
        if constexpr (std::is_void_v<decltype(f(args...))>) {
        
            std::promise<void> promise;
            auto future = promise.get_future();

            auto threadPtr = std::make_unique<std::thread>([promise = std::move(promise), f, args...]() mutable {
                try {
                    f(args...);
                    promise.set_value();
                }
                catch (...) {
                    promise.set_exception(std::current_exception());
                }
                });

            std::lock_guard<std::mutex> lock(mtx);
            threads.push_back(std::move(threadPtr));

            return future;
        }
        else {
            std::promise<decltype(f(args...))> promise;
            auto future = promise.get_future();

            auto threadPtr = std::make_unique<std::thread>([promise = std::move(promise), f, args...]() mutable {
                try {
                    promise.set_value(f(args...));
                }
                catch (...) {
                    promise.set_exception(std::current_exception());
                }
                });

            std::lock_guard<std::mutex> lock(mtx);
            threads.push_back(std::move(threadPtr));

            return future;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при создании асинхронной задачи: " << e.what() << std::endl;
        errors++;
        return std::future<decltype(f(args...))>();
    }
}

template<typename func>
auto ThreadWrapper::beginAsync(func f) -> std::future<decltype(f())> {
    using return_type = decltype(f());    // Определяем тип возвращаемого значения функции

    try {
        std::promise<return_type> promise;
        auto future = promise.get_future();

        auto threadPtr = std::make_unique<std::thread>(
            [promise = std::move(promise), f]() mutable {
                try {
                    if constexpr (std::is_same_v<return_type, void>) {
                        f(); // Для void функции
                        promise.set_value(); // Не передавайте аргументы
                    }
                    else {
                        promise.set_value(f()); // Для всех остальных типов
                    }
                }
                catch (...) {
                    promise.set_exception(std::current_exception());
                }
            });

        std::lock_guard<std::mutex> lock(mtx);
        threads.push_back(std::move(threadPtr));

        return future;
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating async task: " << e.what() << std::endl;
        errors++;
        return std::future<return_type>(); // Возвращаем пустое будущее
    }
}

#endif // THREADWRAPPER_HPP