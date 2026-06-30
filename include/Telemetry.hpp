#pragma once
#include <cstdint>

struct Telemetry
{
    uint64_t total_requests = 0;
    uint64_t active_connections = 0;
    uint64_t total_bytes_sent = 0;
    uint64_t total_bytes_received = 0;
    uint64_t fiber_saturation = 0;
    uint64_t dropped_connections = 0;
    uint64_t average_response_time_ms = 0;
};

namespace TelemetrySystem
{

// Pointer to the global telemetry struct
inline Telemetry* Global_Telemetry_Ptr = nullptr;

void InitTelemetryBridge();
}