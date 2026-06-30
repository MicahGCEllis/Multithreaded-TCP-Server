#include "CpuAffinity.hpp"
#include <windows.h>

namespace Affinity
{
    bool PinThreadToCore(int core_id)
    {
        HANDLE handle = GetCurrentThread();
        DWORD_PTR mask = 1ULL << core_id;

        if (SetThreadAffinityMask(handle, mask) == 0)
        {
            return false;
        }
        return true;
    }
}