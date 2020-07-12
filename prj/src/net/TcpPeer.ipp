#ifndef ECO_NET_PEER_IPP
#define ECO_NET_PEER_IPP
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/net/TcpPeer.h>
#include <eco/net/TcpConnector.h>
#include <eco/net/Context.h>
#include <eco/net/Log.h>
#include <eco/net/protocol/ProtocolFamily.h>
#include "DispatchServer.h"


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpPeer::Impl : public TcpConnectorHandler
{
	ECO_IMPL_INIT(TcpPeer);
public:
	TcpState m_state;
	eco::atomic::State m_data_state;
	TcpPeer::wptr m_peer_observer;
	TcpPeerHandler* m_handler;
	TcpConnector m_connector;
	std::auto_ptr<ConnectionData> m_data;
	MessageWorker* m_server;

public:
	// never be called, this is just for complie success.
	inline Impl() : m_handler(0), m_connector(0), m_server(0)
	{
		assert(false);
	}

	inline Impl(
		IN IoWorker* io_server,
		IN MessageWorker* msg_server,
		IN TcpPeerHandler* hdl)
		: m_handler(hdl)
		, m_connector(io_server)
		, m_server(msg_server)
	{}

	inline ~Impl()
	{
		release();
	}

	// ready to receive data head.
	inline void restart(IN MessageWorker* worker, IN TcpPeer::ptr& peer)
	{
		m_server = worker;
		prepare(peer);
	}

	inline void release()
	{
		m_data.reset();
		m_peer_observer.reset();
		m_state.set_closed();
		m_data_state.none();
		m_connector.release();
		if (m_server)
		{
			m_server->detach();
			m_server = nullptr;
		}
	}

	// peer must be created in the heap(by new).
	inline void prepare(IN TcpPeer::ptr& peer)
	{
		m_peer_observer = peer;

		// init io service and set tcp server.
		m_connector.register_handler(*this, m_peer_observer);

		// init state: prepare for first heartbeat.
		m_state.set_self_live(false);
	}

	void make_connection_data(
		IN MakeConnectionDataFunc make_func,
		IN Protocol* prot);

	// handler objects.
	inline TcpPeerHandler& handler()
	{
		return *m_handler;
	}
	inline const ProtocolFamily& protocol() const
	{
		return *m_handler->m_protocol;
	}
	inline const TcpOption& option() const
	{
		return *m_handler->m_option;
	}
	inline void* owner() const
	{
		return m_handler->m_owner;
	}

	// check tcp message is finished.
	eco::Result on_decode_head(
		OUT MessageHead& head, 
		IN  const char* buff, 
		IN  const uint32_t size,
		OUT eco::Error& err);

	inline void init_option(const TcpOption& opt)
	{
		m_connector.set_no_delay(opt.no_delay());
		m_connector.set_send_capacity(opt.get_send_capacity());
		m_connector.set_send_buffer_size(opt.get_send_buffer_size());
		m_connector.set_recv_buffer_size(opt.get_recv_buffer_size());
		//m_connector.set_send_low_watermark(opt.get_send_low_watermark());
		//m_connector.set_recv_low_watermark(opt.get_recv_low_watermark());
	}

	// raw io socket related by the net framework: exp boost::asio\libevent.
	inline TcpSocket* socket()
	{
		return m_connector.socket();
	}

	// tcp peer identity which is the address of connector.
	inline size_t get_id() const
	{
		return m_connector.get_id();
	}

	inline eco::String get_ip() const
	{
		return m_connector.get_ip();
	}

	inline uint32_t get_port() const
	{
		return m_connector.get_port();
	}

	// tcp peer connection state.
	inline TcpState& state()
	{
		return m_state;
	}
	inline const TcpState& get_state() const
	{
		return m_state;
	}
	// set tcp peer state to connected.
	inline bool connected() const
	{
		return m_state.connected();
	}

	// peer isn't live/connected when on_accepted
	// but sended valid data or heartbeat.
	inline bool check_peer_live()
	{
		if (!m_state.peer_live())
		{
			return false;
		}
		m_state.set_peer_live(false);
		return true;
	}

	// ready to receive data head.
	inline void async_connect(IN const Address& addr)
	{
		m_connector.async_connect(addr);
	}

	// async recv.
	inline void async_recv();

	// ready for receiving data.
	inline void async_recv_by_client()
	{
		m_state.set_ready();
		m_handler->m_on_connect(nullptr);
		async_recv();
	}
	
	// ready for receiving data.
	inline void async_recv_by_server()
	{
		m_state.set_ready();
		async_recv();
	}

	// handle websocket shakehand.
	inline void async_recv_shakehand();				// #0
	inline void send_websocket_shakehand();			// #1
	inline void handle_websocket_shakehand_req(		// #2
		IN const char* data_head,
		IN const uint32_t head_size);
	inline void handle_websocket_shakehand_rsp(		// #3
		IN const char* data_head,
		IN const uint32_t head_size);

	// send response to client.
	inline void send(IN eco::String& data, IN uint32_t start)
	{
		if (m_state.connected())
		{
			m_state.set_self_live(true);
			m_connector.async_write(data, start);
		}
	}

	inline void send(IN const MessageMeta& meta, IN Protocol& prot)
	{
		eco::Error e;
		eco::String data;
		uint32_t start = 0;
		if (!prot.encode(data, start, meta, e))
		{
			ECO_ERROR << NetLog(get_id(), __func__,
				meta.m_session_id) <= e;
			return;
		}
		send(data, start);
	}

	// send heartbeat.
	inline void send_heartbeat(IN const Protocol& prot)
	{
		if (m_state.ready())
		{
			eco::String data;
			prot.encode_heartbeat(data);
			send(data, 0);
		}
	}
	// send live heartbeat.
	inline void send_live_heartbeat(IN Protocol& prot)
	{
		if (m_state.ready())
		{
			// during send tick, if connection itself send a message.
			// indicated it is alive, and no need to send heartbeat.
			if (get_state().self_live())
				state().set_self_live(false);
			else
				send_heartbeat(prot);
		}
	}

	// close peer.
	inline bool close()
	{
		if (!m_state.closed())
		{
			// 1.close state.
			m_state.set_closed();

			// 2.close socket.
			m_connector.close();

			// 3.clear data by the release function in acceptor thread.
			// this is for protect user connection data safe release.
			// release();
			return true;
		}
		return false;
	}

	// close peer and notify peer handler.
	inline bool close_and_notify(const eco::Error& e, IN bool erase_peer)
	{
		if (!close()) return false;
		m_handler->m_on_close(get_id(), e, erase_peer);
		return true;
	}

	// get data context that to be handled by dispatch server.
	inline void get_data_context(
		OUT DataContext& dc,
		IN  MessageHead& head,
		IN  eco::String& data)
	{
		dc.m_category = head.m_category;
		dc.m_head_size = head.m_head_size;
		dc.m_data = std::move(data);
		dc.m_peer_wptr = m_peer_observer;
		dc.m_protocol = head.m_protocol;
	}

	// tcp server: recv request.
	inline void post(IN MessageHead& head, IN eco::String& data)
	{
		eco::net::DataContext dc;
		get_data_context(dc, head, data);
		m_server->queue().post(dc);
	}

public:
	// when accept this peer.
	void on_accept_state();
	void on_accept(const TcpOption& opt);

	// when peer has connected to server.(tcp client)
	virtual void on_connect(
		IN bool is_connected,
		IN const eco::Error* error) override;

	// when the peer has received data.
	virtual void on_read(
		IN MessageHead& head,
		IN eco::String& data,
		IN const eco::Error* error) override;

	// the peer has send data.
	virtual void on_write(
		IN const uint32_t write_size,
		IN const eco::Error* error) override;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif