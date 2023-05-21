
#include "TcpServer.h"

// tcp connect
class TcpConnect : public eco::net::TcpConnect
{
public:
};


class Handler
{
public:
    using SessionLogin = TReqSession<LoginReq, LoginRsp, ProtobufCodec, REQ_OPT_INIT>;
    static void login(SessionLogin* login)
    {
    }

    using SessionLogout = TReqSession<LoginReq, LoginRsp, ProtobufCodec, REQ_OPT_INIT>;
    static void logout(SessionLogout* logout)
    {
    }
};

// tcp server
// 特点：异步，简洁
eco::net::TcpServer& provider();
void uc_server()
{
    // connect扩展方式
    provider().set_connect<TcpConnect>();
    provider().set_address(eco::net::Address);

    // session其实并不存在，他只是实现这个接口的基础设置
    // 如果c++模板技术更好，则不需要这个参数：handler::login(SessionLogin*)
    // provider().dispatch(10004, handler::login);
    provider().dispatch<SessionLogin>(10004, handler::login);
    provider().dispatch<SessionLogout>(10005, handler::logout);
    provider().dispatch<SessionIer>(10006, handler::Ier);
    provider().dispatch<SessionOcr>(10007, handler::Ocr);
    provider().start();
}


void uc_client()
{
    
}


void uc_task_server()
{
    
}

void uc_task_client()
{
    
}

void uc_task_general()
{
    
}