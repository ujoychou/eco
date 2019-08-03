#ifndef VALUES_CTP_FRONT_LOGIN_HANDLER_H
#define VALUES_CTP_FRONT_LOGIN_HANDLER_H
/*******************************************************************************
@ ����

@ ����

@ �쳣

@ ��ע

--------------------------------------------------------------------------------
@ ��ʷ��¼ @
@ ujoy modifyed on 2016-11-02

--------------------------------------------------------------------------------
* ��Ȩ����(c) 2016 - 2019, siberia corp, ��������Ȩ����

*******************************************************************************/
#include <eco/Project.h>
#include <eco/proto/Test.pb.h>
#include <eco/net/ProtobufHandler.h>


ECO_NS_BEGIN(test);
////////////////////////////////////////////////////////////////////////////////
class PingHandler :
	public eco::net::ProtobufHandler<proto::TestReq>,
	public std::enable_shared_from_this<PingHandler>
{
	ECO_HANDLER(1, 2, "login", -1);
public:
	virtual void on_request() override;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(test)
#endif