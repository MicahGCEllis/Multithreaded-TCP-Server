#pragma once

#include <string>
#include <fstream>
#include <thread>
#include <mutex>

class Logger {

    private:
        std::mutex log_mutex;
        std::ofstream traffic;
        std::ofstream systems;
    public:
        Logger();
        ~Logger();
        void LogTraffic(const std::string& method, const std::string& path, int status_code);
        void LogSystem(std::thread::id thread_id, const std::string& event_description);
};