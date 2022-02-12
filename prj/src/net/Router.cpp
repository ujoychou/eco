#include "Pch.h"
#include "Router.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/TcpPeer.h>
#include <eco/net/TcpConnection.h>
#include "TcpOuter.h"
#include "TcpServer.ipp"
#include "TcpClient.ipp"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
// return whether need to dispatch meta context.
void DispatchHandler::handle_server(Context& c, TcpPeer& peer)
{
	const char* func = "server_dc";
	TcpSessionOuter sess(c.m_session);
	auto* server = (TcpServer::Impl*)sess.impl().m_owner.m_owner;
	assert(server != nullptr);

	// #.handle session.
	auto id = peer.impl().id();
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
		ECO_WARN << NetLog(id, func, c.m_meta.m_session_id)
			< " discard " < c.type() <= c.m_meta.m_category;
		return ;
	}

	// make connection data.
	if (!dispatch(c.type(), c))
	{
		ECO_ERROR << NetLog(id, func, c.m_meta.m_session_id)
			< " unknown message type " <= c.type();
		return ;
	}
	server->m_statistics.on_live(peer.impl(), true);
}


////////////////////////////////////////////////////////////////////////////////
void DispatchHandler::handle_client(OUT Context& c, IN TcpPeer& peer)
{
	TcpSessionOuter sess(c.m_session);
	auto* client = (TcpClient::Impl*)sess.impl().m_owner.m_owner;
	assert(client != nullptr);

	// #.handle sync request.
	if (eco::has(c.m_meta.m_category, category_sync))
	{
		// "sync call" isn't need to handle by "dispatcher";
		// 1.sync decode rsp message.
		auto sync = client->pop_sync(c.m_meta.get_req4(), c.m_meta.is_last());
		if (sync != nullptr)
		{
			if (!sync->decode(c.m_message, c.has_error()))
			{
				ECO_ERROR << Log(c)(rsp) <= "decode fail.";
			}
			else
			{
				client->on_live(true);
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
			client->on_live(true);
			async->m_func(c);
		}
		return ;
	}

	// #.handle request.
	if (eco::has(c.m_meta.m_category, category_message))
	{
		if (!dispatch(c.m_meta.m_message_type, c))
		{
			ECO_ERROR < " unknown message type " <= c.type();
			return ;
		}
		client->on_live(true);
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

	// only triger the "receive event".
	if (receive_mode())
	{
		return m_recv(peer, dc);
	}

	// parse message meta.
	Context c;
	auto sess_id = c.m_meta.m_session_id;
	c.m_meta.m_category = dc.m_category;
	if (!dc.m_protocol->decode_meta(
		c.m_meta, c.m_message, dc.m_data, dc.m_head_size))
	{
		ECO_ERROR << NetLog(impl.id(), func, sess_id) <=
			eco::Error();
		return;
	}

	// get message type id and dispatch to the handler.
	TcpSessionOuter sess(c.m_session);
	TcpConnectionOuter conn(sess.impl().m_conn);
	sess.impl().m_owner.set(impl.owner(), impl.state().server());
	conn.set_peer(dc.m_peer_wptr);
	conn.set_id(impl.id());
	c.m_data = std::move(dc.m_data);
	// handle and dispatch context.
	if (!impl.state().server())
		handle_client(c, *peer);
	else
		handle_server(c, *peer);
}


////////////////////////////////////////////////////////////////////////////////
}}