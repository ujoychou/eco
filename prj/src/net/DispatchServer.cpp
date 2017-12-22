#include "PrecHeader.h"
#include <eco/net/DispatchServer.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/TcpPeer.h>
#include "TcpSession.ipp"
#include "TcpServer.ipp"
#include "TcpClient.ipp"



////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
// return whether need to dispatch meta context.
bool handle_server_context(
	IN MetaContext& mc, IN MessageMeta& meta, IN TcpPeer& peer)
{
	TcpServer::Impl* server = &reinterpret_cast<TcpServer*>(
		&mc.m_session.impl().m_host.m_host)->impl();

	// #.handle session.
	if (meta.m_session_id != none_session)
	{
		// open the exist session.
		if (!mc.m_session.open(meta.m_session_id))
		{
			return false;
		}
	}

	// #.handle heartbeat.
	if (eco::has(meta.m_category, category_heartbeat) &&
		meta.m_session_id == none_session &&
		server->m_option.response_heartbeat())
	{
		peer.impl().state().peer_live(true);
		peer.impl().async_send_heartbeat(*server->m_prot_head);
		return false;
	}

	// #.handle request.
	return eco::has(meta.m_category, category_message);
}


////////////////////////////////////////////////////////////////////////////////
bool handle_client_context(
	OUT MetaContext& mc, IN MessageMeta& meta, IN  TcpPeer& peer)
{
	TcpClient::Impl* client = &reinterpret_cast<TcpClient*>(
		&mc.m_session.impl().m_host.m_host)->impl();

	// #.handle authority.
	if (eco::has(meta.m_category, category_authority))
	{
		// authority validate fail.
		if (meta.m_session_id == none_session)
		{
			return false;
		}
		// find authority request.
		auto* sess = reinterpret_cast<TcpSession*>(meta.m_request_data);
		if (sess == nullptr)
		{
			return false;
		}
		SessionDataPack::ptr pack;
		if (!client->m_authority_map.find(pack, sess) ||
			pack->m_user_observer.expired())	// user object destroyed.
		{
			return false;
		}
		client->m_session_map.set(meta.m_session_id, pack);
	}

	// #.handle session.
	if (meta.m_session_id != none_session)
	{
		// can't find the session id.
		if (!mc.m_session.open(meta.m_session_id))
		{
			return false;
		}
	}

	// #.handle heartbeat.
	if (eco::has(meta.m_category, category_heartbeat) &&
		meta.m_session_id == none_session &&
		client->m_option.response_heartbeat())
	{
		peer.impl().state().peer_live(true);
		peer.impl().async_send_heartbeat(*client->m_prot_head);
		return false;
	}

	// #.handle request.
	return eco::has(meta.m_category, category_message);
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
	dc.m_session_host.set_peer(*peer);

	// parse message meta.
	eco::Error e;
	MetaContext mc;
	MessageMeta meta;
	if (!dc.m_prot->decode(meta, mc.m_message, dc.m_data, *dc.m_prot_head, e))
	{
		e << (dc.m_session_host.m_type == tcp_session_host_server
			? "tcp server" : "tcp client") << " decode fail.";
		EcoNet(EcoError, *peer, "dispatch", e);
		return;
	}

	// get message type id and dispatch to the handler.
	if (dc.m_session_host.m_type == tcp_session_host_server &&
		handle_server_context(mc, meta, *peer) ||
		dc.m_session_host.m_type == tcp_session_host_client &&
		handle_client_context(mc, meta, *peer))
	{
		peer->impl().state().peer_active(true);
		mc.set_context(dc, meta);
		dispatch(mc.m_request_type, mc);
	}
}



////////////////////////////////////////////////////////////////////////////////
}}