#include "Fiber.hpp"
#include <iostream>
#include <cassert>

int main ()
{
    Fiber main_fiber([](){});
    Fiber::current_fiber = &main_fiber;
    Fiber::scheduler_fiber = &main_fiber;

    int execution_step = 0;

    Fiber* fiber_a_ptr = nullptr;
    Fiber* fiber_b_ptr = nullptr;

    Fiber fiber_b([&](){
        assert(execution_step == 1);
        std::cout << "Fiber A ran first \n";
        execution_step = 2;
        Fiber::YieldTo(fiber_a_ptr);
    });

    Fiber fiber_a([&](){
        assert(execution_step == 0);
        execution_step = 1;
        Fiber::YieldTo(fiber_b_ptr);

        assert(execution_step == 2);
        execution_step = 3;
        Fiber::current_fiber->is_complete = true;
        Fiber::YieldTo(Fiber::scheduler_fiber);
    });

    fiber_a_ptr = &fiber_a;
    fiber_b_ptr = &fiber_b;

    Fiber::YieldTo(&fiber_a);
    assert(execution_step == 3);
    std::cout << "Scheduler Success";

    return 0;
}