/*
    Define a std::vector and std::thread objects
    define std::queue to hold incoming client connections
    define std::mutex and std::condition_variable to prevent two threads from trying to grab the same client at the same time
*/

#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <winsock2.h>
#include "ConnectionHandler.hpp"
#include "Logger.hpp"

class ThreadPool
{
    private:
        std::vector<std::thread> workers;
        std::queue<SOCKET> clientQueue;
        std::mutex Lock;
        std::condition_variable Condition;
        void WorkerLoop();
        bool stop = false;
        Logger& logger;

    public: 
        ThreadPool(size_t ThreadCount, Logger& logger);
        ~ThreadPool();
        void EnqueueClient(SOCKET client_socket);
};