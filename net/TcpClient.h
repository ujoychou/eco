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
class ECO_API TcpClientHandler
{
public:

};


////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpClient : public TcpClientHandler
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

	// create tcp session.
	template<typename SessionDataT>
	inline eco::net::TcpSession create_session()
	{
		return create_session(make_session_data<SessionDataT>(0));
	}
	// create tcp session.
	eco::net::TcpSession create_session(IN SessionData* data);

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
		IN eco::net::Codec& codec,
		IN const uint32_t session_id,
		IN const uint32_t msg_type,
		IN const MessageModel model,
		IN const uint32_t request_id = 0,
		IN const uint32_t category = category_message);

	// async send request to server.
	void async_request(
		IN eco::net::Codec& codec,
		IN const uint32_t session_id,
		IN const uint32_t msg_type,
		IN const uint32_t request_id = 0,
		IN const uint32_t category = category_message);

	// async send response to client.
	void async_response(
		IN eco::net::Codec& codec,
		IN const uint32_t msg_type,
		IN const uint32_t session_id,
		IN const uint32_t request_id = 0,
		IN const uint32_t category = category_message);
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif