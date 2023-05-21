#pragma once
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
#include <eco/net/TcpOption.h>
#include <eco/net/TcpRequest.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpClient
{
	ECO_SHARED_API(TcpClient);
public:
	// get remote server ip
	eco::String ip();

	// get remote server port
	uint32_t port();

	// whether tcpclient is ready
	bool ready();

	// get tcp connect state
	const TcpState& state();

	// tcp client option
	TcpOptionClient& option();

public:
	// register connection event
	void set_event(OnConnect&&, OnDisconnect&&);

	// set receive callback event.
	void set_recv_event(OnRecvData&&, OnDecodeHead&&);

	// set load callback event when client connected.
	uint32_t set_load_event(OnLoadState&& on_load, bool app_ready_evt = true);
	// set load callback link events.
	uint32_t set_load_event(OnLoadEvent&& on_load, bool app_ready_evt = true);
	uint32_t add_load_event(OnLoadEvent&& on_load);
	// reload event, set event state none and call event.
	void load_event(uint32_t evt);
	// get event state that whether event finished.
	bool load_event_finished(uint32_t evt);

	// register dispatch handler.
	virtual void register_default(HandlerFunc&& hf) override;
	virtual void register_handler(int id, HandlerFunc&&) override;

public:
	// set timeout
	void set_timeout(uint32_t millsec);

	// set address ready to connect to service.
	void address_add(eco::net::Address& service);
	void address_set(eco::net::AddressSet& service);
	void address_set(const char* router, eco::net::AddressSet& router_addr);

	// sync connect to service.
	void connect(uint32_t millsec);
	void connect(eco::net::AddressSet&, uint32_t millsec);

	// async connect to service.
	void connect_async();
	void connect_async(eco::net::AddressSet&);
	// async connect to service of router. (cluster)
	void connect_async(const char* router, char* service);

	// close client: close socket, can be reconnect.
	void close();
	// release client: release thread/tcp_peer/io/timer.
	//void release();

public:
	// sync request a session
	inline eco::Result sync(int req_id, char* req, TcpSession& sess)
	{
		TcpRequestSync item(&TcpRequestSync::on_rsp1, &sess, 0);
		return sync(MessageMeta().type(req_id).data(req), item);
	}

	// sync request a response or a error
	template<typename TCodec, typename TRsp, typename TErr>
	inline eco::Result sync(int req_id, void* req, TRsp& rsp, TRsp& err)
	{
		auto* func = &TcpRequestSync::on_rsp2<TCodec, TRsp, TErr>;
		TcpRequestSync item(func, &rsp, &err);
		return sync(MessageMeta().type(req_id).codec(TCodec(req)), item);
	}

	// sync request a response array or a error
	template<typename TCodec, typename TRsp, typename TErr>
	inline eco::Result sync(int req_id, void* req,
						    std::vector<TRsp>& rsp, TErr& err)
	{
		auto* func = &TcpRequestSync::on_rsp3<TCodec, TRsp, TErr>;
		TcpRequestSync item(func, TRsp, TErr>, &rsp, &err);
		return sync(MessageMeta().type(req_id).codec(TCodec(req)), item);
	}

public:
	// async send message and callback.
	inline void async(
		uint32_t req_id, char* req,
		std::function<void(TcpSession&)>&& on_rsp)
	{
		auto* async1 = TcpRequestAsync::make<TcpRequestAsync1>(on_rsp);
		return async(MessageMeta().type(req_id).data(req), async1);
	}

	// async send message and callback.
	template<typename TCodec, typename TRsp, typename TErr>
	inline void async(
		uint32_t req_id, void* req,
		std::function<void(TRsp&, TErr&, bool)>&& on_rsp)
	{
		auto* async2 = TcpRequestAsync::make<TcpRequestAsync2>(on_rsp);
		return async(MessageMeta().type(req_id).codec(TCodec(req)), async2);
	}

	// async send message and array callback.
	template<typename TCodec, typename TRsp, typename TErr>
	inline void async_set(
		uint32_t req_id, void* req,
		std::function<void(TRsp&, TErr&, bool, bool)>&& on_rsp)
	{
		auto* async3 = TcpRequestAsync::make<TcpRequestAsync3>(on_rsp);
		return async(MessageMeta().type(req_id).codec(TCodec(req)), async3);
	}

	// async send message and array callback.
	template<typename TCodec, typename TRsp, typename TErr>
	inline void async_set(
		uint32_t req_id, void* req,
		std::function<void(std::vector<TRsp>&, TErr&, bool)>&& on_rsp)
	{
		auto* async4 = TcpRequestAsync::make<TcpRequestAsync4>(on_rsp);
		return async(MessageMeta().type(req_id).codec(TCodec(req)), async4);
	}

private:
	void async(MessageMeta& meta, TcpRequestAsync* call);
	eco::result sync(MessageMeta& meta, TcpRequestSync& item);
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif