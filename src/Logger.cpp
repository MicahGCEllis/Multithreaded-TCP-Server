#include "../include/Logger.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

Logger::Logger(const std::string& filename) : log_file(filename, std::ios::app)
{
    if (!log_file.is_open())
    {
        throw std::runtime_error("Unable to open log file: " + filename);
    }
}

Logger::~Logger()
{
    if (log_file.is_open())
    {
        log_file.close();
    }
}

void Logger::Log(const std::string& message)
{
    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    std::lock_guard<std::mutex> lock(log_mutex);
    std::cout << "["
              << std::put_time(std::localtime(&current_time), "%Y-%m-%d %H:%M:%S")
              << "] " << message << std::endl;
    log_file << "["
             << std::put_time(std::localtime(&current_time), "%Y-%m-%d %H:%M:%S")
             << "] " << message << std::endl;
    log_file.flush();
}

