#include "EventLoop.hpp"
#include "Config.hpp"
#include <cassert>
#include <vector>
#include <thread>
#include <iostream>

int main ()
{
    INIT_WINSOCK();
    EventLoop* loop = EventLoop::Create();

    SOCKET listener_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    bind(listener_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    listen(listener_socket, Config::MAX_CONNECTIONS);
    loop->RegisterSocket(listener_socket);

    for (int i = 0; i < 10; ++i)
    {
        loop->AsyncAccept(listener_socket);
    }

    std::thread client_swarm([&](){
        for (int i = 0; i < 10; ++i)
        {
            SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
            connect(client_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
            closesocket(client_socket);
        }
    });

    client_swarm.join();

    std::vector<SOCKET> active_events;
    while (active_events.size() < 10)
    {
        loop->WaitForEvents(active_events, 1000);
    }
    assert(active_events.size() == 10);
    for (int i = 0; i < active_events.size(); ++i)
    {
        assert(active_events[i] != INVALID_SOCKET);
        closesocket(active_events[i]);
    }

    closesocket(listener_socket);
    delete loop;
    CLEANUP_WINSOCK();

    std::cout << "IOCP Accept Success\n";
    return 0;
}