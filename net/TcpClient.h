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
	void set_protocol_head(IN ProtocolHead*);
	ProtocolHead& protocol_head() const;

	// protocol
	void set_protocol(IN Protocol*);
	Protocol& protocol() const;

	// dispatch.
	DispatchRegistry& dispatcher();

public:
	// async connect to service.
	void async_init_service(
		IN const char* service_name_,
		IN eco::net::AddressSet& service_addr);

	// produce next request id.
	uint32_t next_request_id();

	// async send message.
	void async_send(IN eco::String& data);

	// async send message.
	void async_send(
		IN Codec& codec,
		IN const uint32_t session_id,
		IN const uint32_t msg_type,
		IN const MessageModel model,
		IN const MessageCategory category = category_message);

	// async send request to server.
	void async_authorize(
		IN Codec& codec,
		IN TcpSession& session,
		IN const uint32_t msg_type);
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif