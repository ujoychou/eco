#include "PrecHeader.h"
#include "TcpPeer.ipp"
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/Ecode.h>
#include <eco/log/Log.h>
#include <eco/net/protocol/StringCodec.h>
#include <eco/net/protocol/TcpProtocol.h>
#include <eco/net/protocol/WebSocketShakeHand.h>
#include <eco/net/protocol/WebSocketProtocol.h>
#include "TcpOuter.h"



namespace eco{;
namespace net{;
ECO_OBJECT_IMPL(TcpPeer);
/* this thread local data has two serious error:
1.now is: acceptor thread recv data, but not io thread.
2.each connection should has a data_head instead of each thread.
*/
//EcoThreadLocal char s_data_head[32] = { 0 };
////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::make_connection_data(
	IN MakeConnectionDataFunc make_func, IN Protocol* prot)
{
	if (make_func != nullptr && m_data.get() == nullptr)
	{
		m_data.reset(make_func());
		TcpConnectionOuter conn(m_data->connection());
		conn.set_id(get_id());
		conn.set_peer(m_peer_observer);
		if (prot != nullptr) conn.set_protocol(*prot);
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void TcpPeer::Impl::async_recv()
{
	m_connector.async_read_head(head_size());
}
void TcpPeer::Impl::async_recv_shakehand()
{
	m_state.set_websocket();
	m_connector.async_read_until(
		WebSocketShakeHand::size(), WebSocketShakeHand::head_end());
}


////////////////////////////////////////////////////////////////////////////////
inline void TcpPeer::Impl::send_websocket_shakehand()
{
	assert(m_state.websocket());
	WebSocketShakeHand shake_hand;
	async_send(shake_hand.format(), 0);
}


////////////////////////////////////////////////////////////////////////////////
inline void TcpPeer::Impl::handle_websocket_shakehand_req(
	IN const char* data_head, IN const uint32_t head_size)
{
	assert(m_state.websocket());
	if (eco::find(data_head, head_size, "GET ") != data_head)
	{
		ECO_INFO << NetLog(get_id(), __func__)
			<= "web socket shakehand invalid 'Get '.";
		return;
	}

	WebSocketShakeHand shake_hand;
	if (!shake_hand.parse_req(data_head, head_size))
	{
		ECO_INFO << NetLog(get_id(), __func__)
			<= "web socket shakehand invalid.";
		close_and_notify();
		return;
	}
	async_send(shake_hand.response(), 0);
	async_recv_by_server();
}


////////////////////////////////////////////////////////////////////////////////
inline void TcpPeer::Impl::handle_websocket_shakehand_rsp(
	IN const char* data_head, IN const uint32_t head_size)
{
	assert(m_state.websocket());
	if (eco::find(data_head, head_size, "HTTP") != data_head)
	{
		ECO_INFO << NetLog(get_id(), __func__)
			<= "web socket shakehand invalid 'HTTP '.";
		return;
	}

	if (!WebSocketShakeHand().parse_rsp(data_head, m_handler->websocket_key()))
	{
		ECO_INFO << NetLog(get_id(), __func__)
			<= "web socket shakehand invalid.";
		close_and_notify();
		return;
	}
	async_recv_by_client();
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_connect(
	IN bool is_connected,
	IN const eco::Error* e)
{
	// "client on_connect" called only by client peer.
	if (!is_connected)
	{
		m_handler->on_connect(e);
		return;
	}

	set_connected();					// set peer state.
	if (handler().websocket())
	{
		m_state.set_websocket();
		send_websocket_shakehand();
		async_recv_shakehand();
	}
	else
	{
		async_recv_by_client();
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_read_head(IN eco::String& data, IN const eco::Error* err)
{
	if (err != nullptr)	// if peerection occur error, close it.
	{
		ECO_DEBUG << NetLog(get_id(), __func__) <= *err;
		close_and_notify();
		return; 
	}

	// parse message body length from protocol head.
	eco::Error e;
	uint32_t data_size = 0;
	uint32_t head_size = data.size();
	if (!protocol_head().decode_data_size(
		data_size, data.c_str(), head_size, e))
	{
		ECO_DEBUG << NetLog(get_id(), __func__) <= e;
		close_and_notify();
		return;
	}

	// when recv head from peer, means peer is alive.
	m_state.set_peer_live(true);

	// allocate memory for store coming data.
	if (data_size == 0)		// empty message.
	{
		m_handler->on_read(this, data);
		async_recv();		// recv next coming data.
		return;
	}

	// recv data from peer.
	data.resize(head_size + data_size);
	m_connector.async_read_data(data, head_size);
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_read_data(IN eco::String& data, IN const eco::Error* e)
{
	if (e != nullptr)	// if peer occur error, release it.
	{
		ECO_DEBUG << NetLog(get_id(), __func__) <= *e;
		close_and_notify();
		return;
	}

	if (!m_state.ready())
	{
		// handle websocket rsp(client) or websocket req(server).
		if (m_state.websocket())
		{
			if (m_state.server())
				handle_websocket_shakehand_req(data.c_str(), data.size());
			else
				handle_websocket_shakehand_rsp(data.c_str(), data.size());
		}
		return;
	}

	// when recv message from peer, means peer is active.
	// post data message to tcp server.
	m_state.set_peer_active(true);
	m_handler->on_read(this, data);
	async_recv();		// recv next coming data.
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_write(IN const uint32_t size, IN const eco::Error* e)
{
	if (e != nullptr)
	{
		ECO_DEBUG << NetLog(get_id(), __func__) <= *e;
		close_and_notify();
		return;
	}

	// notify tcp handler.
	m_handler->on_send(this, size);
}


//##############################################################################
//##############################################################################
TcpPeer::TcpPeer(
	IN IoService* io_server,
	IN void* msg_server,
	IN TcpPeerHandler* hdl)
{
	m_impl = new Impl(io_server, (MessageWorker*)msg_server, hdl);
	m_impl->init(*this);
}
TcpPeer::ptr TcpPeer::make(
	IN IoService* io_server,
	IN void* msg_server,
	IN TcpPeerHandler* hdl)
{
	TcpPeer::ptr peer(new TcpPeer(io_server, (MessageWorker*)msg_server, hdl));
	peer->impl().prepare(peer);
	return peer;
}
TcpPeerHandler& TcpPeer::handler()
{
	return impl().handler();
}
ConnectionData* TcpPeer::data()
{
	return impl().m_data.get();
}
size_t TcpPeer::get_id() const
{
	return impl().get_id();
}
eco::String TcpPeer::get_ip() const
{
	return std::move(impl().m_connector.get_ip());
}
TcpState& TcpPeer::state()
{
	return impl().state();
}
const TcpState& TcpPeer::get_state() const
{
	return impl().get_state();
}
eco::atomic::State& TcpPeer::data_state()
{
	return impl().m_data_state;
}
const eco::atomic::State& TcpPeer::get_data_state() const
{
	return impl().m_data_state;
}
void TcpPeer::set_connected()
{
	impl().set_connected();
}
void TcpPeer::set_option(IN bool no_delay)
{
	impl().m_connector.set_option(no_delay);
}
void TcpPeer::async_connect(IN const Address& addr)
{
	impl().async_connect(addr);
}
void TcpPeer::async_recv()
{
	impl().async_recv();
}
void TcpPeer::async_recv_shakehand()
{
	impl().async_recv_shakehand();
}
void TcpPeer::async_recv_by_server()
{
	impl().async_recv_by_server();
}
void TcpPeer::async_send(IN eco::String& data, IN const uint32_t start)
{
	impl().async_send(data, start);
}
void TcpPeer::async_send(IN const MessageMeta& meta, IN Protocol& prot)
{
	impl().async_send(meta, prot);
}
void TcpPeer::close()
{
	impl().close();
}
void TcpPeer::close_and_notify()
{
	impl().close_and_notify();
}
void TcpPeer::async_response(
	IN Codec& codec,
	IN const uint32_t type,
	IN const Context& c,
	IN Protocol& prot,
	IN const bool last,
	IN const bool encrypted)
{
	MessageMeta meta(codec, c.m_meta.m_session_id, type, true);
	eco::add(meta.m_category, c.m_meta.m_category);
	meta.set_request_data(c.m_meta.m_request_data, c.m_meta.m_option);
	meta.set_last(last);
	impl().async_send(meta, prot);
}


////////////////////////////////////////////////////////////////////////////////
}}