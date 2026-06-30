#pragma once

#include "ThreadPool.hpp"
#include "Config.hpp"
#include "Telemetry.hpp"
#include "Logger.hpp"
#include <cstdint>
#include <atomic>

// Represents the core network engine
// Handles OS socket binding, port listening, and passing new connections to the Thread Pool
class TcpServer {
    private:
        uint16_t port;
        SOCKET socket_file_descriptor; 
        bool is_running; 
        ThreadPool pool;
        Logger logger;
        Telemetry* Global_Telemetry_Ptr = nullptr;

        // Interanl helper to ask OS for a socket and bind it to the port
        void SetupSocket(); 
        
    public:
        TcpServer(uint16_t port); 
        ~TcpServer(); 

        // Initiates the non-blocking infinite listen loop
        void Start(std::atomic<bool>& isRunning); 

        // Signals the server to stop accepting connections
        void Stop(); 
};
