#include "../include/TcpServer.hpp"
#include "../include/ConnectionHandler.hpp"
#include "../include/Config.hpp"
#include <iostream> // For std::cerr
#include <stdexcept> // For std::runtime_error
#include <winsock2.h>
#include <ws2tcpip.h>
// Using initialization list to initalize private memebers
// Additionally, avoding the chance of duplicates of memebers being initalized in the body of the constructor and class call
TcpServer::TcpServer(uint16_t port) 
    : port(port), socket_file_descriptor(INVALID_SOCKET), 
    is_running(false), logger("server.log"), pool(Config::THREAD_POOL_SIZE, this->logger)
    {
        // Initalize Winsock (Microsft local variable) 
        // Nothing is assigned as this acts like a notepad for the network
        WSADATA wsaData;
        // Initalizing int called startup_result to handle the resutl of WSAStartup()
        // WSAStartup() is a function that initializes the Winsock library and must be called before any other Winsock functions can be used
        // It take two arguments, one is version numbe of Winsock Windows accepts while the other is the address of wsaData variable we initalized
        // Hard coded 2.2 version o winsock as 2.2 does everything a modern OS needs and to dynamically ask would put server into a new enviorment. This is a API Contract
        int startup_result = WSAStartup(MAKEWORD(Config::WINSOCK_MAJOR_VERISON, Config::WINSOCK_MINOR_VERSION), &wsaData);
        // Using cerr instead of cout to stop the complier that moment when encountering error
        if (startup_result != 0) 
        {
            std::cerr << "WSAStartup failed with error: " 
                      << startup_result << std::endl;
            throw std::runtime_error("Failed to initialize Winsock");
        }
    }

void TcpServer::SetupSocket()
{
    // TODO: ask the OS (Windows) to create the netowrk endpoint, we have to assign this to the socket variable earlier
    this->socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (this->socket_file_descriptor == INVALID_SOCKET)
    {
        std::cerr << "socket() failed with error: "
                  << WSAGetLastError() << std::endl;
        throw std::runtime_error("Failed to create socket");
    }
    // Calling variable called server_address of type sockaddr_in (socket address Internet)
    // Calling it blank and then filling in those blanks as the RAM may be filled with random data
    sockaddr_in server_address = {};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(this->port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(this->socket_file_descriptor, 
        (sockaddr*)&server_address, 
        sizeof(server_address)) == SOCKET_ERROR)
    {
        std::cerr << "bind() failed with error: "
                  << WSAGetLastError() << std::endl;
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(this->socket_file_descriptor, 
        Config::MAX_CONNECTIONS) == SOCKET_ERROR)
    {
        std::cerr << "listen() failed with error: "
                  << WSAGetLastError() << std::endl;
        throw std::runtime_error("Failed to listen on socket");
    }
}

void TcpServer::Start(std::atomic<bool>& isRunning) 
{
    SetupSocket();
    logger.Log("Server running and listening on port " + std::to_string(this->port));
    

    while (isRunning.load())
    {
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(this->socket_file_descriptor, &read_fds);
        sockaddr_in client_address = {};
        int client_address_size = sizeof(client_address);
        int activity = select(0, &read_fds, nullptr, nullptr, &timeout);

        if (activity == 0)
        {
            continue;
        }
        else if (activity < 0)
        {
            std::cerr << "select() failed with error: "
                      << WSAGetLastError() << std::endl;
            break;
        }
        else if (activity > 0)
        {
            SOCKET client_socket = accept(this->socket_file_descriptor, 
                                         (sockaddr*)&client_address, 
                                          &client_address_size);

            if (client_socket == INVALID_SOCKET)
            {
            std::cerr << "accept() failed with error: "
                      << WSAGetLastError() << std::endl;
            continue; // Continue to accept the next connection
            }
            else
            {
            this->logger.Log("Accepted new client...connected");
            pool.EnqueueClient(client_socket);
            }
        }
    }
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
    WSACleanup();
}
/*
    implement system calls: socket(), bind(), listen()
    implement a infinite while look that will call accept() and when 

    When accept() catches a new client write the code that packages that client and toss it into threadpool's queue
*/