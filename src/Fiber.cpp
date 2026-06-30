#include "Fiber.hpp"
#include <stdexcept>

thread_local Fiber* Fiber::current_fiber = nullptr;
thread_local Fiber* Fiber::scheduler_fiber = nullptr;

Fiber::Fiber(std::function<void()> work)
{
    this->fiber_work = work;
    stack_memory = new uint8_t[stack_size];

    uint8_t* top_of_stack = stack_memory + stack_size;
    uintptr_t stack_int = reinterpret_cast<uintptr_t>(top_of_stack);
    stack_int &= ~15; // align down to 16-byte boundary
    stack_int -= 8;   // x64 ABI: RSP must be 8-byte aligned (not 16) at function entry
    top_of_stack = reinterpret_cast<uint8_t*>(stack_int);

    top_of_stack -= sizeof(void*);
    *reinterpret_cast<void**>(top_of_stack) = reinterpret_cast<void*>(&Fiber::FiberEntryPoint);
    top_of_stack -= REGISTER_PADDING;
    context.rsp = top_of_stack;
}

Fiber::~Fiber()
{
    delete[] stack_memory;
}

void Fiber::YieldTo(Fiber* next)
{
    if (current_fiber == nullptr)
    {
        throw std::runtime_error("Cannot yield from main OS thread without a bootstrap fiber.");
    }
    Fiber* current = current_fiber;
    current_fiber = next;
    switch_context(&current->context, &next->context);
}

void Fiber::FiberEntryPoint()
{
    current_fiber->fiber_work();
    current_fiber->is_complete = true;
    Fiber::YieldTo(Fiber::scheduler_fiber);
}

FiberContext* Fiber::GetContext()
{
    return &context;
}