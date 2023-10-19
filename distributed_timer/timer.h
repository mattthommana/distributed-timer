#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <mutex>
#include <map>
#include <limits>
#include <thread>
#include <filesystem>
#include <cstring>
#include <nlohmann/json.hpp>

using ClockType = std::chrono::high_resolution_clock;
namespace fs = std::filesystem;

struct raw_event_t {
    std::string name;
    std::string cat;
    void *id;
    int64_t ts;
    uint32_t pid;
    uint32_t tid;
    char ph;
    std::unordered_map<std::string, std::string> args; 
};



/**
 * @brief Enumeration of timer operation types.
 */
enum class TimerOperation{
    Disabled, ///< Timer operation is disabled.
    Chrome,   ///< Timer operation type is Chrome.
    Firefox,  ///< Timer operation type is Firefox.
    CSV       ///< Timer operation type is CSV.
};

/**
 * @brief Class that provides functionalities to time and log various events.
 */
class Timer
{
private:
    std::mutex memoryMutex_;
    fs::path outputPath_;
    std::ofstream outputFile_;
    std::vector<raw_event_t> inMemoryLogs_;
    TimerOperation operation_;

    // Starts logging an event.
    void _start(const std::string &event_category, const std::string &event_name, const std::unordered_map<std::string, std::string> &args={});

    // Stops logging an event.
    void _stop(const std::string &event_category, const std::string &event_name, const std::unordered_map<std::string, std::string> &args={});

    // Core function to add counter event.
    void _coreAddCounterEvent(const std::string &event_category, const std::string &event_name, size_t value, const std::unordered_map<std::string, std::string> &args={});

public:
    /**
     * @brief Construct a new Timer object.
     * 
     * @param outputPath Path where the logs should be saved.
     * @param operation Operation type of the timer.
     */
    Timer(const std::string &outputPath, TimerOperation operation=TimerOperation::Chrome);

    /**
     * @brief Set the operation type of the timer.
     * 
     * @param operation The type of operation to be set.
     */
    void setOperation(const TimerOperation operation);

    /**
     * @brief Get the current operation type of the timer.
     * 
     * @return TimerOperation Current operation type.
     */
    TimerOperation getOperation() const ;

    /**
     * @brief Add a counter event to the log.
     * 
     * @param event_category The category of the event.
     * @param event_name Name of the event.
     * @param value Numeric value to be associated with the counter event.
     * @param args Additional arguments to be logged.
     */
    void addCounterEvent(const std::string &event_category, const std::string &event_name, size_t value, const std::unordered_map<std::string, std::string> &args={});

    /**
     * @brief Start timing and logging an event.
     * 
     * @param event_category The category of the event.
     * @param event_name Name of the event.
     * @param args Additional arguments to be logged.
     * @param measureMemory Flag to indicate whether to measure memory usage or not.
     */
    void start(const std::string &event_category, const std::string &event_name, const std::unordered_map<std::string, std::string> &args, bool measureMemory=true);

    /**
     * @brief Stop timing and logging an event.
     * 
     * @param event_category The category of the event.
     * @param event_name Name of the event.
     * @param args Additional arguments to be logged.
     * @param measureMemory Flag to indicate whether to measure memory usage or not.
     */
    void stop(const std::string &event_category, const std::string &event_name, const std::unordered_map<std::string, std::string> &args, bool measureMemory=true);

    /**
     * @brief Dump all logged events to the output file.
     */
    void dumpLogs();
};
