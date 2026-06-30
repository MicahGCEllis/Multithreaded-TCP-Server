#include "Telemetry.hpp"
#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>

void TelemetrySystem::InitTelemetryBridge()
    {
        TelemetrySystem::Global_Telemetry_Ptr = new Telemetry();
        std::thread([]()
        {
            HANDLE hPipe =
                CreateNamedPipeA("\\\\.\\pipe\\planet9_telemetry",
                                 PIPE_ACCESS_OUTBOUND,
                                 PIPE_TYPE_MESSAGE | PIPE_WAIT,
                                 1,
                                 sizeof(Telemetry),
                                 sizeof(Telemetry),
                                 0,
                                 nullptr);
            DWORD written = 0;
            while(true)
            {
                ConnectNamedPipe(hPipe, nullptr);
                while(true)
                {
                    if (WriteFile(hPipe, Global_Telemetry_Ptr, sizeof(Telemetry), &written, nullptr))
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    }
                    else 
                    {
                        std::cerr << "Failed to write telemetry data to pipe: " << std::endl;
                        break;
                    }
                }
            DisconnectNamedPipe(hPipe);
        }
        }).detach();        
    }