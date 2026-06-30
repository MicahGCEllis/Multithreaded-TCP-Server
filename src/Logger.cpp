#include "../include/Logger.hpp"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>

Logger::Logger()
{
    kill = true; // Thread should stay alive
    // Ensure the logs directory exists on startup
    if (!std::filesystem::exists("../logs/"))
    {
        std::filesystem::create_directories("logs/");
    }

    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream time_stream;
    
    // Format timestamp for unique log file naming
    time_stream << std::put_time(std::localtime(&current_time), "%m_%d_%Y_%H_%M_%S");
    std::string traffic_directory = "../logs/traffic_" + time_stream.str() + ".csv";
    std::string system_directory = "../logs/system_" + time_stream.str() + ".csv";

    // Open streams in append mode
    traffic.open(traffic_directory, std::ios::app);
    system.open(system_directory, std::ios::app);

    log_thread = std::thread(&Logger::WriteLoop, this);

    if (!traffic.is_open() || !system.is_open())
    {
       throw std::runtime_error("Failed to open log files!");
    }
}

Logger::~Logger()
{
    kill = false;
    bell.notify_all();

    if (log_thread.joinable())
    {
        log_thread.join();
    }

    // Safely close streams on shutdown
    if (traffic.is_open() && system.is_open())
    {
        traffic.close();
        system.close();
    }
}

void Logger::LogTraffic(const std::string& method, const std::string& path, status status_code)
{
    uint16_t code = static_cast<uint16_t>(status_code);
    std::stringstream traffic_stream;
    LogEvent traffic_event;
    
    traffic_stream << method << ", " 
                   << path << ", " 
                   << code << std::endl;

    traffic_event.state = Logtype::traffic;
    traffic_event.Logmessage = traffic_stream.str();
    // Lock the mutex to ensure thread-safe console and file output
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        log_queue.push(traffic_event);
    }
    bell.notify_one();
    std::cout << traffic_stream.str();
}

void Logger::LogSystem(std::thread::id thread_id, const std::string& event_description)
{
    std::stringstream system_stream;
    LogEvent system_event;
    
    system_stream << thread_id << ", " << event_description << std::endl;

    system_event.state = Logtype::system;
    system_event.Logmessage = system_stream.str();
    // Lock the mutex to ensure thread-safe console and file output
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        log_queue.push(system_event);
    }
    bell.notify_one();
    std::cout << system_stream.str();
}

void Logger::WriteLoop()
{
    while(true)
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        bell.wait(lock, [&] {
            return !log_queue.empty() || kill == false;
        });

        if (kill == false && log_queue.empty())
        {
            break;
        }

        LogEvent write;
        write = log_queue.front();
        log_queue.pop();

        lock.unlock();

        if (write.state == Logtype::traffic)
        {
            traffic << write.Logmessage;
            traffic.flush();
        }
        else if (write.state == Logtype::system)
        {
            system << write.Logmessage;
            system.flush();
        }
    }
    
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

