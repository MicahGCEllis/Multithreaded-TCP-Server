#pragma once

#include <string>
#include <fstream>
#include <mutex>

class Logger {

    private:
        std::ofstream log_file;
        std::mutex log_mutex;

    public:
        Logger(const std::string& filename);
        ~Logger();
        void Log(const std::string& message);
};