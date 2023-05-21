
#pragma once
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2023 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include "LinuxSocket.h"


ECO_NS_BEGIN(eco)
ECO_NS_BEGIN(net)
class EpollAccept;
////////////////////////////////////////////////////////////////////////////////
class EpollWorker : public eco::Object<EpollWorker>
{
public:
    enum
    {
        MAX_ACTIVE_EVENTS = 64,
    };

    // create epoll fd.
    inline EpollWorker() : active_events(MAX_ACTIVE_EVENTS)
    {
        this->fd_epoll = ::epoll_create1(EPOLL_CLOEXEC);
        if (this->fd_epoll <= 0)
        {
            throw(-1);
        }
        this->fd_notify = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (this->fd_notify <= 0)
        {
            throw(-1);
        }
    }

    // close epoll fd
    inline ~EpollWorker()
    {
        if (this->fd_epoll > 0)
        {
            ::close(this->fd_epoll);
        }
        if (this->fd_notify > 0)
        {
            ::close(this->fd_notify);
        }
    }

    // run worker thread
    inline void run(const char* name)
    {
        this->thread.run(name, std::bind(&EpollWorker::work, this));
    }

    // stop worker thread
    inline void stop()
    {
        this->running = false;
        this->notify_poll();
        this->thread.join();
    }

    // worker is running
    inline bool stopped()
    {
        return this->running == 0;
    }

    // post task to this work
    inline void post(eco::Task& task)
    {
        {
            std::lock_guard<std::mutex> lock(this->mutex);
            this->tasks.emplace_back(std::move(task));
        }
        this->notify_poll();
    }

public:
    inline void socket_listen_add(LinuxSocket::ref& sock)
    {
        // edge triger: read(listen)
        epoll_add(sock, EPOLLIN | EPOLLET);
    }

    inline void socket_client_add(LinuxSocket& sock)
    {
        // edge triger: read & wirte
        epoll_add(sock, EPOLLIN | EPOLLOUT | EPOLLET);
    }

    inline void epoll_add(LinuxSocket& sock, uint32_t events)
    {
        struct epoll_event event;
        memset(&event, 0, sizeof(event));
        event.events = events;
        event.data.ptr = &sock;
        int err = ::epoll_ctl(this->fd_epoll, EPOLL_CTL_ADD, sock.fd, &event);
        if (err < 0)
        {
            throw(-1);
        }
    }

    inline void epoll_del(LinuxSocket& sock)
    {
        struct epoll_event event;
        memset(&event, 0, sizeof(event));
        event.events = events;
        event.data.ptr = &sock;
        int err = ::epoll_ctl(this->fd_epoll, EPOLL_CTL_DEL, sock.fd, &event);
        if (err < 0)
        {
            throw(-1);
        }
    }

    inline LinuxSocket::ref get_socket(struct epoll_event& event)
    {
        LinuxSocket* sock = (LinuxSocket*)event.data.ptr;
        return LinuxSocket::ref(sock);
    }

    // get socket error
    inline bool set_error(int fd, int event)
    {
        if (event.events & (POLLERR | POLLNVAL))
        {
            int err = 0;
            const char* msg = sockets::getSocketError(err, fd);
            eco::this_thread::set_error(err, msg);
            return true;
        }
        return false;
    }

    // bind server accept
    void bind_accept(LinuxAccept& accept);

    // epoll thread work function
    void work();

    // notify epoll thread wakeup when poll wait
    void notify_poll();
    void notify_read();

private:
    // server listen socket
    LinuxAccept* accept = 0;

    // epoll events
    int fd_epoll = 0;
    int fd_notify = 0;
    TcpEvent* event = 0;
    std::vector<epoll_event> active_events;

    // thread task queue
    std::mutex tasks_mutex;
    std::vector<eco::Task> tasks;

    // epoll thread
    int running = false;
    eco::Thread thread;
};
typedef EpollWorker TcpWorker;


////////////////////////////////////////////////////////////////////////////////
void LinuxSocket::release(LinuxSocket* sock)
{
    sock.worker.post([sock]() { delete sock; } );
}

////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)
