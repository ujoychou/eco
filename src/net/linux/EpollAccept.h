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
#include "EpollWorker.h"
#include "../TcpWorker.h"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class EpollAccept : public eco::Object<EpollAccept>
{
public:
    // construct
    inline EpollAccept(TcpServerOption& option,
                       EpollWorker& accept,
                       EpollWorker& socket)
    {}

    // set tcp connect maker
    inline void set_connect(TcpConnect::Make make)
    {
        this->make_connect = make;
    }

    // listen and bind address port
    inline void listen(TcpEvent& tcp_event)
    {
        // bind address to socket
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(this->option.port());
        int err = bind(this->listen.fd, (struct socketaddr*)&addr, sizeof(addr));
        if (err < 0)
        {
            throw(-1);
        }

        // listen socket.
        err = ::listen(this->listen.fd, 3);
        if (err < 0)
        {
            throw(-1);
        }
        this->worker_accept.bind_accept(*this, tcp_event);
    }

    // start accept socket
    inline LinuxSocket::ref accept()
    {
        sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        int fd = ::accept(listen.fd, (sockaddr*)&addr, &addr_len);
        if (fd >= sockets.size())
        {
            eco::this_thread::set_error(fd);
        }
        LinuxSocket::ref sock = this->make_connect();
        sock->init(fd, addr, worker_socket.worker());
        return sock;
    }

    // listen socket
    LinuxSocket         listen;

private:
    TcpServerOption&    option;
    MakeLinuxSocket     make_connect;
    EpollWorker&        worker_accept;
    EpollWorkerPoll&    worker_socket;
};
typedef EpollAccept TcpAccept;


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)