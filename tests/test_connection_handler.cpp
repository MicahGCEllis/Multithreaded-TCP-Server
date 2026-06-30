#include "ConnectionHandler.hpp"
#include "Config.hpp"
#include "Telemetry.hpp"
#include "Fiber.hpp"
#include <cassert>
#include <string>
#include <cstring>

int main ()
{
    INIT_WINSOCK();
    TelemetrySystem::InitTelemetryBridge();

    Fiber main_fiber([](){});
    Fiber::current_fiber = &main_fiber;
    Fiber::scheduler_fiber = &main_fiber;


    Logger logger;
    SOCKET listener_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(9090);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    bind(listener_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    listen(listener_socket, Config::MAX_CONNECTIONS);
    {
        SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

        connect(client_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

        SOCKET server_socket = accept(listener_socket, nullptr, nullptr);
        
        std::string get = "GET /index.html HTTP/1.1\r\n\r\n";
        send(client_socket, get.c_str(), get.size(), 0);

        ConnectionHandler handle = ConnectionHandler(server_socket, logger);
        handle.HandleConnection();

        char buffer[4096]= {0};
        recv(client_socket, buffer, sizeof(buffer), 0);
        std::string search(buffer);

        assert(search.find("200 OK") != std::string::npos);
        closesocket(client_socket);
        closesocket(server_socket);
    }

    {
        SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

        connect(client_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

        SOCKET server_socket = accept(listener_socket, nullptr, nullptr);
        
        std::string get = "GET /does_not_exist.txt HTTP/1.1\r\n\r\n";
        send(client_socket, get.c_str(), get.size(), 0);

        ConnectionHandler handle = ConnectionHandler(server_socket, logger);
        handle.HandleConnection();

        char buffer[4096]= {0};
        recv(client_socket, buffer, sizeof(buffer), 0);
        std::string search(buffer);

        assert(search.find("404 Not Found") != std::string::npos);
        closesocket(client_socket);
        closesocket(server_socket);
    }

    {
        SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

        connect(client_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

        SOCKET server_socket = accept(listener_socket, nullptr, nullptr);
        
        std::string get = "GET /../Config.hpp HTTP/1.1\r\n\r\n";
        send(client_socket, get.c_str(), get.size(), 0);

        ConnectionHandler handle = ConnectionHandler(server_socket, logger);
        handle.HandleConnection();

        char buffer[4096]= {0};
        recv(client_socket, buffer, sizeof(buffer), 0);
        std::string search(buffer);

        assert(search.find("403 Forbidden") != std::string::npos);
        closesocket(client_socket);
        closesocket(server_socket);
    }
    closesocket(listener_socket);
    CLEANUP_WINSOCK();

    std::cout << "Connection Handler Success\n";
    return 0;
}