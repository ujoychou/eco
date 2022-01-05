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
#include <eco/net/protocol/ProtocolFamily.h>
#include "DispatchServer.h"


ECO_NS_BEGIN(eco);
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpPeer::Impl : public TcpConnectorHandler
{
	ECO_IMPL_INIT(TcpPeer);
public:
	
	TcpState			m_state;			// peer socket state.
	TcpPeer::wptr		m_peer_observer;	// this peer.
	TcpPeerHandler*		m_handler;			// peer handler.
	TcpConnector		m_connector;		// socket io.
	MessageWorker*		m_server;			// message server post.
	SessionId			m_id;				// peer session id, uuid.
	Protocol*			m_protocol;			// current protocol.
	eco::String			m_user;				// user name.
	eco::String			m_lang;				// lang name.
	std::auto_ptr<ConnectionData> m_data;	// connection data.
	TimingWheel::Timer m_timer_recv;		// tcp peer recv heartbeat timer.
	TimingWheel::Timer m_timer_send;		// tcp peer send heartbeat timer.

public:
	// never be called, this is just for complie success.
	inline Impl() : m_connector(0)
	{
		m_id = 0;
		m_server = nullptr;
		m_handler = nullptr;
		m_protocol = nullptr;
		assert(false);
	}

	inline Impl(
		IN IoWorker* io_server,
		IN MessageWorker* msg_server,
		IN TcpPeerHandler* hdl)
		: m_handler(hdl)
		, m_connector(io_server)
		, m_server(msg_server)
		, m_id(0), m_protocol(0)
	{}

	inline ~Impl()
	{
		release();
	}

	inline void release()
	{
		m_data.reset();
		m_peer_observer.reset();
		m_state.set_closed();
		m_user.clear();
		m_lang.clear();
		m_connector.release();
		if (m_server)
		{
			m_server->detach();
			m_server = nullptr;
		}
	}

	// set protocol.
	inline void set_protocol(IN Protocol* prot)
	{
		if (m_protocol != prot) m_protocol = prot;
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

	void make_connection_data(const MakeConnectionData& make_func);

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
		IN  const uint32_t size);

	inline void init_option(const TcpOption& opt)
	{
		try
		{
			m_connector.set_no_delay(opt.no_delay());
			m_connector.set_send_capacity(opt.send_capacity());
			m_connector.set_send_buffer_size(opt.send_buffer_size());
			m_connector.set_recv_buffer_size(opt.recv_buffer_size());
			//m_connector.set_send_low_watermark(opt.send_low_watermark());
			//m_connector.set_recv_low_watermark(opt.recv_low_watermark());
		}
		catch (std::exception& e)
		{
			ECO_FUNC(error) < id() <= e.what();
		}
	}

	// raw io socket related by the net framework: exp boost::asio\libevent.
	inline TcpSocket* socket()
	{
		return m_connector.socket();
	}

	// tcp peer identity which is the address of connector.
	inline SessionId id() const
	{
		return m_id;
	}

	inline eco::String ip() const
	{
		return m_connector.ip();
	}

	inline uint32_t port() const
	{
		return m_connector.port();
	}

	// tcp peer connection state.
	inline TcpState& get_state()
	{
		return m_state;
	}
	inline const TcpState& state() const
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
	inline bool async_connect(IN const Address& addr)
	{
		return m_connector.async_connect(addr);
	}

	// async recv.
	inline void async_recv();

	// ready for receiving data.
	inline void async_recv_by_client()
	{
		m_state.set_ready();
		m_handler->m_on_connect(false);
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

	inline void send(IN const MessageMeta& meta)
	{
		eco::String data;
		uint32_t start = 0;
		if (!m_protocol->encode(data, start, meta))
		{
			ECO_ERROR << NetLog(id(), __func__,	meta.m_session_id)
				<= eco::this_thread::error();
			return;
		}
		m_state.set_self_live(true);
		send(data, start);
	}

	// send heartbeat.
	inline void send_heartbeat()
	{
		if (m_state.ready())
		{
			eco::String data;
			m_protocol->encode_heartbeat(data);
			// don't call send(data, start), because of it will
			// set "state.set_self_live(true);"
			if (m_state.connected())
			{
				m_connector.async_write(data, 0);
			}
		}
	}
	// send live heartbeat.
	inline void send_live_heartbeat()
	{
		if (m_state.ready())
		{
			// during send tick, if connection itself send a message.
			// indicated it is alive, and no need to send heartbeat.
			if (get_state().self_live())
				get_state().set_self_live(false);
			else
				send_heartbeat();
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

			// 4.cancel timer.
			m_timer_recv.cancel();
			m_timer_send.cancel();
			ECO_FUNC(debug) < id();
			return true;
		}
		return false;
	}

	// close peer and notify peer handler.
	inline bool close_and_notify()
	{
		if (!close()) return false;
		m_handler->m_on_close(this);
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
	void on_accept_state(uint64_t id);
	void on_accept(const TcpOption& opt);

	// when peer has connected to server.(tcp client)
	virtual void on_connect(
		IN bool err) override;

	// when the peer has received data.
	virtual void on_read(
		IN MessageHead& head,
		IN eco::String& data, bool err) override;

	// the peer has send data.
	virtual void on_write(
		IN uint32_t size, bool err) override;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif