#include "timer.h"
#include "utils.h" // getCurrentMemoryDraw

void Timer::_start(const std::string &event_category, const std::string &event_name, const std::unordered_map<std::string, std::string> &argsMap) {
    if (operation_ != TimerOperation::Disabled) {
        int64_t ts_start_micros = ClockType::now().time_since_epoch().count() / 1000;
        size_t categoryHash = std::hash<std::string>{}(event_category);

        raw_event_t event;
        event.name = event_name;
        event.cat = event_category;
        event.pid = 1;
        event.tid = categoryHash;
        event.ts = ts_start_micros;
        event.ph = 'B';
        event.args = argsMap;

        inMemoryLogs_.push_back(event);
    }
}

void Timer::_stop(const std::string &event_category, const std::string &event_name, const std::unordered_map<std::string, std::string> &argsMap) {
    if (operation_ != TimerOperation::Disabled) {
        int64_t ts_stop_micros = ClockType::now().time_since_epoch().count() / 1000;
        size_t categoryHash = std::hash<std::string>{}(event_category);

        raw_event_t event;
        event.name = event_name;
        event.cat = event_category;
        event.pid = 1;
        event.tid = categoryHash;
        event.ts = ts_stop_micros;
        event.ph = 'E';
        event.args = argsMap;

        inMemoryLogs_.push_back(event);
    }
}


void Timer::_coreAddCounterEvent(const std::string &event_category, const std::string &event_name, size_t value, const std::unordered_map<std::string, std::string> &args) {
    if (operation_ != TimerOperation::Disabled) {
        int64_t ts_micros = ClockType::now().time_since_epoch().count() / 1000;
        size_t categoryHash = std::hash<std::string>{}(event_category);

        raw_event_t event;
        event.name = event_name;
        event.cat = event_category;
        event.pid = 2;
        event.tid = categoryHash;
        event.ts = ts_micros;
        event.ph = 'C';
        event.args = args;
        event.args[event_name] = std::to_string(value); // adding the counter value

        inMemoryLogs_.push_back(event);
    }
}


Timer::Timer(const std::string &outputPath, TimerOperation operation) : outputPath_(fs::path(outputPath)), operation_(operation)
{
    if (operation_ != TimerOperation::Disabled)
    {
        _start("Timer", "Constructor", {});
        fs::create_directories(outputPath_.parent_path());
        outputFile_.open(outputPath_);
        if (!outputFile_.is_open())
            std::cerr << "Failed to open file: " << outputPath_ << ". Error: " << std::strerror(errno) << std::endl;
        _stop("Timer", "Constructor", {});
    }
}

void Timer::setOperation(const TimerOperation operation){
    operation_ = operation;
}

TimerOperation Timer::getOperation() const {
    return operation_;
}

void Timer::addCounterEvent(const std::string &event_category, const std::string &event_name, size_t value, const std::unordered_map<std::string, std::string> &args)
{
    if (operation_ != TimerOperation::Disabled)
    {
        _start("Timer", "addCounterEvent Function", {});
        std::lock_guard<std::mutex> lock(memoryMutex_);
        _coreAddCounterEvent(event_category, event_name, value, args);
        _stop("Timer", "addCounterEvent Function", {});
    }
}

void Timer::start(const std::string &event_category, const std::string &event_name, const std::unordered_map<std::string, std::string> &args, bool measureMemory)
{
    if (operation_ != TimerOperation::Disabled)
    {
        _start("Timer", "start Function", {});
        std::lock_guard<std::mutex> lock(memoryMutex_);
        if (measureMemory)
        {
            size_t memoryDrawBytes = getCurrentMemoryDraw();
            _coreAddCounterEvent(event_category, event_category + " Memory", memoryDrawBytes, args);
        }
        
        std::string real_event_name = (operation_==TimerOperation::Firefox)? event_category: event_name; // To keep everything on the same line all event_names must be the same for firefox
        
        _start(event_category, real_event_name, args);
        _stop("Timer", "start Function", {});
    }
}

void Timer::stop(const std::string &event_category, const std::string &event_name, const std::unordered_map<std::string, std::string> &args, bool measureMemory)
{
    if (operation_ != TimerOperation::Disabled)
    {
        _start("Timer", "stop Function", {});
        std::lock_guard<std::mutex> lock(memoryMutex_);
        if (measureMemory)
        {
            size_t memoryDrawBytes = getCurrentMemoryDraw();
            _coreAddCounterEvent(event_category, event_category + " Memory", memoryDrawBytes, args);
        }
        std::string real_event_name = (operation_==TimerOperation::Firefox)? event_category: event_name; // To keep everything on the same line all event_names must be the same for firefox
        _stop(event_category, real_event_name, args);
        _stop("Timer", "stop Function", {});
    }
}

void Timer::dumpLogs() {
    if (operation_ != TimerOperation::Disabled) {
        _start("Timer", "dumpLogs Function", {});

        if (!outputFile_.is_open()) {
            std::cerr << "Output file is not open. Cannot dump logs." << std::endl;
            return;
        }

        std::lock_guard<std::mutex> lock(memoryMutex_);

        nlohmann::json jsonLogs = nlohmann::json::array();
        for (const raw_event_t& event : inMemoryLogs_) {
            nlohmann::json jEvent;
            jEvent["pid"] = event.pid;
            jEvent["tid"] = event.tid;
            jEvent["ts"] = event.ts;
            jEvent["ph"] = std::string(1,event.ph);
            jEvent["name"] = event.name;
            jEvent["cat"] = event.cat;

            // Given that args is now an unordered_map, this becomes simpler
            jEvent["args"] = event.args;

            jsonLogs.push_back(jEvent);
        }

        outputFile_ << jsonLogs.dump(4);
        inMemoryLogs_.clear();

        _stop("Timer", "dumpLogs Function", {});
    }
}