#include "ThreadPool.hpp"
#include "Telemetry.hpp"
#include "Logger.hpp"
#include <unordered_map>

ThreadPool::ThreadPool(size_t ThreadCount, Logger& logger)
: logger(logger)
{
    // Pre-allocate and start all worker threads upon instatiation
    for (size_t i = 0; i < ThreadCount; ++i)
    {
        workers.emplace_back(&ThreadPool::WorkerLoop, this, i);
    }
}

void ThreadPool::WorkerLoop(int thread_id)
{
    Affinity::PinThreadToCore(thread_id);
    EventLoop* local_loop = EventLoop::Create();
    Fiber master_fiber([](){});

    Fiber::scheduler_fiber = &master_fiber;

    Fiber::current_fiber = &master_fiber;
    std::unordered_map<SOCKET, Fiber*> active_fibers;
    while(true)
    {
        // Lock the mutex before checking the queue 
        std::unique_lock<std::mutex> lock(Lock);

        // Put thread to sleep to save CPU cycels until a client is available or shutdown is tirggered
        std::vector<SOCKET> new_clients;

        while (!clientQueue.empty())
        {
            new_clients.push_back(clientQueue.front());
            clientQueue.pop();
        }

        lock.unlock();

        for (int client_socket : new_clients)
        {
            this->logger.LogSystem(std::this_thread::get_id(), "Processing Connection");

            Fiber* fiber = new Fiber(
                [this, client_socket]()
                {
                    ConnectionHandler handler(client_socket, this->logger);
                    handler.HandleConnection();
                }
            );
            local_loop->RegisterSocket(client_socket);
            active_fibers.emplace(client_socket, fiber);

            Fiber::YieldTo(fiber);

            if (fiber->is_complete)
            {
                delete fiber;
                active_fibers.erase(client_socket);
            }
        }

        std::vector<SOCKET> active_events;
        local_loop->WaitForEvents(active_events, 100);

        for (SOCKET active_socket : active_events)
        {
            if (active_socket == INVALID_SOCKET) { continue; }

            if (active_fibers.find(active_socket) != active_fibers.end())
            {
                Fiber* target_fiber = active_fibers[active_socket];
                Fiber::YieldTo(target_fiber);

                if (target_fiber->is_complete)
                {
                    delete target_fiber;
                    active_fibers.erase(active_socket);
                }
                else
                {
                    // Re-arm the zero-byte read so the fiber wakes up for the next data chunk
                    local_loop->RegisterSocket(active_socket);
                }
            }
        }

        // If shutdown flag is active and queue is empty, terminate the thread
        if (this->stop && clientQueue.empty())
        {
            return;
        }

        // Safely extract client and release the lock immediately so toher threads can access the queue
        // SOCKET client_socket = clientQueue.front();
        // clientQueue.pop();
        // TelemetrySystem::Global_Telemetry_Ptr->fiber_saturation--;
        // lock.unlock();
    }
}

void ThreadPool::EnqueueClient(SOCKET client_socket)
{
    //Secure exclusive access to the queue before pushing new data
    std::lock_guard<std::mutex> lock(Lock);
    if (clientQueue.size() >= Config::MAX_CONNECTIONS)
    {
        linger axe;
        axe.l_onoff = 1;
        axe.l_linger = 0;
        setsockopt(client_socket, SOL_SOCKET, SO_LINGER, CAST_SOCKOPT(&axe), sizeof(axe));
        closesocket(client_socket);
        TelemetrySystem::Global_Telemetry_Ptr->dropped_connections++;
        return;
    }
    clientQueue.push(client_socket);
    TelemetrySystem::Global_Telemetry_Ptr->fiber_saturation++;

    // Ping the condition variable to wake up exactly one sleeping worker
    Condition.notify_one();
}

ThreadPool::~ThreadPool()
{
    {
        // Lock queue and trigger the global kill switch
        std::lock_guard<std::mutex> LockGuard(Lock);
        stop = true;
    }
    // Wake up all sleeping threads so they can evaluate the stop flag and exit naturally
    Condition.notify_all();

    for (std::thread &worker : workers)
    {
        // Re-sync worker threads with the main program execution before destructing
        if (worker.joinable())
        {
            worker.join();
        }
    }
}