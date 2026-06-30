#include "TcpServer.hpp"
#include "ConnectionHandler.hpp"
#include "Telemetry.hpp"
#include "EventLoop.hpp"
#include <filesystem>
#include <iostream> 
#include <stdexcept> 

TcpServer::TcpServer(uint16_t port) 
    : port(port), socket_file_descriptor(INVALID_SOCKET), 
    is_running(false), logger(), pool(Config::THREAD_POOL_SIZE, this->logger)
    { 
    }

void TcpServer::SetupSocket()
{
    // Request an IPv4, TCP streaming socket from the operating system
    this->socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (this->socket_file_descriptor == INVALID_SOCKET)
    {
        std::cerr << "socket() failed with error: "
                  << NET_ERROR() << std::endl;
        throw std::runtime_error("Failed to create socket");
    }

    // Configure network address architurture 
    sockaddr_in server_address = {};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(this->port);  // Convert prot to network byte order
    server_address.sin_addr.s_addr = INADDR_ANY; // Listen onall availiable netowrk interfaces

    if (bind(this->socket_file_descriptor, 
        (sockaddr*)&server_address, 
        sizeof(server_address)) == SOCKET_ERROR)
    {
        std::cerr << "bind() failed with error: "
                  << NET_ERROR() << std::endl;
        if (NET_ERROR() == NET_ERR_ADDRINUSE)
        {
            throw std::runtime_error("CRITICAL: Port 8080 is already in use by another program! Did you leave another instance of this server running?");
        }
    }

    // Begin queuing incoming connection requests
    if (listen(this->socket_file_descriptor, 
        Config::MAX_CONNECTIONS) == SOCKET_ERROR)
    {
        std::cerr << "listen() failed with error: "
                  << NET_ERROR() << std::endl;
        throw std::runtime_error("Failed to listen on socket");
    }
}

void TcpServer::Start(std::atomic<bool>& isRunning) 
{
    SetupSocket();
    std::cout << "Server running and listening on port " << this->port << std::endl;
    EventLoop* loop = EventLoop::Create();
    loop->RegisterSocket(this->socket_file_descriptor);

    for (int i = 0; i <= 99; ++i)
    {
        loop->AsyncAccept(this->socket_file_descriptor);
    }

    while (isRunning.load())
    {
        std::vector<SOCKET> active_sockets;
        loop->WaitForEvents(active_sockets, 100);

        for (int i = 0; i < active_sockets.size(); ++i)
        {
            if (active_sockets.at(i) != INVALID_SOCKET)
            {
                pool.EnqueueClient(active_sockets.at(i));
            }
            loop->AsyncAccept(this->socket_file_descriptor);
        }
    }
    delete loop;
}

void TcpServer::Stop()
{
    is_running = false;
}

TcpServer::~TcpServer()
{
    Stop();

    if (this->socket_file_descriptor != INVALID_SOCKET)
    {
        closesocket(this->socket_file_descriptor);
    }

    // Free OS network resources
    CLEANUP_WINSOCK();
    std::remove(Config::PID_FILE_PATH); // Remove PID file on shutdown
}