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
#include <eco/net/Address.h>
#include <eco/net/TcpSession.h>
#include <eco/net/TcpDispatch.h>
#include <eco/net/TcpClientOption.h>
#include <eco/net/protocol/TcpProtocol.h>


////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpClient : public TcpDispatch
{
	ECO_SHARED_API(TcpClient);
public:
	// network server option.
	void set_option(IN const TcpClientOption&);
	TcpClientOption& option();
	const TcpClientOption& get_option() const;
	TcpClient& option(IN const TcpClientOption&);

	// service name.
	inline const char* get_name() const
	{
		return get_option().get_name();
	}

	// set timeout
	void set_timeout(IN const uint32_t millsec);

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

	// set connection data type.
	template<typename ConnectionDataT>
	inline void set_connection_data()
	{
		set_connection_data(&make_connection_data<ConnectionDataT>);
	}
	void set_connection_data(IN MakeConnectionDataFunc make);

	template<typename ConnectionDataT>
	inline ConnectionDataT& cast()
	{
		return *static_cast<ConnectionDataT*>(data());
	}
	ConnectionData* data();

	// set session data class and tcp session mode.
	template<typename SessionDataT>
	inline void set_session_data()
	{
		set_session_data(&make_session_data<SessionDataT>);
	}
	void set_session_data(IN MakeSessionDataFunc make);

	// register connection event
	void set_event(IN OpenFunc on_open, IN CloseFunc on_close = nullptr);

	// register dispatch handler.
	virtual void register_default_handler(IN HandlerFunc hf) override;
	virtual void register_handler(IN uint64_t id, IN HandlerFunc hf) override;

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

	// release client.
	void close();

/////////////////////////////////////////////////////////////////// SERVICE MODE
public:
	// open session.
	TcpSession open_session();

	// service mode: set address ready to connect to service.
	void set_address(IN eco::net::AddressSet&);

	// service mode: async connect to service.
	void connect(IN eco::net::AddressSet&);
	void connect();

	// service mode. sync connect to service.
	void connect_wait(IN eco::net::AddressSet&, IN uint32_t millsec);
	void connect_wait(IN uint32_t millsec);

	// async send request to server.
	void authorize(IN TcpSession& session, IN MessageMeta& meta);

	// async send message.
	void send(IN eco::String& data, IN const uint32_t start);

	// async send message.
	void send(IN MessageMeta& meta);

	// req/rsp mode: send message and get a response.
	template<typename codec_t, typename req_t, typename rsp_t>
	inline eco::Result request(
			IN uint32_t req_type,
			IN req_t& req,
			IN rsp_t& rsp,
			IN bool encrypted = true)
	{
		codec_t codec_req(req);
		codec_t codec_rsp(rsp);
		MessageMeta meta_req(codec_req, none_session, req_type, encrypted);
		return request(meta_req, codec_rsp);
	}
	template<typename codec_t, typename msg_t>
	inline eco::Result request(
		IN uint32_t req_type,
		IN msg_t& msg,
		IN bool encrypted = true)
	{
		codec_t codec_rsp(msg);
		MessageMeta meta_req(codec_t(msg), none_session, req_type, encrypted);
		return request(meta_req, codec_rsp);
	}

public:
	// sync request data set.
	template<typename codec_t, typename req_t, typename rsp_t>
	inline eco::Result request_set(
		IN uint32_t req_type,
		IN req_t& req,
		IN AutoArray<rsp_t>& rsp_set,
		IN bool encrypted = true)
	{
		codec_t rsp_codec;
		codec_t req_codec(req);
		TypeCodec codec(rsp_codec, TypeCodec::make<rsp_t>);
		MessageMeta req_meta(req_codec, none_session, req_type, encrypted);
		return request(req_meta, codec, &rsp_set.get());
	}

#ifndef ECO_NO_PROTOBUF
	// sync request data set.
	template<typename codec_t, typename req_t, typename rsp_t>
	inline void request_proto_set(
		IN uint32_t req_type,
		IN req_t& req,
		IN std::vector<std::shared_ptr<rsp_t> >& rsp_set,
		IN bool encrypted = true)
	{
		AutoArray<rsp_t> set;
		auto result = request_set<codec_t>(req_type, req, set, encrypted);
		if (result != eco::ok)
		{
			EcoThrow(result) << "request_proto_set" <= get_name() <= req_type;
		}
		else if (rsp_set.size() == 1 && rsp_set[0]->has_error())
		{
			auto& e = rsp_set[0]->error();
			EcoThrow(e.id()) << "request_proto_set"
				<= get_name() <= req_type <= e.message();
		}

		rsp_set.reserve(set.size());
		for (size_t i = 0; i < set.size(); i++)
		{
			typedef std::shared_ptr<rsp_t> rsp_ptr;
			rsp_set.push_back(rsp_ptr(set.release(i)));
		}
	}

	// async send protobuf.
	inline void send(
		IN google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const uint32_t request_data = 0,
		IN const bool encrypted = true)
	{
		ProtobufCodec codec(msg);
		MessageMeta meta(codec, none_session, type, encrypted);
		// when request data is not send, it's value=0.
		if (request_data != 0)
			meta.set_request_data(request_data);
		send(meta);
	}

	// sync request protobuf.
	inline eco::Result request(
		IN const uint32_t req_type,
		IN google::protobuf::Message& req_msg,
		IN google::protobuf::Message& rsp_msg,
		IN const bool encrypted = true)
	{
		return request<ProtobufCodec>(req_type, req_msg, rsp_msg, encrypted);
	}

	// sync request protobuf.
	inline eco::Result request(
		IN const uint32_t req_type,
		IN google::protobuf::Message& msg,
		IN const bool encrypted = true)
	{
		return request<ProtobufCodec>(req_type, msg, encrypted);
	}
	
#endif

private:
	// sync request data or data set.
	eco::Result request(
		IN MessageMeta& req,
		IN Codec& rsp_codec,
		IN std::vector<void*>* rsp_set = nullptr);
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif