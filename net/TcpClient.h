#ifndef ECO_NET_TCP_CLIENT_H
#define ECO_NET_TCP_CLIENT_H
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-12-14.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Project.h>
#include <eco/net/Address.h>
#include <eco/net/TcpSession.h>
#include <eco/net/DispatchRegistry.h>
#include <eco/net/TcpClientOption.h>
#include <eco/net/protocol/TcpProtocol.h>


////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpClient
{
	ECO_OBJECT_API(TcpClient);
public:
	// service name.
	void set_service_name(IN const char*);
	const char* get_service_name() const;
	TcpClient& service_name(IN const char*);

	// protocol head.
	template<typename ProtocolHeadT>
	inline void set_protocol_head()
	{
		set_protocol_head(new ProtocolHeadT());
	}
	void set_protocol_head(IN ProtocolHead*);
	ProtocolHead& protocol_head() const;

	// protocol
	template<typename ProtocolT>
	inline void set_protocol()
	{
		set_protocol(new ProtocolT());
	}
	void set_protocol(IN Protocol*);
	Protocol& protocol() const;

	// set session data class and tcp session mode.
	template<typename SessionDataT>
	inline void set_session_data()
	{
		set_session_data(&make_session_data<SessionDataT>);
	}
	void set_session_data(IN MakeSessionDataFunc make);

	// dispatch.
	DispatchRegistry& dispatcher();

	// produce next request id.
	// TODO
	uint32_t next_request_id();

	// release client.
	void release();

//////////////////////////////////////////////////////////////////// ROUTER MODE
public:
	// router mode: async call cluster init router.
	void async_init_router(
		IN const char* router_name,
		IN eco::net::AddressSet& router_addr);

	// router mode: async connect to service of router.
	void async_init(
		IN const char* router_name,
		IN const char* service_name);

/////////////////////////////////////////////////////////////////// SERVICE MODE
public:
	// open session.
	TcpSession open_session();

	// service mode: async connect to service.
	void async_connect(
		IN eco::net::AddressSet& service_addr);

	// async send request to server.
	void async_auth(IN TcpSession& session, IN MessageMeta& meta);

	// async send message.
	void async_send(IN eco::String& data);

	// async send message.
	void async_send(IN MessageMeta& meta);

	
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif