// Ensure that this header file is only included once duirng compilation0
#pragma once
// include cstdint
// cstdint provides fixed-width integer types which is useful for network packets
#include "ThreadPool.hpp"
#include "Logger.hpp"
#include <cstdint>
#include <atomic>
#include <winsock2.h>

// Define the TcpServer class
class TcpServer {
    private:
        uint16_t port; // Port number the server will listen on
        SOCKET socket_file_descriptor; // Socket file descriptor for the server
        bool is_running; // Flag to tell whether or not the server is running or not
        void SetupSocket(); // Private helper that start() calls to ask OS for socket and binding it to port
        ThreadPool pool;
        Logger logger;
    public:
        TcpServer(uint16_t port); // Constructor to initalize server
        ~TcpServer(); // Destructor to clean up resources like closing the socket
        void Start(std::atomic<bool>& isRunning); // Start server and begin awaiting client connections
        void Stop(); // Stop server and clean up resources
};
