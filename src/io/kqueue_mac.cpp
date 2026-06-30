#include "EventLoop.hpp"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/event.h>
#include <unistd.h>
#include <stdexcept>

class KqueueEventLoop : public EventLoop
{
    private:
        int kq_fd;
        SOCKET master_listener = INVALID_SOCKET;

    public:
        KqueueEventLoop()
        {
            kq_fd = kqueue();

            if (kq_fd == -1)
            {
                throw std::runtime_error("Failed to create Kqueue file descriptor");
            }
        }

        ~KqueueEventLoop() noexcept
        {
            close(kq_fd);
        }

        void RegisterSocket(SOCKET socket) override
        {
            struct kevent event;

            EV_SET(&event, socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

            kevent(kq_fd, &event, 1, NULL, 0, NULL);
        }

        void WaitForEvents(std::vector<SOCKET>& active_events, int timeout_ms) override
        {
            std::vector<struct kevent> events(64);
            struct timespect ts;
            ts.tv_sec = timeout_ms / 1000;
            ts.tv_nsec = (timeout_ms % 1000) * 100000

            int triggered = kevent(kq_fd, NULL, 0, events.data(), 64, &ts);

            for (int i = 0; i < triggered; ++i)
            {
                SOCKET triggered_fd = events[i].ident;

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
    return new KqueueEventLoop();
}