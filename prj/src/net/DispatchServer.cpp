#include "PrecHeader.h"
#include "DispatchServer.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/TcpPeer.h>
#include <eco/net/TcpConnection.h>
#include <eco/net/Log.h>
#include "TcpOuter.h"
#include "TcpServer.ipp"
#include "TcpClient.ipp"


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
// return whether need to dispatch meta context.
void DispatchHandler::handle_server(Context& c, TcpPeer& peer, Protocol* prot)
{
	const char* func = "server_dc";
	TcpSessionOuter sess(c.m_session);
	auto* server = (TcpServer::Impl*)sess.impl().m_owner.m_owner;
	assert(server != nullptr);

	// #.handle session.
	if (c.m_meta.m_session_id != none_session)
	{
		// open the exist session.
		if (!sess.open(c.m_meta.m_session_id))
		{
			ECO_WARN << "session has expired: sid=" << c.m_meta.m_session_id;
			return ;
		}
	}

	// #.handle request.
	if (!eco::has(c.m_meta.m_category, category_message))
	{
		ECO_WARN << NetLog(peer.impl().get_id(), func, c.m_meta.m_session_id)
			<= "discard" <= c.m_meta.m_category
			<= c.m_meta.m_message_type;
		return ;
	}

	// make connection data.
	server->set_valid_peer(peer, prot);
	peer.impl().state().set_peer_active(true);
	dispatch(c.m_meta.m_message_type, c);
}


////////////////////////////////////////////////////////////////////////////////
void DispatchHandler::handle_client(OUT Context& c, IN TcpPeer& peer)
{
	TcpSessionOuter sess(c.m_session);
	auto* client = (TcpClient::Impl*)sess.impl().m_owner.m_owner;
	assert(client != nullptr);

	// #.handle authority.
	if (eco::has(c.m_meta.m_category, category_authority) &&
		eco::has(c.m_meta.m_category, category_session))
	{
		// find authority request.
		auto* key = reinterpret_cast<TcpSession*>(c.m_meta.m_request_data);
		if (key == nullptr)
		{
			ECO_WARN << "authority request id invalid.";
			return ;
		}
		SessionDataPack::ptr pack = client->find_authority(key);
		if (pack == nullptr)
		{
			ECO_WARN << "authority pack has expired: req_id="
				<< c.m_meta.m_request_data;
			return ;
		}

		// keep user live.
		sess.impl().m_user = pack->m_user_observer.lock();
		if (sess.impl().m_user == nullptr)
		{
			ECO_WARN << "authority user has expired: sid="
				<< c.m_meta.m_request_data;
			return ;
		}
		// open session: set session data.
		sess.impl().m_session_wptr = pack->m_session;
		sess.impl().m_session_id = c.m_meta.m_session_id;
		// set session.
		if (c.m_meta.m_session_id != none_session)
		{
			SessionDataOuter outer(*pack->m_session);
			outer.set_id(c.m_meta.m_session_id);
			client->m_session_map.set(c.m_meta.m_session_id, pack);
		}
		c.m_meta.m_request_data = pack->m_request_data;
	}
	else
	{
		if (c.m_meta.m_session_id != none_session)
		{
			// can't find the session id.
			if (!sess.open(c.m_meta.m_session_id))
			{
				ECO_WARN << "session has expired id=" << c.m_meta.m_session_id;
				return ;
			}
		}
	}

	// #.handle sync request.
	if (eco::has(c.m_meta.m_category, category_sync))
	{
		// "sync call" isn't need to handle by "dispatcher";
		// 1.sync decode rsp message.
		auto sync = client->pop_sync(c.m_meta.get_req4(), c.m_meta.is_last());
		if (sync != nullptr)
		{
			sync->m_error_id = c.m_meta.m_error_id;
			if (!sync->decode(c.m_message))
			{
				ECO_ERROR << Log(c.m_session, c.m_meta.m_message_type,
					nullptr)(eco::net::rsp) <= "decode fail.";
			}
			else
			{
				peer.impl().state().set_peer_active(true);
			}

			if (c.is_last())
			{
				sync->m_monitor.finish_one();		// only one.
			}
			return;
		}

		// 2.async callback func.
		auto async = client->pop_async(c.m_meta.get_req4(), c.m_meta.is_last());
		if (async != nullptr)
		{
			peer.impl().state().set_peer_active(true);
			async->m_func(c);
		}
		return ;
	}

	// #.handle request.
	if (eco::has(c.m_meta.m_category, category_message))
	{
		peer.impl().state().set_peer_active(true);
		dispatch(c.m_meta.m_message_type, c);
	}
}


////////////////////////////////////////////////////////////////////////////////
void DispatchHandler::operator()(IN DataContext& dc)
{
	const char* func = "handle_dc";
	// check whether peer is expired.
	TcpPeer::ptr peer = dc.m_peer_wptr.lock();
	if (peer == nullptr)
	{
		ECO_DEBUG << func <= "peer is empty.";
		return;
	}
	auto& impl = peer->impl();
	
	// #.heartbeat is unrelated to session, it's manage remote peer life.
	if (is_heartbeat(dc.m_category))
	{
		if (impl.option().response_heartbeat())
		{
			impl.send_heartbeat(*dc.m_protocol);
		}
		return ;
	}

	// only triger the "receive event".
	if (receive_mode())
	{
		if (impl.state().server())
		{
			auto* server = (TcpServer::Impl*)impl.owner();
			assert(server != nullptr);
			server->set_valid_peer(*peer, dc.m_protocol);
		}
		return m_recv(peer, dc);
	}

	// parse message meta.
	Context c;
	eco::Error e;
	auto sess_id = c.m_meta.m_session_id;
	c.m_meta.m_category = dc.m_category;
	if (!dc.m_protocol->decode_meta(
		c.m_meta, c.m_message, dc.m_data, dc.m_head_size, e))
	{
		ECO_ERROR << NetLog(impl.get_id(), func, sess_id) <= e;
		return;
	}

	// get message type id and dispatch to the handler.
	TcpSessionOuter sess(c.m_session);
	TcpConnectionOuter conn(sess.impl().m_conn);
	sess.impl().m_owner.set(impl.owner(), impl.state().server());
	conn.set_peer(dc.m_peer_wptr);
	conn.set_protocol(*dc.m_protocol);
	conn.set_id(impl.get_id());
	c.m_data = std::move(dc.m_data);
	// handle and dispatch context.
	if (!impl.state().server())
		handle_client(c, *peer);
	else
		handle_server(c, *peer, dc.m_protocol);
}


////////////////////////////////////////////////////////////////////////////////
}}