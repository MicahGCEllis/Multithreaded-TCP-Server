#pragma once

#include "HttpStatus.hpp"
#include <string>
#include <atomic>
#include <cstdint>
#include <fstream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

// Thread safe logging utility that writes server events to local CSV files

enum class Logtype : uint8_t {
    traffic = 1,
    system = 0
};
struct LogEvent {
    Logtype state;
    std::string Logmessage;
};
class Logger {

    private:
        std::ofstream traffic;
        std::ofstream system;
        std::queue<LogEvent> log_queue;
        std::mutex queue_mutex;
        std::atomic<bool> kill;
        std::thread log_thread;
        std::condition_variable bell;
        void WriteLoop();
    public:
        Logger();
        ~Logger();
        void LogTraffic(const std::string& method, const std::string& path, status status_code);
        void LogSystem(std::thread::id thread_id, const std::string& event_description);
};