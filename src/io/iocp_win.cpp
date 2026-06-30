#include "EventLoop.hpp"
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>

enum class IoOperation
{
    Accept,
    Read,
    Write
};

struct PerIoContext
{
    WSAOVERLAPPED overlapped;
    IoOperation operation_type;
    SOCKET client_socket;
    SOCKET listener_socket;
    DWORD bytes_received;
    char buffer[128];
};

class IocpEventLoop : public EventLoop
{
    private:
        HANDLE iocp_handle;
        LPFN_ACCEPTEX AcceptExPtr = nullptr;

    public:
        IocpEventLoop()
        {
            iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

            SOCKET dummy = socket(AF_INET, SOCK_STREAM, 0);
            GUID guid = WSAID_ACCEPTEX;
            DWORD bytes;

            int status = WSAIoctl(dummy, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &this->AcceptExPtr, sizeof(this->AcceptExPtr), &bytes, nullptr, nullptr);

            if (status == SOCKET_ERROR)
            {
                throw std::runtime_error("Failed to load AcceptEx");
            }

            closesocket(dummy);

            if (iocp_handle == NULL)
            {
                throw std::runtime_error("Failed to create IOCP handle");
            }
        }
        ~IocpEventLoop() noexcept override
        {
            PostQueuedCompletionStatus(iocp_handle, 0, 0, nullptr);
            CloseHandle(iocp_handle);
        }

        void RegisterSocket(SOCKET socket) override
        {
            CreateIoCompletionPort((HANDLE)socket, iocp_handle, 0, 0);

            // THE KERNEL SHIELD: Ask Windows if this is the Front Door
            BOOL is_listening = FALSE;
            int optlen = sizeof(BOOL);
            getsockopt(socket, SOL_SOCKET, SO_ACCEPTCONN, (char*)&is_listening, &optlen);

            // Only arm the 0-byte read if this is an actual Web Browser!
            if (!is_listening)
            {
                PerIoContext* context = new PerIoContext();
                ZeroMemory(&context->overlapped, sizeof(WSAOVERLAPPED));
                context->operation_type = IoOperation::Read;
                context->client_socket = socket;

                WSABUF wsaBuf;
                wsaBuf.buf = nullptr;
                wsaBuf.len = 0;
                DWORD flags = 0;

                int rc = WSARecv(socket, &wsaBuf, 1, nullptr, &flags, &context->overlapped, nullptr);
                if (rc == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
                {
                    delete context;
                    return;
                }
            }
        }

        void WaitForEvents(std::vector<SOCKET>& active_events, int timeout_ms) override
        {
            OVERLAPPED* overlapped = nullptr;
            DWORD bytes_transferred;
            ULONG_PTR completion_key;

            bool success = GetQueuedCompletionStatus(iocp_handle, &bytes_transferred, &completion_key, &overlapped, timeout_ms);

            if (overlapped != nullptr)
            {
                PerIoContext* context = reinterpret_cast<PerIoContext*>(overlapped);

                if (success && context->operation_type == IoOperation::Accept)
                {
                    setsockopt(context->client_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                               reinterpret_cast<char*>(&context->listener_socket), sizeof(context->listener_socket));
                    active_events.push_back(context->client_socket);
                }
                else if (success && context->operation_type == IoOperation::Read)
                {
                    active_events.push_back(context->client_socket);
                }
                else if (!success && context->operation_type == IoOperation::Accept)
                {
                    // Failed accept: close the ghost socket and signal that a re-arm is needed
                    closesocket(context->client_socket);
                    active_events.push_back(INVALID_SOCKET);
                }
                else if (!success && context->operation_type == IoOperation::Read)
                {
                    // Client disconnected or network error: push socket so the fiber can clean up
                    active_events.push_back(context->client_socket);
                }
                delete context;
            }
            else
            {
                return;
            }
        }

        bool AsyncAccept(SOCKET listener)
        {
            PerIoContext* context = new PerIoContext();
            ZeroMemory(&context->overlapped, sizeof(WSAOVERLAPPED));
            context->operation_type = IoOperation::Accept;
            context->listener_socket = listener;

            context->client_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP,NULL, 0, WSA_FLAG_OVERLAPPED);

            bool call = AcceptExPtr(listener, context->client_socket, context->buffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &context->bytes_received, &context->overlapped);

            if (call)
            {
                return call;
            }
            else
            {
                if (WSAGetLastError() == WSA_IO_PENDING)
                {
                    return !call;
                }
                else
                {
                    closesocket(context->client_socket);
                    delete context;
                    return call;
                }
            }
        }
};

EventLoop* EventLoop::Create()
{
    return new IocpEventLoop();
}