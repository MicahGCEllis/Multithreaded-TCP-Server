#include "CpuAffinity.hpp"
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>

namespace Affinity
{
    bool PinThreadToCore(int core_id)
    {
        thread_port_t thread_port = mach_thread_self();
        thread_affinity_policy_data_t policy;
        
        policy.affinity_tag = core_id;

        if (thread_policy_set(thread_port, THREAD_AFFINITY_POLICY, reinterpret_cast<thread_policy_t>(&policy), THREAD_AFFINITY_POLICY_COUNT) == KERN_SUCCESS)
        {
            mach_port_deallocate(mach_task_self(), thread_port);
            return true;
        }
        mach_port_deallocate(mach_task_self(), thread_port);
        return false;
    }
}