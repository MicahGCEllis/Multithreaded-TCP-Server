#include "ConnectionHandler.hpp"
#include "Security.hpp"
#include "Telemetry.hpp"
#include "HttpStatus.hpp"
#include "Fiber.hpp"
#include <sstream>
#include <filesystem>

ConnectionHandler::ConnectionHandler(SOCKET client_socket, Logger& logger) : client_socket(client_socket), logger(logger) {};

void ConnectionHandler::HandleConnection()
{
    TelemetrySystem::Global_Telemetry_Ptr->active_connections++;
    MAKE_SOCKET_NON_BLOCKING(client_socket);
    auto time_start = std::chrono::high_resolution_clock::now();

    std::string client_message = "";
    
    while (true){
        char buffer[Config::BUFFER_SIZE] = {0};
        int bytes_read = recv(client_socket, buffer, Config::BUFFER_SIZE, 0);

        // Only process if actual data was recieved
        if (bytes_read > 0)
        {
            TelemetrySystem::Global_Telemetry_Ptr->total_bytes_received += bytes_read;

            // Construct string suing exact bytes_read to preven out-of-bounds memeory reading
            client_message.append(buffer, bytes_read);

            if (client_message.find("\r\n\r\n") != std::string::npos) {break;}
        }

        else if (bytes_read == 0)
        {
            closesocket(client_socket);
            TelemetrySystem::Global_Telemetry_Ptr->active_connections--;
            return;
        }

        else if (bytes_read == SOCKET_ERROR)
        {
            if (NET_ERROR() == NET_ERR_WOULDBLOCK)
            {
                Fiber::YieldTo(Fiber::scheduler_fiber);
            }
            else 
            {
            closesocket(client_socket); 
            TelemetrySystem::Global_Telemetry_Ptr->active_connections--;
            return;
            }
        }
    }

    std::istringstream request_stream(client_message);
            
    std::string method;
    std::string path;

    // Extract HTTP method and target URL from the request stream
    request_stream >> method >> path;

    TelemetrySystem::Global_Telemetry_Ptr->total_requests++;

    // Route empty root requests to default homepage
    if (path == "/")
    {
        path = "/index.html";
    }

    // String leading forward slash to prevent absolute path confusion on the local OS
    path = path.substr(1);

    // Security Checkpoint: Block directory traveral attempts (e.g., ../)
    if (!Security::isPathSafe(path))
    {
        logger.LogTraffic(method,path, status::FORBIDDEN);
        std::string response = "HTTP/1.1 403 Forbidden\r\nContent-Length: 11\r\n\r\nForbidden";
        send(client_socket, response.c_str(), response.size(), 0);
        TelemetrySystem::Global_Telemetry_Ptr->total_bytes_sent += response.size();
        closesocket(client_socket);
        TelemetrySystem::Global_Telemetry_Ptr->dropped_connections++;
        TelemetrySystem::Global_Telemetry_Ptr->active_connections--;
        return;
    }

    path = "../public/" + path; // Prepend public directory to ensure all requests are relative to it

    std::cout << "CRITICAL MEASUREMENT: OS is looking for file at: " 
          << std::filesystem::absolute(path) << std::endl;

    std::ifstream file(path);

    std::string content_type = "text/html"; // Default MIME type

    // MIME Type Router: Dynamically adjust heads based on file extension
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

    // Server requested or throw 404 error
    if (file.is_open())
    {
        std::stringstream file_buffer; 
        file_buffer << file.rdbuf(); // Read entire file inot memeory

        std::string body = file_buffer.str();

        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type + "\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n";
        response += body;

        logger.LogTraffic(method, path, status::OK);
        send(client_socket, response.c_str(), response.size(), 0);
        TelemetrySystem::Global_Telemetry_Ptr->total_bytes_sent += response.size();
    }

    else
    {
        std::string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
        logger.LogTraffic(method, path, status::NOT_FOUND);
        send(client_socket, response.c_str(), response.size(), 0);
        TelemetrySystem::Global_Telemetry_Ptr->total_bytes_sent += response.size();
    }

    auto time_end = std::chrono::high_resolution_clock::now();
    uint64_t current_total = TelemetrySystem::Global_Telemetry_Ptr->total_requests;
    uint64_t current_avg = TelemetrySystem::Global_Telemetry_Ptr->average_response_time_ms;
    uint64_t response_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count(); 
    TelemetrySystem::Global_Telemetry_Ptr->average_response_time_ms = (current_avg * (current_total - 1) + response_time_ms) / current_total;

    // Always close the socket to prevent resource leaks
    closesocket(client_socket);
    TelemetrySystem::Global_Telemetry_Ptr->active_connections--;
}