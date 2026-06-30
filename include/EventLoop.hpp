#pragma once

#include "Config.hpp"
#include <vector>

class EventLoop
{
    private:

    public:
        virtual void RegisterSocket(SOCKET socket) = 0;
        virtual void WaitForEvents(std::vector<SOCKET>&active_events, int timeout_ms) = 0;
        virtual bool AsyncAccept(SOCKET listener) = 0;
        static EventLoop* Create();
        virtual ~EventLoop() noexcept = default;
};