#include "PrecHeader.h"
#include <eco/net/DispatchServer.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/TcpPeer.h>
#include <eco/net/TcpConnection.h>
#include <eco/net/Log.h>
#include "TcpOuter.h"
#include "TcpServer.ipp"
#include "TcpClient.ipp"



////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
// return whether need to dispatch meta context.
bool handle_server_context(OUT Context& c, IN TcpPeer& peer)
{
	TcpSessionOuter sess(c.m_session);
	auto* server = (TcpServer::Impl*)sess.impl().m_owner.m_owner;
	assert(server != nullptr);

	// #.handle session.
	if (c.m_meta.m_session_id != none_session)
	{
		// open the exist session.
		if (!sess.open(c.m_meta.m_session_id))
		{
			return false;
		}
	}

	// #.handle request.
	return eco::has(c.m_meta.m_category, category_message);
}


////////////////////////////////////////////////////////////////////////////////
bool handle_client_context(
	OUT Context& c, IN  TcpPeer& peer)
{
	TcpSessionOuter sess(c.m_session);
	auto* client = (TcpClient::Impl*)sess.impl().m_owner.m_owner;
	assert(client != nullptr);

	// #.handle authority.
	if (eco::has(c.m_meta.m_category, category_authority))
	{
		// authority validate fail.
		if (c.m_meta.m_session_id == none_session)
		{
			return false;
		}
		// find authority request.
		auto* sess = reinterpret_cast<TcpSession*>(c.m_meta.m_request_data);
		if (sess == nullptr)
		{
			return false;
		}
		SessionDataPack::ptr pack = client->find_authority(sess);
		// user object destroyed.
		if (pack == nullptr || pack->m_user_observer.expired())
		{
			return false;
		}
		c.m_meta.m_request_data = pack->m_request_data;
		client->m_session_map.set(c.m_meta.m_session_id, pack);
	}

	// #.handle session.
	if (c.m_meta.m_session_id != none_session)
	{
		// can't find the session id.
		if (!sess.open(c.m_meta.m_session_id))
		{
			return false;
		}
	}

	// #.handle request.
	return eco::has(c.m_meta.m_category, category_message);
}


////////////////////////////////////////////////////////////////////////////////
void DispatchHandler::operator()(IN DataContext& dc) const
{
	// check whether peer is expired.
	TcpPeer::ptr peer = dc.m_peer_wptr.lock();
	if (peer == nullptr)
	{
		return;
	}
	
	// #.heartbeat is unrelated to session, it's manage remote peer life.
	TcpSessionOwnerOuter owner(dc.m_session_owner);
	if (eco::has(dc.m_category, category_heartbeat))
	{
		peer->impl().state().set_peer_live(true);
		if (owner.response_heartbeat())
		{
			peer->impl().async_send_heartbeat(*owner.protocol_head());
		}
		return ;
	}

	// parse message meta.
	eco::Error e;
	Context c;
	c.m_meta.m_category = dc.m_category;
	if (!dc.m_prot->decode(c.m_meta, c.m_message, dc.m_data, e))
	{
		ECO_LOG_NET(peer->get_id(), c.m_meta.m_session_id, e);
		return;
	}

	// get message type id and dispatch to the handler.
	TcpSessionOuter sess(c.m_session);
	TcpConnectionOuter conn(sess.impl().m_conn);
	sess.impl().m_peer = peer.get();
	sess.impl().m_owner = dc.m_session_owner;
	conn.set_peer(dc.m_peer_wptr);
	conn.set_protocol(*dc.m_prot);
	conn.set_id(peer->get_id());
	c.m_data = std::move(dc.m_data);
	if (dc.m_session_owner.m_server && handle_server_context(c, *peer) ||
		!dc.m_session_owner.m_server && handle_client_context(c, *peer))
	{
		peer->impl().state().set_peer_active(true);
		dispatch(c.m_meta.m_message_type, c);
	}
}



////////////////////////////////////////////////////////////////////////////////
}}