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
	OUT TcpSession& sess, IN MessageMeta& meta, IN TcpPeer& peer)
{
	TcpServer::Impl& server = reinterpret_cast<TcpServer*>(
		&sess.impl().m_host.m_host)->impl();

	// #.handle session.
	if (meta.m_session_id != none_session)
	{
		// open the exist session.
		if (!sess.open(meta.m_session_id))
		{
			return false;
		}
	}

	// #.handle request.
	return eco::has(meta.m_category, category_message);
}


////////////////////////////////////////////////////////////////////////////////
bool handle_client_context(
	OUT TcpSession& sess, IN MessageMeta& meta, IN  TcpPeer& peer)
{
	TcpClient::Impl* client = &reinterpret_cast<TcpClient*>(
		&sess.impl().m_host.m_host)->impl();

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
		SessionDataPack::ptr pack = client->find_authority(sess);
		if (pack == nullptr || pack->m_user_observer.expired())	// user object destroyed.
		{
			return false;
		}
		client->m_session_map.set(meta.m_session_id, pack);
	}

	// #.handle session.
	if (meta.m_session_id != none_session)
	{
		// can't find the session id.
		if (!sess.open(meta.m_session_id))
		{
			return false;
		}
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

	// #.heartbeat is unrelated to session, it's manage remote peer life.
	if (eco::has(dc.m_category, category_heartbeat) &&
		dc.m_session_host.response_heartbeat())
	{
		peer->impl().state().peer_live(true);
		peer->impl().async_send_heartbeat(*dc.m_session_host.protocol_head());
		return ;
	}

	// parse message meta.
	eco::Error e;
	eco::Bytes message;
	MessageMeta meta(dc.m_category);
	uint32_t head_size = dc.m_prot_head->head_size();
	if (!dc.m_prot->decode(meta, message, dc.m_data, head_size, e))
	{
		e << (dc.m_session_host.m_type == tcp_session_host_server
			? "tcp server" : "tcp client") << " decode fail.";
		EcoNet(EcoError, *peer, "dispatch", e);
		return;
	}

	// get message type id and dispatch to the handler.
	MetaContext mc(dc, message, meta);
	if (dc.m_session_host.m_type == tcp_session_host_server &&
		handle_server_context(mc.m_session, meta, *peer) ||
		dc.m_session_host.m_type == tcp_session_host_client &&
		handle_client_context(mc.m_session, meta, *peer))
	{
		peer->impl().state().peer_active(true);
		dispatch(mc.m_request_type, mc);
	}
}



////////////////////////////////////////////////////////////////////////////////
}}