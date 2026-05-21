/*
    Use recv() system call to read bytes sent by client
    Parse those bytes (reading HTTP request or simple text message)
    Use send() system call to push response back to client and finally close that client's socket
*/

#include "../include/ConnectionHandler.hpp"
#include "../include/Security.hpp"
#include <sstream>

ConnectionHandler::ConnectionHandler(SOCKET client_socket, Logger& logger)
: client_socket(client_socket), logger(logger)
{
}

void ConnectionHandler::HandleConnection()
{
    char buffer[Config::BUFFER_SIZE] = {0};
    int bytes_read = recv(client_socket, buffer, Config::BUFFER_SIZE, 0);

    if (bytes_read > 0)
    {
        std::string client_message(buffer, bytes_read);
        std::istringstream request_stream(client_message);
        this->logger.Log("Received data from client: " + client_message);
        std::string method;
        std::string path;

        request_stream >> method >> path;

        if (path == "/")
        {
            path = "/index.html";
        }

        path = path.substr(1);

        if (!Security::isPathSafe(path))
        {
            logger.Log("SECURITY ALERT: Unsafe path detected: " + path);
            std::string response = "HTTP/1.1 403 Forbidden\r\nContent-Length: 11\r\n\r\nForbidden";
            send(client_socket, response.c_str(), response.size(), 0);
            closesocket(client_socket);
            return;
        }

        std::ifstream file(path);

        std::string content_type = "text/html";
        
        if (path.find_last_of('.') != std::string::npos)
        {
            std::string extension = path.substr(path.find_last_of('.'));

            if (extension == ".html")
            {
                content_type = "text/html";
            }
            else if (extension == ".css")
            {
                content_type = "text/css";
            }
            else if (extension == ".js")
            {
                content_type = "application/javascript";
            }

            
        }

        if (file.is_open())
        {
            std::stringstream file_buffer;
            file_buffer << file.rdbuf();
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type + "\r\n\r\n";
            response += file_buffer.str();
            send(client_socket, response.c_str(), response.size(), 0);
        }

        else
        {
            std::string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
            send(client_socket, response.c_str(), response.size(), 0);
        }
    }
    closesocket(client_socket);
}