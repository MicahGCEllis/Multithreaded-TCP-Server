#include "EventLoop.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include <vector>

class EpollEventLoop : public EventLoop
{
    private:
        int epoll_fd;
        SOCKET master_listener = INVALID_SOCKET;


    public:
        EpollEventLoop()
        {
            epoll_fd = epoll_create1(0);

            if (epoll_fd == -1)
            {
                throw std::runtime_error("Failed to create epoll file descriptor");
            }
        }
        ~EpollEventLoop() noexcept override
        {
            close(epoll_fd);
        }

        void RegisterSocket(SOCKET socket) override
        {
            epoll_event event;

            event.events = EPOLLIN;
            event.data.fd = socket;

            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &event);
        }

        void WaitForEvents(std::vector<SOCKET>& active_events, int timeout_ms) override
        {
            std::vector<epoll_event> events(64);

            int triggered = epoll_wait(epoll_fd, events.data(), 64, timeout_ms);

            for (int i = 0; i < triggered; ++i)
            {
                SOCKET triggered_fd = events[i].data.fd;

                if (triggered_fd == this->master_listener)
                {
                    SOCKET new_client = accept(master_listener, nullptr, nullptr);
                    active_events.push_back(new_client);
                }
                else
                {
                    active_events.push_back(triggered_fd);
                }
            }
        }

        void AsyncAccept(SOCKET listner) override
        {
            this->master_listener = listner;
        }
};

EventLoop* EventLoop::Create()
{
    return new EpollEventLoop();
}