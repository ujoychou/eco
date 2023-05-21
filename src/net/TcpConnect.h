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
#include <eco/net/TcpSocket.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class TcpConnect
{
private:
    template<typename TTcpConnect>
    struct TcpConnectImpl : public TcpSocket
    {
        TTcpConnect connect;
    };

public:
    // make connect function of TTcpConnect
    template<typename TTcpConnect>
    inline static TcpSocket* make()
    {
        return new TcpConnectImpl<TTcpConnect>();
    }

    uint32_t id;
    eco::String user;
    eco::String lang;

    inline void auth(const char* user, const char* lang)
    {
        this->user = user;
        this->lang = lang;
    }

    inline TcpSocket& socket()
    {
        return *(TcpSocket*)(this - sizeof(TcpSocket));
    }
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)