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
#include <eco/Object.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class TcpEvent
{
public:
    // tcp server event
    virtual void on_accept() = 0;

    // tcp client event
    virtual void on_connect() = 0;

    // tcp event: close tcp socket
    virtual void on_close() = 0;

    // tcp event: recv message from tcp socket
    virtual void on_recv() = 0;

    // tcp event: recv message to tcp socket
    virtual void on_send() = 0;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)