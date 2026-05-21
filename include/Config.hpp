// Config file for the server
#pragma once
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
#endif
#include <cstdint>
#include <thread>

// Using inline constexpr instead of const to not confuse the compiler and accidentally create duplicate variables in memeory
// Creates a single instance of each variable that is used everywhere (constant could be used if this was all in main)
namespace Config 
{
    // API Contract: Define Major and Minor version of winsock
    // Using unsigned int as we only very little space to hold these numbers
    const uint8_t WINSOCK_MAJOR_VERISON = 2;
    const uint8_t WINSOCK_MINOR_VERSION = 2;

    // Networking constants
    const uint16_t DEFAULT_PORT = 8080;
    // Maximum number of pending connections that will be passed to listen()
    // Using SOMAXCONN which asks the OS for the maximum number of pending connections it can handle
    const int MAX_CONNECTIONS = SOMAXCONN;

    // Data Limitations
    // Buffer size of 4KB
    const size_t BUFFER_SIZE = 4096;

    // Hardware Scaling (Thread Pooling)
    // Number of working there will be made and will be the size of any system it is run on
    // Asks the number of cores the physical CPU has
    const size_t THREAD_POOL_SIZE = std::thread::hardware_concurrency();
} 