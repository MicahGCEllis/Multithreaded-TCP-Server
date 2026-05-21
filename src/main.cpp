#include "../include/TcpServer.hpp"
#include "../include/Config.hpp"
#include "../include/ConnectionHandler.hpp"
#include "../include/ThreadPool.hpp"
#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>
#include <exception>

std::atomic<bool> running(true);

void SignalHandler(int signal);

// ThreadPooling
// Initiate TcpServer and give port number 8080
// Finally call server.start() to start signal handling and passing ctrl+C shuts the server down instead of a crash

int main() {

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
    

}

void SignalHandler(int signal)
{
    std::cout << "Shutdown signal received: " << signal << ". Closing engine" << std::endl;
    running = false;
}