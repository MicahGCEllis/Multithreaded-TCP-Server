#include "Logger.hpp"
#include <thread>
#include <vector>
#include <iostream>

int main ()
{
    {
        Logger logger;

        std::vector<std::thread> producers;

        for (int i = 0; i < 4; ++i)
        {
            producers.emplace_back([&logger]() {
            for (int i = 0; i < 25; ++i)
            {
                logger.LogSystem(std::this_thread::get_id(), "Stress Test Message");
            }
            });
        }
        for (int i = 0; i < producers.size(); ++i)
        {
            producers[i].join();
        }
    }
    std::cout << "Telemetry Checkpoint Success\n";

    return 0;
}