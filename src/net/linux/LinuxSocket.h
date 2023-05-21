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
#include <eco/rx/RxHeap.h>


ECO_NS_BEGIN(eco)
ECO_NS_BEGIN(net)
class EpollWorker;
////////////////////////////////////////////////////////////////////////////////
class LinuxSocket : public eco::RxHeap
{
    ECO_NONCOPYABLE(LinuxSocket);
public:
    typedef eco::Autoref<LinuxSocket> ref;

    // create socket fd.
    inline LinuxSocket()
    {
        this->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (this->fd <= 0)
        {
            throw (-1);
        }
    }

    // auto close socket fd
    virtual ~LinuxSocket()
    {
        this->close();
    }

    inline void init(int fd, sockaddr_in* addr,
                    EpollWorker& worker, Router& router)
    {
        this->fd = fd;
        this->addr = *addr;
        this->worker = &worker;
        this->router = &router;
    }

    // close socket fd
    inline void close()
    {
        if (this->fd != 0)
        {
            close(this->fd);
            this->fd = 0;
        }
    }

    // check whether socket is valid
    inline bool operator() const
    {
        return this->fd != 0;
    }

    // socket setting, reuse address/port. so that:
    // you can start two tcp server listen on same port.
    inline bool reuse_address_port()
    {
        int opt = 1;
        int opt_reuse = SO_REUSEADDR | SO_REUSEPORT;
        return ::setsockopt(this->socket.fd, SOL_SOCKET,
                            opt_reuse, &opt, sizeof(opt));
    }

    int fd = 0;
    int id = 0;
    struct sockaddr_in addr;
    eco::String buffer;

    // autoref ptr
    inline void ref_add() { ++refcount; }
    inline void ref_del() { if (--refcount ==0) { release(this); } }
    inline uint32_t ref_size() { return refcount; }

private:
    inline static void release(TcpSocket* sock);
    EpollWorker* worker = 0;
    eco::net::Router* router = 0;
    std::atomic<uint32_t> refcount;
};


////////////////////////////////////////////////////////////////////////////////
// make connect function
typedef LinuxSocket::ref (*MakeLinuxSocket)(void);
typedef LinuxSocket TcpSocket;
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)
