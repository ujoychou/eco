#include "PrecHeader.h"
#include "DispatchServer.h"
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
			ECO_WARN << "session has expired: sid=" << c.m_meta.m_session_id;
			return false;
		}
	}

	// #.handle request.
	return eco::has(c.m_meta.m_category, category_message);
}


////////////////////////////////////////////////////////////////////////////////
bool handle_client_context(OUT Context& c, IN  TcpPeer& peer)
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
			return false;
		}
		SessionDataPack::ptr pack = client->find_authority(key);
		if (pack == nullptr)
		{
			ECO_WARN << "authority pack has expired: req_id="
				<< c.m_meta.m_request_data;
			return false;
		}

		// keep user live.
		sess.impl().m_user = pack->m_user_observer.lock();
		if (sess.impl().m_user == nullptr)
		{
			ECO_WARN << "authority user has expired: sid="
				<< c.m_meta.m_request_data;
			return false;
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
				ECO_WARN << "session has expired: sid=" << c.m_meta.m_session_id;
				return false;
			}
		}
	}

	// #.handle sync request.
	if (eco::has(c.m_meta.m_category, category_sync))
	{
		// "sync call" isn't need to handle by "dispatcher";
		// 1.sync decode rsp message.
		auto sync = client->pop_sync(c.m_meta.get_req4(), c.m_meta.last());
		if (sync != nullptr)
		{
			if (!sync->m_rsp_codec->decode(
				c.m_message.m_data, c.m_message.m_size))
			{
				ECO_ERROR(eco::net::rsp) << Log(c.m_session,
					c.m_meta.m_message_type, nullptr) <= "decode fail.";
			}

			// push the object to set.
			if (sync->m_rsp_set != nullptr)
			{
				sync->m_rsp_set->push_back(sync->m_rsp_codec->get_message());
			}
			if (c.last())
			{
				sync->m_monitor.finish_one();		// only one.
			}
			return false;
		}
		// 2.async callback func.
		auto async = client->pop_async(c.m_meta.get_req4(), c.m_meta.last());
		if (async != nullptr)
		{
			async->m_func(c);
		}
		return false;
	}

	// #.handle request.
	return eco::has(c.m_meta.m_category, category_message);
}


////////////////////////////////////////////////////////////////////////////////
void DispatchHandler::operator()(IN DataContext& dc) const
{
	const char* func = "handle_dc";
	// check whether peer is expired.
	TcpPeer::ptr peer = dc.m_peer_wptr.lock();
	if (peer == nullptr)
	{
		ECO_DEBUG << func <= "peer is empty.";
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
		ECO_DEBUG << NetLog(peer->get_id(), "heartbeat recv");
		return ;
	}

	// parse message meta.
	eco::Error e;
	Context c;
	auto sess_id = c.m_meta.m_session_id;
	c.m_meta.m_category = dc.m_category;
	if (!dc.m_prot->decode(c.m_meta, c.m_message, dc.m_data, e))
	{
		ECO_ERROR << NetLog(peer->get_id(), func, sess_id) <= e;
		return;
	}

	// get message type id and dispatch to the handler.
	TcpSessionOuter sess(c.m_session);
	TcpConnectionOuter conn(sess.impl().m_conn);
	sess.impl().m_owner = dc.m_session_owner;
	conn.set_peer(dc.m_peer_wptr);
	conn.set_protocol(*dc.m_prot);
	conn.set_id(peer->get_id());
	c.m_data = std::move(dc.m_data);

	// receive by server
	if (dc.m_session_owner.m_server)
	{
		if (handle_server_context(c, *peer))
		{
			peer->impl().state().set_peer_active(true);
			dispatch(c.m_meta.m_message_type, c);
		}
		else
		{
			ECO_WARN << NetLog(peer->get_id(), func, sess_id)
				<= "discard" <= c.m_meta.m_category 
				<= c.m_meta.m_message_type;
		}
	}
	// receive by client
	else
	{
		if (handle_client_context(c, *peer))
		{
			peer->impl().state().set_peer_active(true);
			dispatch(c.m_meta.m_message_type, c);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
}}