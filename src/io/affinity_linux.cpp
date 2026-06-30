#include "CpuAffinity.hpp"
#include <pthread.h>
#include <sched.h>

namespace Affinity
{
    bool PinThreadToCore(int core_id)
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU(&cpuset);

        CPU_SET(core_id, &cpuset);

        pthread_t thread_handle = pthread_self();

        if (pthread_setaffinity_np(thread_handle, sizeof(cpu_set_t), &cpuset) == 0)
        {
            return true;
        }
        return false;
    }
}