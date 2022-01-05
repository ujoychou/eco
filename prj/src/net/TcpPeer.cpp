#include "Pch.h"
#include "TcpPeer.ipp"
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/Ecode.h>
#include <eco/log/Log.h>
#include <eco/net/protocol/StringCodec.h>
#include <eco/net/protocol/TcpProtocol.h>
#include <net/protocol/WebSocketShakeHand.h>
#include <net/protocol/WebSocketProtocol.h>
#include "TcpOuter.h"


ECO_NS_BEGIN(eco);
namespace net{;
ECO_OBJECT_IMPL(TcpPeer);
/* this thread local data has two serious error:
1.now is: acceptor thread recv data, but not io thread.
2.each connection should has a data_head instead of each thread.
*/
//EcoThreadLocal char s_data_head[32] = { 0 };
////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::make_connection_data(const MakeConnectionData& make_func)
{
	m_data.reset(make_func());
	TcpConnectionOuter conn(m_data->connection());
	conn.set_id(id());
	conn.set_peer(m_peer_observer);
}


////////////////////////////////////////////////////////////////////////////////
inline void TcpPeer::Impl::async_recv()
{
	m_connector.async_read();
}
void TcpPeer::Impl::async_recv_shakehand()
{
	m_state.set_websocket();
	m_connector.async_read_until(WebSocketShakeHand::head_end());
}


////////////////////////////////////////////////////////////////////////////////
inline void TcpPeer::Impl::send_websocket_shakehand()
{
	WebSocketShakeHand shake_hand;
	send(shake_hand.format(), 0);
}


////////////////////////////////////////////////////////////////////////////////
inline void TcpPeer::Impl::handle_websocket_shakehand_req(
	IN const char* data_head, IN const uint32_t head_size)
{
	assert(m_state.websocket());
	if (eco::find(data_head, head_size, "GET ") != data_head)
	{
		ECO_THIS_ERROR(e_websocket_shakehand_get) <
			"websocket shakehand req find 'GET ' fail.";
		ECO_INFO << NetLog(id(), __func__) <= eco::this_thread::error();
		close_and_notify();
		return;
	}

	WebSocketShakeHand shake_hand;
	if (!shake_hand.parse_req(data_head, head_size))
	{
		ECO_THIS_ERROR(e_websocket_shakehand_req) <
			"websocket req shakehand invalid.";
		ECO_INFO << NetLog(id(), __func__) <= eco::this_thread::error();
		close_and_notify();
		return;
	}
	send(shake_hand.response(), 0);
	async_recv_by_server();
}


////////////////////////////////////////////////////////////////////////////////
inline void TcpPeer::Impl::handle_websocket_shakehand_rsp(
	IN const char* data_head, IN const uint32_t head_size)
{
	assert(m_state.websocket());
	if (eco::find(data_head, head_size, "HTTP") != data_head)
	{
		ECO_THIS_ERROR(e_websocket_shakehand_http) <
			"websocket shakehand rsp find 'HTTP ' fail.";
		ECO_INFO << NetLog(id(), __func__) <= eco::this_thread::error();
		close_and_notify();
		return;
	}

	if (!WebSocketShakeHand().parse_rsp(data_head, m_handler->m_websocket_key))
	{
		ECO_THIS_ERROR(e_websocket_shakehand_rsp) <
			"websocket rsp shakehand invalid.";
		ECO_INFO << NetLog(id(), __func__) <= eco::this_thread::error();
		close_and_notify();
		return;
	}
	async_recv_by_client();
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_accept_state(uint64_t id)
{
	m_id = id;
	m_state.set_connected();
	m_state.set_peer_live(true);
	m_state.set_server();
}
void TcpPeer::Impl::on_accept(const TcpOption& opt)
{
	init_option(opt);

	// tcp_connection start work and recv request.
	if (opt.websocket())
	{
		async_recv_shakehand();
	}
	else
	{
		async_recv_by_server();
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_connect(IN bool err)
{
	// "client on_connect" called only by client peer.
	if (err)
	{
		m_handler->m_on_connect(err);
		return;
	}

	m_state.set_connected();					// set peer state.
	if (option().websocket())
	{
		send_websocket_shakehand();
		async_recv_shakehand();
	}
	else
	{
		async_recv_by_client();
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_read(IN MessageHead& head, eco::String& data, bool err)
{
	if (err)	// if peer occur error, release it.
	{
		ECO_DEBUG << NetLog(id(), __func__) <= eco::this_thread::error();
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

	// post data message to tcp server.
	m_handler->m_on_read(this, head, data);
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_write(IN uint32_t size, bool err)
{
	if (err)
	{
		ECO_DEBUG << NetLog(id(), __func__) <= eco::this_thread::error();
		close_and_notify();
		return;
	}

	// notify tcp handler.
	m_handler->m_on_send(this, size);
}


////////////////////////////////////////////////////////////////////////////////
eco::Result TcpPeer::Impl::on_decode_head(
	MessageHead& head, const char* buff, uint32_t size)
{
	/*@forbid memory explosion and crush when [on_decode_head] aways
	return false, and head.message_size() is (A)==0 or (B)>size.
	1.keep message size algor right when get head_size & data_size.(A)
	2.set message max size in [on_decode_head] & protocol.(B)
	3.set tcp server & client max size.(B)
	4.set tcpconnector buff max size.(B)
	note important: all up 4 point must satisfied.
	*/
	eco::Result res = eco::ok;
	if (!m_handler->m_on_decode_head)
		res = protocol().on_decode_head(head, buff, size);
	else
		res = m_handler->m_on_decode_head(head, buff, size);
	if (res != eco::ok) return res;

	// message is valid.
	if (head.message_size() == 0)
	{
		ECO_THIS_ERROR(e_message_decode) < "decode head message_size=0";
		return eco::error;
	}
	if (head.message_size() > option().max_byte_size())
	{
		ECO_THIS_ERROR(e_message_overszie)
			< "message size > tcp option max_byte_size: "
			< head.message_size() < '>'
			< option().max_byte_size();
		return eco::error;
	}
	return (head.message_size() <= size) ? eco::ok : eco::fail;
}


//##############################################################################
//##############################################################################
TcpPeer::TcpPeer(IoWorker* io_server, void* msg_server, TcpPeerHandler* hdl)
{
	m_impl = new Impl(io_server, (MessageWorker*)msg_server, hdl);
	m_impl->init(*this);
}
TcpPeer::ptr TcpPeer::make(IoWorker* io_srv, void* msg_srv, TcpPeerHandler* hdl)
{
	TcpPeer::ptr peer(std::make_shared<TcpPeer>(io_srv, msg_srv, hdl));
	peer->impl().prepare(peer);
	return peer;
}
void TcpPeer::init_option(const TcpOption& opt)
{
	impl().init_option(opt);
}
TcpPeerHandler& TcpPeer::handler()
{
	return impl().handler();
}
ConnectionData* TcpPeer::data()
{
	return impl().m_data.get();
}
SessionId TcpPeer::id() const
{
	return impl().id();
}
eco::String TcpPeer::ip() const
{
	return impl().m_connector.ip();
}
uint32_t TcpPeer::port() const
{
	return impl().m_connector.port();
}
const TcpState& TcpPeer::state() const
{
	return impl().state();
}
void TcpPeer::authorize(const char* user, const char* lang, TcpPeer::ptr& peer)
{
	std_lock_guard lock(impl().m_server->mutex());
	impl().m_user = user;
	impl().m_lang = lang;
	impl().m_handler->m_on_auth(user, lang, peer);
	ECO_KEY < NetLog(peer->impl().id(), __func__) <= user <= lang;
}
bool TcpPeer::authorized() const
{
	std_lock_guard lock(impl().m_server->mutex());
	return !impl().m_user.empty();
}
const char* TcpPeer::user() const
{
	std_lock_guard lock(impl().m_server->mutex());
	return impl().m_user.c_str();
}
const char* TcpPeer::lang() const
{
	std_lock_guard lock(impl().m_server->mutex());
	return impl().m_lang.c_str();
}


////////////////////////////////////////////////////////////////////////////////
bool TcpPeer::async_connect(IN const Address& addr)
{
	return impl().async_connect(addr);
}
void TcpPeer::send(IN eco::String& data, IN uint32_t start)
{
	impl().send(data, start);
}
void TcpPeer::send(IN const MessageMeta& meta)
{
	impl().send(meta);
}
void TcpPeer::response(MessageMeta& meta, const Context& c)
{
	meta.session_id(c.m_meta.m_session_id);
	if (eco::has(c.m_meta.m_category, category_sync))
		eco::add(meta.m_category, category_sync);
	if (eco::has(c.m_meta.m_option, option_data))
		eco::add(meta.m_option, option_data);

	if (eco::has(c.m_meta.m_option, option_req1))
		eco::add(meta.m_option, option_req1);
	if (eco::has(c.m_meta.m_option, option_req2))
		eco::add(meta.m_option, option_req2);
	if (eco::has(c.m_meta.m_option, option_req4))
		eco::add(meta.m_option, option_req4);
	if (eco::has(c.m_meta.m_option, option_req8))
		eco::add(meta.m_option, option_req8);
	meta.m_option_data = c.m_meta.m_option_data;
	impl().send(meta);
}
void TcpPeer::close()
{
	impl().close();
}
void TcpPeer::close_and_notify()
{
	impl().close_and_notify();
}
bool TcpPeer::stopped() const
{
	return impl().m_connector.stopped();
}


////////////////////////////////////////////////////////////////////////////////
}}