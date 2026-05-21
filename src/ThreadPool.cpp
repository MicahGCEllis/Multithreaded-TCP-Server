/*
    Logic for "Worker Loop"
    An infinite loop where threads sleep until condition variables wakes them
    Once awake the thread locks the mutex and puls client from queue and unlocks mutex
    passses client to Connection Handler
*/

#include "../include/ThreadPool.hpp"

ThreadPool::ThreadPool(size_t ThreadCount, Logger& logger)
: logger(logger)
{
    for (size_t i = 0; i < ThreadCount; ++i)
    {
        workers.emplace_back(&ThreadPool::WorkerLoop, this);
    }
}

void ThreadPool::WorkerLoop()
{
    while(true)
    {
        std::unique_lock<std::mutex> lock(Lock);
        Condition.wait(lock, [this] 
        {
            return !clientQueue.empty() || this->stop;
        });

        if (this->stop && clientQueue.empty())
        {
            return;
        }

        SOCKET client_socket = clientQueue.front();
        clientQueue.pop();
        lock.unlock();

        ConnectionHandler handler(client_socket, this->logger);
        handler.HandleConnection();
    }
}

void ThreadPool::EnqueueClient(SOCKET client_socket)
{
    std::lock_guard<std::mutex> lock(Lock);

    clientQueue.push(client_socket);
    Condition.notify_one();
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> LockGuard(Lock);
        stop = true;
    }
    Condition.notify_all();
    for (std::thread &worker : workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}