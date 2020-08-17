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
#include <eco/net/protocol/TcpProtocol2.h>
#ifndef ECO_NO_PROTOBUF
#include <eco/proto/Proto.h>
#endif


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
	inline const char* name() const
	{
		return get_option().get_name();
	}

	// service module name.
	inline const char* module_() const
	{
		return get_option().get_module_();
	}

	// whether tcpclient is ready.
	inline bool ready() const;

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
	const TcpState& state() const;

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

	// set load callback event when client connected.
	void set_load_event(OnLoadDataFunc on_load_data);

	// register dispatch handler.
	virtual void register_default(IN HandlerFunc hf) override;
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
	bool connect(IN uint32_t millsec);
	bool connect(IN eco::net::AddressSet&, IN uint32_t millsec);

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

	/*@sync request data or data set. success when return eco::ok, esle fail.
	if server reject request and response with a error object, "err_codec" will
	decode the error object message.
	* @err_codec: reject error message decode.
	* @rsp_codec: resolve response message decode.
	* @rsp_set: save object set when response object set.
	*/
	bool request(
		IN MessageMeta& req,
		IN Codec& err_codec,
		IN Codec& rsp_codec,
		IN std::vector<void*>* rsp_set = nullptr);

	// req/rsp mode: send async message.
	typedef std::function<void(eco::net::Context&)> ResponseFunc;
	void async(IN MessageMeta& req, IN ResponseFunc& rsp_func);

/////////////////////////////////////////////////////////////////// SYNC REQUEST
public:
	// req/rsp mode: send message and get a response.
	template<typename codec_t>
	inline bool request(
		IN uint32_t req_type,
		IN const void* req = nullptr,
		OUT void* err = nullptr,
		OUT void* rsp = nullptr)
	{
		codec_t req_codec(req);
		codec_t err_codec(err);
		codec_t rsp_codec(rsp);
		MessageMeta meta;
		meta.codec(req_codec).message_type(req_type);
		return request(meta, err_codec, rsp_codec, nullptr);
	}

	// sync request data set.
	template<typename codec_t, typename rsp_t>
	inline bool request_set(
		IN uint32_t req_type,
		IN const void* req,
		OUT void* err,
		IN AutoArray<rsp_t>& rsp_set)
	{
		codec_t req_codec(req);
		codec_t err_codec(err);
		codec_t rsp_codec;
		WrapCodec wrap_codec(rsp_codec, WrapCodec::make<rsp_t>);
		MessageMeta meta;
		meta.codec(req_codec).message_type(req_type);
		return request(meta, err_codec, wrap_codec, &rsp_set.get());
	}

	// req/rsp mode: async send message and callback.
	template<typename codec_t, typename err_t>
	inline void async(
		IN uint32_t req_type, IN const void* req,
		IN std::function<void(err_t&, eco::net::Context&)>&& reject,
		IN std::function<void(eco::net::Context&)>&& resolve);

	// req/rsp mode: async send message and callback.
	template<typename codec_t, typename err_t, typename rsp_t>
	inline void async(
		IN uint32_t req_type, IN const void* req,
		IN std::function<void(err_t&, eco::net::Context&)>&& reject,
		IN std::function<void(rsp_t&, eco::net::Context&)>&& resolve);

	// req/rsp mode: async send message and callback.
	template<typename codec_t, typename err_t, typename rsp_set_t>
	inline void async_set(
		IN uint32_t req_type, IN const void* req,
		IN std::function<void(err_t&, eco::net::Context&)>&& reject,
		IN std::function<void(std::shared_ptr<rsp_set_t>&)>&& resolve);

#ifndef ECO_NO_PROTOBUF
	// sync request protobuf.
	inline bool request(
		IN uint32_t req_type,
		IN const google::protobuf::Message* req,
		OUT google::protobuf::Message* err,
		OUT google::protobuf::Message* rsp)
	{
		return request<ProtobufCodec>(req_type, req, err, rsp);
	}

	/*@sync request data ptr set.
	* @rsp_t: response object type.
	* @rsp_ptr_t: std::shared_ptr<rsp_t>/ std::auto_ptr<rsp_t> /rsp_t* /void*.
	* @rsp_ptr_set_t: std::vector/list/queue so on.
	*/
	template<typename rsp_ptr>
	inline bool request_set(
		IN uint32_t req_type,
		IN const google::protobuf::Message* req,
		OUT google::protobuf::Message* err,
		OUT std::vector<rsp_ptr>& rsp_set)
	{
		AutoArray<rsp_ptr::element_type> set;
		if (request_set<ProtobufCodec, rsp_ptr::element_type>(
			req_type, req, err, set))
		{
			rsp_set.reserve(set.size());
			for (size_t i = 0; i < set.size(); i++)
			{
				rsp_set.push_back(rsp_ptr(set.release(i)));
			}
			return true;
		}
		return false;
	}

	// sync request data set.
	template<typename err_t>
	inline void async(
		IN uint32_t req_type,
		IN const google::protobuf::Message* req,
		IN std::function<void(err_t&, eco::net::Context&)>&& reject,
		IN std::function<void(eco::net::Context&)>&& resolve)
	{
		async<ProtobufCodec, err_t>(
			req_type, req, std::move(reject), std::move(resolve));
	}
	template<typename err_t, typename rsp_t>
	inline void async(
		IN uint32_t req_type,
		IN const google::protobuf::Message* req,
		IN std::function<void(err_t&, eco::net::Context&)>&& reject,
		IN std::function<void(rsp_t&, eco::net::Context&)>&& resolve)
	{
		async<ProtobufCodec, err_t, rsp_t>(
			req_type, req, std::move(reject), std::move(resolve));
	}
	
	// sync request data set.
	template<typename err_t, typename rsp_set_t>
	inline void async_set(
		IN uint32_t req_type,
		IN const google::protobuf::Message* req,
		IN std::function<void(err_t&, eco::net::Context&)>&& reject,
		IN std::function<void(std::shared_ptr<rsp_set_t>&)>&& resolve)
	{
		async_set<ProtobufCodec, err_t, rsp_set_t>(
			req_type, req, std::move(reject), std::move(resolve));
	}
#endif

private:
	template<typename codec_t, typename err_t>
	static inline void call_reject(
		IN const char* name_, IN eco::net::Context& c,
		IN std::function<void(err_t&, eco::net::Context&)>& reject);

	template<typename codec_t, typename err_t, typename rsp_t>
	static inline bool call_resolve(
		IN const char* name_, IN eco::net::Context& c, OUT rsp_t& rsp,
		IN std::function<void(err_t&, eco::net::Context&)>& reject);

	template<typename codec_t, typename err_t>
	static inline ResponseFunc context_func(
		std::function<void(err_t&, eco::net::Context&)>& reject,
		std::function<void(eco::net::Context&)>& resolve);

	template<typename codec_t, typename err_t, typename rsp_t>
	static inline ResponseFunc context_func(
		std::function<void(err_t&, eco::net::Context&)>& reject,
		std::function<void(rsp_t&, eco::net::Context&)>& resolve);

	template<typename codec_t, typename err_t, typename rsp_set_t>
	static inline ResponseFunc context_func(
		std::shared_ptr<rsp_set_t>& rsp_set,
		std::function<void(err_t&, eco::net::Context&)>& reject,
		std::function<void(std::shared_ptr<rsp_set_t>&)>& resolve);
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#include "TcpClient.inl"
#endif