#pragma once

#include <iostream>
#include "Config.hpp"
#include "Logger.hpp"

// Manges lifecycle of a single client connection
// Responsible for reading newtowrk butter, routing the file request, and sending HTTP responses
class ConnectionHandler
{
    private:
        SOCKET client_socket;
        Logger& logger; // Rerence ot global thread-safe logger
    public:
        ConnectionHandler(SOCKET client_socket, Logger& logger);
        void HandleConnection();
};