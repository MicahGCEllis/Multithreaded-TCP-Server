#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <winsock2.h>
#include "ConnectionHandler.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "EventLoop.hpp"
#include "Fiber.hpp"
#include "CpuAffinity.hpp"

// Manages a pre-allocated pool of workr threads to handle concurrent connections
// Prevents the overhead of creating/destroying threads for every single HTTP request
class ThreadPool
{
    private:
        std::vector<std::thread> workers;
        std::queue<SOCKET> clientQueue;

        // Synchronization primitives to manage thread access to shared queue
        std::mutex Lock;
        std::condition_variable Condition;

        bool stop = false;
        Logger& logger;

        // The infinite execution loop assigned to every worker thread
        void WorkerLoop(int thread_id);

    public: 
        ThreadPool(size_t ThreadCount, Logger& logger);
        ~ThreadPool();

        // Pushes a new client into queue and wakes up a sleeping worker
        void EnqueueClient(SOCKET client_socket);
};