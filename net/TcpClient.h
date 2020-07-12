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
#include <eco/net/TcpOption.h>
#include <eco/net/protocol/TcpProtocol.h>
#ifndef ECO_NO_PROTOBUF
#include <eco/proto/Proto.h>
#endif


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
// async request for: ResoveFunc/RejectFunc/ResponseFunc.
typedef std::function<void(eco::net::Context&)> RejectFunc;
typedef std::function<void(eco::net::Context&)> ResponseFunc;


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

	// protocol
	template<typename protocol_t>
	inline void add_protocol()
	{
		add_protocol(new protocol_t());
	}
	void add_protocol(IN Protocol*);

	// get latest protocol.
	Protocol* protocol_latest() const;
	Protocol* protocol(int version) const;

	// get connection id.
	ConnectionId get_id();

	// set connection data type.
	template<typename connection_data_t>
	inline void set_connection_data()
	{
		set_connection_data(&make_connection_data<connection_data_t>);
	}
	void set_connection_data(IN MakeConnectionDataFunc make);

	// connection data.
	template<typename connection_data_t>
	inline connection_data_t& cast()
	{
		return *static_cast<connection_data_t*>(data());
	}
	ConnectionData* data();

	// connection data state.
	eco::atomic::State& data_state();

	// get tcp peer state. like whether connected.
	const TcpState& get_state() const;

	// set session data class and tcp session mode.
	template<typename session_data_t>
	inline void set_session_data()
	{
		set_session_data(&make_session_data<session_data_t>);
	}
	void set_session_data(IN MakeSessionDataFunc make);

	// register connection event
	void set_event(IN ClientOpenFunc on_open, IN ClientCloseFunc on_close);

	// set receive callback event.
	void set_recv_event(OnRecvDataFunc on_recv, OnDecodeHeadFunc on_decode);

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
	void add_address(IN eco::net::Address&);
	void set_address(IN eco::net::AddressSet&);

	// service mode: async connect to service.
	void async_connect(IN bool reconnect = false);
	void async_connect(IN eco::net::AddressSet&, IN bool reconnect = false);

	// service mode. sync connect to service.
	void connect(IN uint32_t millsec);
	void connect(IN eco::net::AddressSet&, IN uint32_t millsec);

	// async send request to server.
	void authorize(IN TcpSession& session, IN MessageMeta& meta);

	// async send message.
	void send(IN eco::String& data, IN const uint32_t start);

	// async send message.
	void send(IN MessageMeta&);

	// async send message with no message object.
	inline void send(IN uint32_t type, IN uint32_t req_id)
	{
		MessageMeta meta;
		meta.message_type(type).set_request_data(req_id);
		send(meta);
	}

/////////////////////////////////////////////////////////////////// SYNC REQUEST
public:
	// req/rsp mode: send message and get a response.
	inline eco::Result request(IN uint32_t req_type)
	{
		MessageMeta meta;
		meta.message_type(req_type);
		return request(meta, nullptr, nullptr, nullptr);
	}

	// req/rsp mode: send message and get a response.
	template<typename codec_t, typename req_t, typename rsp_t>
	inline eco::Result request(
		IN uint32_t req_type, IN const req_t& req, IN rsp_t& rsp)
	{
		codec_t req_codec(req);
		codec_t rsp_codec(rsp);
		MessageMeta meta;
		meta.codec(req_codec).message_type(req_type);
		return request(meta, nullptr, &rsp_codec, nullptr);
	}

	// sync request data set.
	template<typename codec_t, typename req_t, typename rsp_t>
	inline eco::Result request_set(
		IN uint32_t req_type,
		IN const req_t& req,
		IN AutoArray<rsp_t>& rsp_set)
	{
		codec_t rsp_codec;
		codec_t req_codec(req);
		MessageMeta req_meta;
		req_meta.codec(req_codec).message_type(req_type);
		WrapCodec rsp_wrap_codec(rsp_codec, WrapCodec::make<rsp_t>);
		return request(req_meta, nullptr, &rsp_wrap_codec, &rsp_set.get());
	}

	// sync request data set.
	template<typename codec_t, typename req_t, typename err_t, typename rsp_t>
	inline eco::Result request_set(
		IN uint32_t req_type,
		IN const req_t& req,
		IN const err_t& err,
		IN AutoArray<rsp_t>& rsp_set)
	{
		codec_t rsp_codec;
		codec_t req_codec(req);
		codec_t err_codec(err);
		MessageMeta req_meta;
		req_meta.codec(req_codec).message_type(req_type);
		WrapCodec rsp_wrap_codec(rsp_codec, WrapCodec::make<rsp_t>);
		return request(req_meta, &err_codec, &rsp_wrap_codec, &rsp_set.get());
	}

	// req/rsp mode: async send message and callback.
	template<typename codec_t, typename rsp_t, typename req_t>
	inline void async(
		IN uint32_t req_type,
		IN req_t& req,
		IN RejectFunc&& reject,
		IN std::function<void(rsp_t&, eco::net::Context&)>&& resolve)
	{
		async_ref<codec_t, rsp_t>(req_type, req, reject, resolve);
	}

#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void send(
		IN MessageOption& opt,
		IN const google::protobuf::Message& msg)
	{
		ProtobufCodec codec(msg);
		send(MessageMeta(&codec, opt));
	}

	// async send protobuf.
	inline void send(
		IN uint32_t type, 
		IN const google::protobuf::Message& msg)
	{
		ProtobufCodec codec(msg);
		send(MessageMeta(&codec, MessageOption(type)));
	}

	// sync request protobuf.
	inline eco::Result request(
		IN uint32_t req_type,
		IN const google::protobuf::Message& req_msg,
		IN const google::protobuf::Message& rsp_msg)
	{
		return request<ProtobufCodec>(req_type, req_msg, rsp_msg);
	}

	/*@sync request data ptr set.
	* @rsp_t: response object type.
	* @rsp_ptr_t: std::shared_ptr<rsp_t>/ std::auto_ptr<rsp_t> /rsp_t* /void*.
	* @rsp_ptr_set_t: std::vector/list/queue so on.
	*/
	template<typename rsp_t, typename rsp_ptr_set_t>
	inline eco::Result request_set(
		IN uint32_t req_type,
		IN const google::protobuf::Message& req,
		IN rsp_ptr_set_t& rsp_set)
	{
		AutoArray<rsp_t> set;
		eco::Result ec = request_set<ProtobufCodec, 
			google::protobuf::Message>(req_type, req, set);
		if (ec == eco::ok)
		{
			rsp_set.reserve(set.size());
			for (size_t i = 0; i < set.size(); i++)
			{
				rsp_set.push_back(rsp_ptr_set_t::value_type(set.release(i)));
			}
		}
		return ec;
	}

	// sync request data set.
	template<typename rsp_t, typename rsp_ptr_set_t>
	inline eco::Result request_set(
		IN uint32_t req_type,
		IN const google::protobuf::Message& req,
		IN const google::protobuf::Message& err,
		IN rsp_ptr_set_t& rsp_set)
	{
		AutoArray<rsp_t> set;
		auto result = request_set<ProtobufCodec>(req_type, req, err, set);
		eco::Result ec = request_set<ProtobufCodec,
			google::protobuf::Message>(req_type, req, err, set);
		if (result == eco::ok)
		{
			rsp_set.reserve(set.size());
			for (size_t i = 0; i < set.size(); i++)
			{
				rsp_set.push_back(rsp_ptr(set.release(i)));
			}
		}
		return result;
	}

	// sync request data set.
	template<typename rsp_t>
	inline void async(
		IN uint32_t req_type,
		IN const google::protobuf::Message& req,
		IN RejectFunc&& reject,
		IN std::function<void(rsp_t&, eco::net::Context&)>&& resolve)
	{
		typedef std::function<void(rsp_t&, eco::net::Context&)> Func;
		async_ref<ProtobufCodec, rsp_t>(req_type, req, 
			std::forward<RejectFunc>(reject), std::forward<Func>(resolve));
	}

	// sync request data set.
	template<typename rsp_t, typename rsp_ptr_set_t>
	inline void async_set(
		IN uint32_t req_type,
		IN const google::protobuf::Message& req,
		IN RejectFunc&& reject,
		IN std::function<void(std::shared_ptr<rsp_ptr_set_t>&)>&& resolve)
	{
		typedef std::function<void(std::shared_ptr<rsp_ptr_set_t>&)> Func;
		async_set_ref<ProtobufCodec, rsp_t, rsp_ptr_set_t>(req_type, req, 
			std::forward<RejectFunc>(reject), std::forward<Func>(resolve));
	}
#endif

private:
	/*@sync request data or data set. success when return eco::ok, esle fail.
	if server reject request and response with a error object, "err_codec" will
	decode the error object message.
	* @err_codec: reject error message decode.
	* @rsp_codec: resolve response message decode.
	* @rsp_set: save object set when response object set.
	*/
	eco::Result request(
		IN MessageMeta& req,
		IN Codec* err_codec,
		IN Codec* rsp_codec,
		IN std::vector<void*>* rsp_set);

	// req/rsp mode: send async message.
	void async(IN MessageMeta& req, IN ResponseFunc& rsp_func);

	// req/rsp mode: async send message and callback.
	template<typename codec_t, typename rsp_t, typename req_t>
	inline void async_ref(
		IN uint32_t req_type,
		IN const req_t& req,
		IN RejectFunc&& reject,
		IN std::function<void(rsp_t&, eco::net::Context&)>&& resolve)
	{
		ResponseFunc rsp_func = [=](eco::net::Context& c) mutable {
			if (c.has_error())
			{
				reject(c);
				return;
			}
			rsp_t rsp;
			codec_t rsp_codec;
			rsp_codec.set_message(handle_context_make(rsp));
			if (!rsp_codec.decode(c.m_message.m_data, c.m_message.m_size))
			{
				c.m_meta.error_id(eco::error);
				reject(c);
				return;
			}
			resolve(rsp, c);
		};

		codec_t req_codec(req);
		MessageMeta req_meta;
		req_meta.codec(req_codec).message_type(req_type);
		return async(req_meta, rsp_func);
	}

	// req/rsp mode: async send message and callback.
	template<
		typename codec_t, 
		typename rsp_t, 
		typename rsp_ptr_set_t, 
		typename req_t>
	inline void async_set_ref(
		IN uint32_t req_type,
		IN const req_t& req,
		IN RejectFunc& reject,
		IN std::function<void(std::shared_ptr<rsp_ptr_set_t>&)>&& resolve)
	{
		std::shared_ptr<rsp_ptr_set_t> rsp_set(new rsp_ptr_set_t());
		ResponseFunc rsp_func = [=](eco::net::Context& c) mutable {
			if (c.has_error())
			{
				reject(c);
				return;
			}
			rsp_t* obj = nullptr;
			typename rsp_ptr_set_t::value_type rsp(obj = new rsp_t());
			if (!codec_t(*obj).decode(c.m_message.m_data, c.m_message.m_size))
			{
				c.m_meta.error_id(eco::error);
				reject(c);
				return;
			}
			rsp_set->push_back(rsp);	// add data to response array.
			if (c.is_last()) resolve(rsp_set);
		};

		codec_t req_codec(req);
		MessageMeta req_meta;
		req_meta.codec(req_codec).message_type(req_type);
		return async(req_meta, rsp_func);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif