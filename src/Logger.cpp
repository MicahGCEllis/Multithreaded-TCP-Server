#include "../include/Logger.hpp"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>

Logger::Logger()
{
    if (!std::filesystem::exists("logs/"))
    {
        std::filesystem::create_directories("logs/");
    }

    auto now = std::chrono::system_clock::now();

    std::time_t current_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream time_stream;
    time_stream << std::put_time(std::localtime(&current_time), "%m_%d_%Y_%H_%M_%S");
    std::string traffic_directory = "logs/traffic_" + time_stream.str() + ".csv";
    std::string system_directory = "logs/system_" + time_stream.str() + ".csv";

    traffic.open(traffic_directory, std::ios::app);
    systems.open(system_directory, std::ios::app);

    if (!traffic.is_open() || !systems.is_open())
    {
       throw std::runtime_error("Failed to open log files!");
    }
}

Logger::~Logger()
{
    if (traffic.is_open() && systems.is_open())
    {
        traffic.close();
        systems.close();
    }
}

void Logger::LogTraffic(const std::string& method, const std::string& path, int status_code)
{
    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    std::lock_guard<std::mutex> lock(log_mutex);
    std::cout << method << ", " << path << ", " << status_code << std::endl;
    traffic << method << ", " 
            << path << ", " 
            << status_code << std::endl;
    traffic.flush();

}

void Logger::LogSystem(std::thread::id thread_id, const std::string& event_description)
{
    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    std::cout << "Thread [" << thread_id << "] " << event_description << std::endl;

    std::lock_guard<std::mutex> lock(log_mutex);
    systems << thread_id << ", " 
            << event_description << std::endl;
    systems.flush();
}

// void Logger::Log(const std::string& message)
// {
//     auto now = std::chrono::system_clock::now();
//     std::time_t current_time = std::chrono::system_clock::to_time_t(now);

//     std::lock_guard<std::mutex> lock(log_mutex);
//     std::cout << "["
//               << std::put_time(std::localtime(&current_time), "%Y-%m-%d %H:%M:%S")
//               << "] " << message << std::endl;
//     log_file << "["
//              << std::put_time(std::localtime(&current_time), "%Y-%m-%d %H:%M:%S")
//              << "] " << message << std::endl;
//     log_file.flush();
// }

