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
#include <eco/log/Log.h>
#include <eco/net/Protocol.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
template<bool check_auth, bool check_app_ready>
struct ReqOption
{
};
using REQ_OPT_AUTH = ReqOption<true, true>;
using REQ_OPT_INIT = ReqOption<false, true>;


////////////////////////////////////////////////////////////////////////////////
template<int req_srv, int rsp_srv, int err_srv>
struct ReqLogger
{
};
using REQ_LOG_INFO = ReqLogger<info, info, error>;
using REQ_LOG_DEBUG = ReqLogger<debug, debug, error>;


////////////////////////////////////////////////////////////////////////////////
class TcpSession : public eco::RxHeap
{
public:
    // session handle event
    typedef void (*RspFunc)(TcpSession&);
    typedef void (*ReqFunc)(TcpSession::ptr&);
    typedef std::function<void(TcpSession::ptr&)> Handler;
    typedef TcpSession::ptr (*Make)(MessageMeta&, MessageTcp&);

    // 1.session create
    static inline TcpSession::ptr make(MessageMeta& meta, MessageTcp& data)
    {
        return std::make_shared<TcpSession>(meta, data);
    }
};


////////////////////////////////////////////////////////////////////////////////
template<
typename TReq,
typename TRsp,
typename TCodec,
typename ReqOption = REQ_OPT_AUTH,
typename ReqLogger = REQ_LOG_INFO>
class TReqSession : public TcpSession
{
    ECO_OBJECT(TReqSession);
public:
    // session handle event
    typedef TReqSession<TReq, TRsp, TCodec, ReqOption, ReqLogger> ReqSession;
    typedef void (*RspFunc)(ReqSession&);
    typedef void (*ReqFunc)(ReqSession::ptr&);
    typedef std::function<void(ReqSession::ptr&)> Handler;

    // 1.session create
    inline static TcpSession::ptr make(IN MessageMeta& meta, IN TcpData& data)
    {
        return std::make_shared<ReqSession>(meta, data);
    }

private:
    // 2.session constructor.
    inline TReqSession(IN MessageMeta& meta, IN TcpData& data) 
        : TcpSession(meta, data)
    {}

    // 3.decode tcp message to message.
    virtual bool on_decode() override
    {
        TCodec codec(&this->req);
        return codec.decode(this->TcpSession::data, this->TcpSession::size);
    }

public:
    // 4.handle request message.
    inline static void on_request(TcpSession::ptr& sess, ReqFunc on_req)
    {
        // cast session and notify event.
        on_req(std::dynamic_cast<ReqSession>(sess));
    }

public:
    TReq req;
    TRsp rsp;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)