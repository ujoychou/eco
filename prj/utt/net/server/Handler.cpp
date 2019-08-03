#include "PrecHeader.h"
#include "Handler.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Memory.h>
#include "App.h"


ECO_NS_BEGIN(test);
////////////////////////////////////////////////////////////////////////////////
void PingHandler::on_request()
{
	eco::String ip = std::move(connection().get_ip());
	ECO_LOG_REQ(info) << request().int1() <= request().int2()
		<< request().int3() <= request().int4()
		<= request().str1() <= request().str2()
		<= request().str3() <= request().str4()
		<= request().dbl1() <= request().dbl2()
		<= request().dbl3() <= request().dbl4();

	proto::TestRsp rsp;
	*rsp.mutable_data() = request();
	for (size_t i = 0; i < 1000; i++)
	{
		rsp.mutable_data()->set_int1(++get_app().m_req_id);
		connection().publish(rsp, 100, eco::snap_none);
		connection().publish(rsp, 100, eco::snap_none);
		connection().publish(rsp, 100, eco::snap_none);
		connection().publish(rsp, 100, eco::snap_none);
	}
	response(rsp, response_type(), true);
	ECO_LOG_RSP(info, rsp, *this);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(test)
