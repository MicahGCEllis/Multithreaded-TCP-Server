#include "Fiber.hpp"
#include <iostream>
#include <cassert>

extern "C" uintptr_t get_rsp();

int main ()
{
    Fiber main_fiber([](){});
    Fiber::current_fiber = &main_fiber;
    Fiber::scheduler_fiber = &main_fiber;

    uintptr_t observed_rsp = 0;

    Fiber test_fiber([&](){
        observed_rsp = get_rsp();
        Fiber::current_fiber->is_complete = true;
        Fiber::YieldTo(Fiber::scheduler_fiber);
    });

    Fiber::YieldTo(&test_fiber);

    assert(observed_rsp % 16 == 8);

    std::cout << "Success" << std::endl;

    return 0;
}