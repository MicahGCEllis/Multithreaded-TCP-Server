#include "TcpServer.hpp"
#include "Config.hpp"
#include "ConnectionHandler.hpp"
#include "ThreadPool.hpp"
#include "Telemetry.hpp"
#include "Fiber.hpp"
#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>
#include <exception>

// Global atomic flag to control the main execution loop safely across threads
std::atomic<bool> running(true);

// Forward declaration for OS signal interception
void SignalHandler(int signal);

int main() {

    INIT_WINSOCK(); // Initialize WinSock on Windows, no-op on Unix-like systems
    TelemetrySystem::InitTelemetryBridge();

    std::ofstream pid_file(Config::PID_FILE_PATH);
        if (pid_file.is_open())
        {
            pid_file << GET_PID();
            pid_file.close();
        }

    // Bind OS termination signals (Ctrl + C) to custom handler
    // Allows a graceful shutdown, ensuring threads and files close properly
    std::signal(SIGINT, SignalHandler);

    try
    {
        TcpServer server(Config::DEFAULT_PORT);
        server.Start(running);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}

// Trigger when user attemps to terminate program/server
void SignalHandler(int signal)
{
    std::cout << "Shutdown signal received: " << signal << ". Closing engine" << std::endl;
    running = false; // Breakes TcpServer loop
}