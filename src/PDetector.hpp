#pragma once
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace PDetect {
    class PerformanceTimer {
    private:
        struct TimeRecord {
            std::string name;
            std::chrono::high_resolution_clock::time_point start;
            long long duration = 0;
        };

        struct PerformanceStats {
            double mean = 0.0;
            double median = 0.0;
            double stdDev = 0.0;
            long long min = 0;
            long long max = 0;
        };

        static inline std::vector<TimeRecord> timeStack;
        static inline std::unordered_map<std::string, std::vector<long long>> performanceData;

    public:
        static void BeginCalcTime(const std::string& name) {
            TimeRecord record;
            record.name = name;
            record.start = std::chrono::high_resolution_clock::now();
            timeStack.push_back(record);
        }

        static long long EndCalcTime() {
            if (timeStack.empty()) {
                return 0;
            }

            auto& record = timeStack.back();
            auto end = std::chrono::high_resolution_clock::now();

            record.duration = std::chrono::duration_cast<std::chrono::microseconds>(
                end - record.start
            ).count();

            // Сохраняем результат для статистики
            performanceData[record.name].push_back(record.duration);

            // Вывод текущего замера
            Log("[PERF] %s: %lld us", record.name.c_str(), record.duration);

            long long duration = record.duration;
            timeStack.pop_back();

            return duration;
        }

        // Расчет статистики по замерам
        static PerformanceStats GetPerformanceStats(const std::string& name, int numRuns = 5) {
            auto& measurements = performanceData[name];

            // Если недостаточно измерений
            if (measurements.size() < numRuns) {
                return {};
            }

            PerformanceStats stats;

            // Расчет среднего значения
            stats.mean = std::accumulate(measurements.begin(), measurements.end(), 0.0) / measurements.size();

            // Расчет медианы
            auto medianIt = measurements.begin() + measurements.size() / 2;
            std::nth_element(measurements.begin(), medianIt, measurements.end());
            stats.median = *medianIt;

            // Расчет стандартного отклонения
            double variance = 0.0;
            for (auto& val : measurements) {
                variance += std::pow(val - stats.mean, 2);
            }
            variance /= measurements.size();
            stats.stdDev = std::sqrt(variance);

            // Минимум и максимум
            auto [minIt, maxIt] = std::minmax_element(measurements.begin(), measurements.end());
            stats.min = *minIt;
            stats.max = *maxIt;

            // Вывод статистики US
            /*Log("[PERF STATS] %s:", name.c_str());
            Log("  Mean: %.2f us", stats.mean);
            Log("  Median: %.2f us", stats.median);
            Log("  Std Dev: %.2f us", stats.stdDev);
            Log("  Min: %lld us", stats.min);
            Log("  Max: %lld us", stats.max);*/

            // Вывод статистики RU
            Log("[PERF СТАТИСТИКА] %s:", name.c_str());
            Log("  Среднее: %.2f мкс", stats.mean);
            Log("  Медиана: %.2f мкс", stats.median);
            Log("  Станд. отклонение: %.2f мкс", stats.stdDev);
            Log("  Минимум: %lld мкс", stats.min);
            Log("  Максимум: %lld мкс", stats.max);

            return stats;
        }

        // Очистка данных для конкретного замера
        static void ResetPerformanceData(const std::string& name) {
            performanceData[name].clear();
        }

        // Полная очистка всех накопленных данных
        static void ResetAllPerformanceData() {
            performanceData.clear();
        }
    };
}

// Макросы для удобства использования
#define PERF_BEGIN(name) PDetect::PerformanceTimer::BeginCalcTime(name)
#define PERF_END() PDetect::PerformanceTimer::EndCalcTime()
#define PERF_STATS(name, runs) PDetect::PerformanceTimer::GetPerformanceStats(name, runs)