#pragma once
/*******************************************************************************
@ name


@ function


@ exception


@ remark


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/rx/RxApi.h>
#include <eco/net/TcpOption.h>
#include <eco/net/Protocol.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpServer
{
	ECO_OBJECT_API(TcpServer);
public:
	/*@ start tcp server.*/
	void start();

	/*@ stop this server, and wait all the request in server to be handled.*/
	void stop();

	/*@ join this server threads.*/
	void join();

	// network server option.
	TcpOptionServer& option();

	// set connection data type.
	template<typename TTcpConnect>
	inline void set_connect()
	{
		set_connect(&TcpConnect::make<TTcpConnect>);
	}

	// register route of message handler
	template<typename TTcpSession>
	inline void route(int id, TTcpSession::Handler&& handler)
	{
		TcpSession::Handler handler_tcp = std::bind(
			&TTcpSession::on_request, std::placeholder::_1, handler);
		this->route(id, TTcpSession::make, handler_tcp);
	}

	// register route of message default handler
	void route_default(TcpSession::Handler&& handler);

public:
	// set tcp protocol
	void protocol_tcp(ProtocolTcp* tcp);

	// template protocol derived from protocol
	template<typename TProtocol>
	inline void protocol_add()
	{
		protocol_add(new TProtocol());
	}

	// get latest protocol
	Protocol* protocol_latest() const;

	// get protocol by version
	Protocol* protocol(int version) const;

public:
	// set connection open close event
	void set_event(OnAccept&&, OnClose&&);

	// set receive callback event.
	void set_recv_event(OnRecvData&&, OnDecodeHead&&);

	// is receive mode: using recv callback event to handle message.
	bool receive_mode() const;

	// is dispatch mode: using eco dispatch mechanism to handle message.
	bool dispatch_mode() const;

private:
	// websocket, protocol-eco, ftdc, so on.
	void protocol_add(IN Protocol* heap);

	// set connection make func
	void set_connect(TcpConnect::Make make);
	
	// set route of session handler
	void route(int id, TcpSession::Make make, TcpSession::Handler& handler);
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)