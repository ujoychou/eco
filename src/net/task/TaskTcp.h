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
#include <Task.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class TcpClientTask : public Task
{
public:
    inline TcpClientTask(TcpClient& c) : client(c)
    {}

    // request
    inline void req()
    {

    }

    virtual void async() override
    {
        this->client.worker().post();
    }

private:
    TcpClient& client;
};


////////////////////////////////////////////////////////////////////////////////
class TaskGraphFeature
{
    void start()
    {
        TcpClient trade_srv;
        proto::User user;
        Task* task = new TcpClientTask(trade_srv);
        task->request_id(10001);
        task->request_encode(user);
        task->async(on_user);
    }

    void on_user()
    {
        
    }
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco)