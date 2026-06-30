#pragma once

#include "FiberContext.hpp"
#include <cstdint>
#include <cstddef>
#include <functional>

extern "C" const uint64_t REGISTER_PADDING;

class Fiber
{
    private:
        struct FiberContext context;
        uint8_t* stack_memory;
        size_t stack_size = 65536; // 64KB
        std::function<void()> fiber_work;
        static void FiberEntryPoint();

    public:
        bool is_complete = false;
        static thread_local Fiber* current_fiber;
        static thread_local Fiber* scheduler_fiber;
        Fiber(std::function<void()> work);
        ~Fiber();
        FiberContext* GetContext();
        static void YieldTo(Fiber* next);
};