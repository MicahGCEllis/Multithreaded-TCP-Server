#include "Telemetry.hpp"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <stdexcept>

void TelemetrySystem::InitTelemetryBridge()
{
    int fd = open("../planet.dat", O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        throw std::runtime_error("Failed to open telemetry storage file");
    }

    if (ftruncate(fd, sizeof(Telemetry)) == -1)
    {
        close(fd);
        throw std::runtime_error("Failed to size telemetry storage file");
    }

    void* mapped = mmap(nullptr, sizeof(Telemetry), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED)
    {
        close(fd);
        throw std::runtime_error("Failed to memory map telemetry shared memory block");
    }

    TelemetrySystem::Global_Telemetry_Ptr = static_cast<Telemetry*>(mapped);
    close(fd); // File descriptor can safely close; mapping persists in virtual memory
}