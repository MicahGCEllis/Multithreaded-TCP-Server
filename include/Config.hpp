// Config file for the server
#pragma once
#include <cstdint>
#include <iostream>
#include <fstream>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define NET_ERR_ADDRINUSE WSAEADDRINUSE
    #define NET_ERR_WOULDBLOCK WSAEWOULDBLOCK
    #define CAST_SOCKOPT(x) (const char*)(x)
    #define GET_PID() GetCurrentProcessId()
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <fcntl.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <cerrono>
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <uinistd.h>
    #define NET_ERR_ADDRINUSE EADDRINUSE
    #define NET_ERR_WOULDBLOCK EWOULDBLOCK
    #define CAST_SOCKOPT(x) (const void*)(x)
    #define GET_PID() getpid()
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
    inline int NET_ERROR() 
    {
        return errno;
    }
#endif
#include <thread>

namespace Config 
{

    // API Versioning: WinSock 2.2
    const uint8_t WINSOCK_MAJOR_VERISON = 2;
    const uint8_t WINSOCK_MINOR_VERSION = 2;

    // Networking Constant
    const uint16_t DEFAULT_PORT = 8080;

    // SOMAXCONN provides the maximum queue length allowed by OS
    const int MAX_CONNECTIONS = SOMAXCONN;

    // Data handling
    const size_t BUFFER_SIZE = 4096;
    const size_t QUEUE_SIZE = 5000;

    // Concurrency: Scales dynamically to the host's CPU core count
    const size_t THREAD_POOL_SIZE = std::thread::hardware_concurrency();

    // Path to file where process ID to be stored
    constexpr const char* PID_FILE_PATH = "../planet.pid";
} 

#ifdef _WIN32
    inline void INIT_WINSOCK() 
    {
        WSADATA wsaData;
        int startup_result = WSAStartup(MAKEWORD(Config::WINSOCK_MAJOR_VERISON, Config::WINSOCK_MINOR_VERSION), &wsaData);
        if (startup_result != 0) 
        {
            std::cerr << "WSAStartup failed with error: " 
                      << startup_result << std::endl;
            throw std::runtime_error("Failed to initialize Winsock");
        }
    }
    inline void CLEANUP_WINSOCK() 
    {
        WSACleanup();
    }
    inline int NET_ERROR() 
    {
        return WSAGetLastError();
    }
    inline void MAKE_SOCKET_NON_BLOCKING(SOCKET s)
    {
        u_long mode = 1;
        ioctlsocket(s, FIONBIO, &mode);
    }
    
#else
    inline void INIT_WINSOCK() 
    {
        // No initialization needed on Unix-like systems
    }
    inline void CLEANUP_WINSOCK() 
    {
        // No cleanup needed on Unix-like systems
    }
    inline void MAKE_SOCKET_NON_BLOCKING(SOCKET s)
    {
        int flags = fcntl(s, F_GETFL, 0);
        fcntl(s, F_SETFL, flags | O_NONBLOCK);
    }
#endif