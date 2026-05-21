/*
    Delcare class and set of function that tkae a client socket as an argument
*/

#pragma once

#include <winsock2.h>
#include <iostream>
#include "Config.hpp"
#include "Logger.hpp"

class ConnectionHandler
{
    private:
        SOCKET client_socket;
        Logger& logger;
    public:
        ConnectionHandler(SOCKET client_socket, Logger& logger);
        void HandleConnection();
};