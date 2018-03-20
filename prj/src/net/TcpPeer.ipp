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
#include <eco/Project.h>
#include <eco/net/TcpPeer.h>
#include <eco/net/TcpConnector.h>
#include <eco/net/Context.h>
#include <eco/net/Log.h>


namespace eco{;
namespace net{;



////////////////////////////////////////////////////////////////////////////////
class TcpPeer::Impl : public TcpConnectorHandler
{
	ECO_IMPL_INIT(TcpPeer);
public:
	TcpState m_state;
	TcpPeer::wptr m_peer_observer;
	TcpPeerHandler* m_handler;
	TcpConnector m_connector;
	std::auto_ptr<ConnectionData> m_data;
	// the session of tcp peer.
	//std::vector<uint32_t> m_session_id;
	//eco::Mutex m_session_id_mutex;

public:
	// never be called, this is just for complie success.
	inline Impl() : m_handler(nullptr), m_connector(nullptr)
	{
		assert(false);
	}

	inline Impl(IN IoService* io, IN TcpPeerHandler* hdl)
		: m_handler(hdl), m_connector(io)
	{}

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

	inline TcpPeerHandler& handler()
	{
		return *m_handler;
	}
	inline ProtocolHead& protocol_head()
	{
		return m_handler->protocol_head();
	}
	inline uint32_t head_size() const
	{
		return m_handler->protocol_head().size();
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

	inline const eco::String get_ip() const
	{
		return (eco::String&&)m_connector.get_ip();
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
	inline void set_connected()
	{
		m_state.set_connected();
		m_state.set_peer_live(true);
	}

	// ready for receiving data.
	inline void async_recv_by_client()
	{
		m_state.set_ready();
		m_handler->on_connect();
		async_recv();
	}

	// ready to receive data head.
	inline void async_connect(IN const Address& addr)
	{
		m_connector.async_connect(addr);
	}
	inline void async_recv();
	inline void async_recv_by_server()
	{
		m_state.set_ready();
		async_recv();
	}

	// send websocket shakehand.
	inline void send_websocket_shakehand();

	// handle websocket shakehand.
	inline void async_recv_shakehand();
	inline void handle_websocket_shakehand_req(
		IN const char* data_head,
		IN const uint32_t head_size);
	inline void handle_websocket_shakehand_rsp(
		IN const char* data_head,
		IN const uint32_t head_size);

	// send response to client.
	inline void async_send(
		IN eco::String& data, 
		IN const uint32_t start)
	{
		m_state.set_self_live(true);
		m_connector.async_write(data, start);
	}

	inline void async_send(IN const MessageMeta& meta, IN Protocol& prot)
	{
		eco::Error e;
		eco::String data;
		uint32_t start = 0;
		if (!prot.encode(data, start, meta, e))
		{
			EcoError << NetLog(get_id(), ECO_FUNC, meta.m_session_id) <= e;
			return;
		}
		async_send(data, start);
	}

	// send heartbeat.
	inline void async_send_heartbeat(IN ProtocolHead& prot_head)
	{
		if (!m_state.ready())
			return;
		eco::String data;
		prot_head.encode_heartbeat(data);
		async_send(data, 0);
	}
	// send live heartbeat.
	inline void async_send_live_heartbeat(IN ProtocolHead& prot_head)
	{
		if (!m_state.ready())
			return;
		// during send tick, if connection itself send a message.
		// indicated it is alive, and no need to send heartbeat.
		if (get_state().self_live())
			state().set_self_live(false);
		else
			async_send_heartbeat(prot_head);
	}

	// close peer.
	inline void close()
	{
		if (!m_state.closed())
		{
			// 1.close state.
			m_state.set_closed();

			// 2.close socket.
			m_connector.close();
		}
	}

	// close peer and notify peer handler.
	inline void close_and_notify(IN const eco::Error* e)
	{
		close();
		m_handler->on_close(get_id());
	}

	// get data context that to be handled by dispatch server.
	inline void get_data_context(
		OUT DataContext& dc,
		IN  MessageCategory category,
		IN  eco::String& data,
		IN  Protocol* prot)
	{
		dc.m_category = category;
		dc.m_data = std::move(data);
		dc.m_peer_wptr = m_peer_observer;
		dc.m_prot = prot;
	}

public:
	// when peer has connected to server.(tcp client)
	virtual void on_connect(
		IN bool is_connected,
		IN const eco::Error* error) override;

	// when the peer has received data head.
	virtual void on_read_head(
		IN char* data,
		IN const uint32_t head_size,
		IN const eco::Error* error) override;

	// when the peer has received data.
	virtual void on_read_data(
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