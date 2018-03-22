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
	ECO_SHARED_API(TcpClient);
public:
	// network server option.
	void set_option(IN const TcpClientOption&);
	TcpClientOption& option();
	const TcpClientOption& get_option() const;
	TcpClient& option(IN const TcpClientOption&);

	// service name.
	inline const char* get_service_name() const
	{
		return get_option().get_service_name();
	}

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

	// get connection id.
	ConnectionId get_id();

	// set session data class and tcp session mode.
	template<typename SessionDataT>
	inline void set_session_data()
	{
		set_session_data(&make_session_data<SessionDataT>);
	}
	void set_session_data(IN MakeSessionDataFunc make);

	// register connection event
	void set_event(IN OnConnectFunc on_connect, IN OnCloseFunc on_close);

	// dispatch.
	DispatchRegistry& dispatcher();

	// release client.
	void close();

//////////////////////////////////////////////////////////////////// ROUTER MODE
public:
	// router mode: async call cluster init router.
	void async_connect_router(
		IN const char* router_name,
		IN eco::net::AddressSet& router_addr);

	// router mode: async connect to service of router.
	void async_connect(
		IN const char* router_name,
		IN const char* service_name);

/////////////////////////////////////////////////////////////////// SERVICE MODE
public:
	// open session.
	TcpSession open_session();

	// service mode: set address ready to connect to service.
	void set_address(IN eco::net::AddressSet&);

	// service mode: async connect to service.
	void async_connect(IN eco::net::AddressSet&);
	void async_connect();

	// async send request to server.
	void async_auth(IN TcpSession& session, IN MessageMeta& meta);

	// async send message.
	void async_send(IN eco::String& data, IN const uint32_t start);

	// async send message.
	void async_send(IN MessageMeta& meta);

	// req/rsp mode: send message and get a response.
	template<typename codec_t, typename req_t, typename rsp_t>
	inline eco::Result request(
			IN const uint32_t req_type, IN req_t& req,
			IN const uint32_t rsp_type, IN rsp_t& rsp,
			IN const bool encrypted = true)
	{
		codec_t codec_req(req);
		codec_t codec_rsp(rsp);
		MessageMeta meta_req(codec_req, none_session, req_type, encrypted);
		return request(meta_req, codec_rsp);
	}
	template<typename codec_t, typename msg_t>
	inline eco::Result request(
		IN const uint32_t req_type,
		IN const uint32_t rsp_type,
		IN msg_t& msg,
		IN const bool encrypted = true)
	{
		codec_t codec_rsp(msg);
		MessageMeta meta_req(codec_t(msg), none_session, req_type, encrypted);
		return request(meta_req, codec_rsp);
	}

#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void async_send(
		IN google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const uint32_t request_data,
		IN const bool encrypted = true)
	{
		ProtobufCodec codec(msg);
		MessageMeta meta(codec, none_session, type, encrypted);
		meta.set_request_data(request_data);
		async_send(meta);
	}

	// sync request protobuf.
	inline eco::Result request(
		IN const uint32_t req_type,
		IN google::protobuf::Message& req_msg,
		IN const uint32_t rsp_type,
		IN google::protobuf::Message& rsp_msg,
		IN const bool encrypted = true)
	{
		return request<ProtobufCodec>(
			req_type, req_msg, rsp_type, rsp_msg, encrypted);
	}

	// sync request protobuf.
	inline eco::Result request(
		IN const uint32_t req_type,
		IN const uint32_t rsp_type,
		IN google::protobuf::Message& msg,
		IN const bool encrypted = true)
	{
		return request<ProtobufCodec>(req_type, rsp_type, msg, encrypted);
	}
#endif

private:
	eco::Result request(IN MessageMeta& req, IN Codec& rsp);
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif