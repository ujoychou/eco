#include "PrecHeader.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/net/Context.h>
#include "TcpSession.ipp"
#include "TcpServer.ipp"
#include "TcpClient.ipp"



////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
bool TcpSessionHost::session_mode() const
{
	if (m_type == tcp_session_host_client)
	{
		auto* client = (TcpClient::Impl*)m_host;
		return client->session_mode();
	}
	else if (m_type == tcp_session_host_server)
	{
		auto* server = (TcpServer::Impl*)m_host;
		return server->session_mode();
	}
	assert(false);
	return false;
}
bool TcpSessionHost::response_heartbeat() const
{
	if (m_type == tcp_session_host_client)
	{
		return ((TcpClient::Impl*)m_host)->m_option.response_heartbeat();
	}
	assert(m_type == tcp_session_host_server);
	return ((TcpServer::Impl*)m_host)->m_option.response_heartbeat();
}
ProtocolHead* TcpSessionHost::protocol_head() const
{
	if (m_type == tcp_session_host_client)
	{
		return ((TcpClient::Impl*)m_host)->m_prot_head.get();
	}
	assert(m_type == tcp_session_host_server);
	return ((TcpServer::Impl*)m_host)->m_prot_head.get();
}


ECO_SHARED_IMPL(TcpSession);
////////////////////////////////////////////////////////////////////////////////
bool TcpSession::open(IN const SessionId session_id)
{
	// create new session.
	if (session_id == none_session)
	{
		// note: client open a none session is invalid.
		if (impl().m_host.m_type == tcp_session_host_server)
		{
			auto* server = (TcpServer::Impl*)impl().m_host.m_host;
			SessionData::ptr sess = server->add_session(
				impl().m_session_id, *impl().m_host.m_peer);
			if (sess != nullptr)
			{
				impl().m_session_wptr = sess;
				return true;
			}
		}
	}
	// get the exist session.
	else if (impl().m_host.m_type == tcp_session_host_client)
	{
		auto* client = (TcpClient::Impl*)impl().m_host.m_host;
		SessionDataPack::ptr pack = client->find_session(session_id);
		if (pack != nullptr)
		{
			impl().m_user = pack->m_user_observer.lock();
			if (impl().m_user != nullptr)
			{
				impl().m_session_wptr = pack->m_session;
				impl().m_session_id = session_id;
				return true;
			}
		}
	}
	else if (impl().m_host.m_type == tcp_session_host_server)
	{
		auto* server = (TcpServer::Impl*)impl().m_host.m_host;
		auto sess = server->find_session(session_id);
		if (sess != nullptr)
		{
			impl().m_session_wptr = sess;
			impl().m_session_id = session_id;
			return true;
		}
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
bool TcpSession::opened() const
{
	return impl().m_session_wptr.expired();
}


////////////////////////////////////////////////////////////////////////////////
void TcpSession::close()
{
	if (impl().m_session_id != none_session)
	{
		if (impl().m_host.m_type == tcp_session_host_client)
		{
			auto* client = (TcpClient::Impl*)impl().m_host.m_host;
			client->erase_session(impl().m_session_id);
		}
		else if (impl().m_host.m_type == tcp_session_host_server)
		{
			auto* server = (TcpServer::Impl*)impl().m_host.m_host;
			server->erase_session(impl().m_session_id);
		}
	}
	impl().m_session_wptr.reset();
	impl().m_host.clear();
}


////////////////////////////////////////////////////////////////////////////////
SessionData::ptr TcpSession::data()
{
	return impl().m_session_wptr.lock();
}
uint32_t TcpSession::get_session_id() const
{
	return impl().m_session_id;
}
TcpPeer& TcpSession::get_peer() const
{
	return *impl().m_host.m_peer;
}


////////////////////////////////////////////////////////////////////////////////
void TcpSession::async_send(IN MessageMeta& meta)
{
	SessionData::ptr lock = impl().m_session_wptr.lock();
	if (lock == nullptr)
	{
		return;
	}

	if (impl().m_host.m_type == tcp_session_host_client)
	{
		auto* client = (TcpClient::Impl*)impl().m_host.m_host;
		meta.set_session_id(impl().m_session_id);
		client->async_send(meta);
	}
	else if (impl().m_host.m_type == tcp_session_host_server)
	{
		auto* server = (TcpServer::Impl*)impl().m_host.m_host;
		meta.set_session_id(impl().m_session_id);
		impl().m_host.m_peer->async_send(meta, *impl().m_prot);
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpSession::async_auth(IN MessageMeta& meta)
{
	SessionData::ptr lock = impl().m_session_wptr.lock();
	if (lock == nullptr)
	{
		return;
	}
	if (impl().m_host.m_type != tcp_session_host_client)
	{
		EcoThrow(e_session_auth_host_client)
			<< "session host who async auth must be client.";
	}

	auto* client = (TcpClient::Impl*)impl().m_host.m_host;
	meta.set_session_id(impl().m_session_id);
	client->async_auth(*this, meta);
}


////////////////////////////////////////////////////////////////////////////////
void TcpSession::async_resp(
	IN Codec& codec,
	IN const uint32_t type,
	IN const Context& c,
	IN const bool last)
{
	MessageMeta meta(codec, impl().m_session_id, type, c.m_meta.m_category);
	meta.set_request_data(c.m_meta.m_request_data, c.m_meta.m_option);
	meta.set_last(last);
	async_send(meta);
}
bool TcpSession::session_mode() const
{
	return impl().m_host.session_mode();
}

////////////////////////////////////////////////////////////////////////////////
}}