#include "PrecHeader.h"
#include <eco/net/TcpSession.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/net/Context.h>
#include "TcpClient.ipp"
#include "TcpServer.ipp"
#include "TcpOuter.h"



////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
bool TcpSessionInner::session_mode() const
{
	assert(impl().m_owner.m_owner != nullptr);
	return (impl().m_owner.m_server)
		? ((TcpServer::Impl*)(impl().m_owner.m_owner))->session_mode()
		: ((TcpClient::Impl*)(impl().m_owner.m_owner))->session_mode();
}


////////////////////////////////////////////////////////////////////////////////
bool TcpSessionInner::auth()
{
	// note: client open a none session is invalid.
	if (impl().m_session_id == none_session && impl().m_owner.m_server)
	{
		// create new session in server.
		auto* srv = (TcpServer::Impl*)impl().m_owner.m_owner;
		auto sess = srv->add_session(impl().m_session_id, *impl().m_peer);
		if (sess != nullptr)
		{
			impl().m_session_wptr = sess;
			return true;
		}
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
void TcpSessionInner::close()
{
	if (impl().m_session_id != none_session)
	{
		if (impl().m_owner.m_server)
		{
			auto* server = (TcpServer::Impl*)impl().m_owner.m_owner;
			server->erase_session(impl().m_session_id);
		}
		else
		{
			auto* client = (TcpClient::Impl*)impl().m_owner.m_owner;
			client->erase_session(impl().m_session_id);
		}
	}
	impl().m_session_wptr.reset();
	impl().m_owner.clear();
}


////////////////////////////////////////////////////////////////////////////////
void TcpSessionInner::async_send(IN MessageMeta& meta)
{
	SessionData::ptr lock = impl().m_session_wptr.lock();
	if (lock == nullptr)
	{
		return;
	}

	if (impl().m_owner.m_server)
	{
		TcpConnectionOuter conn(impl().m_conn);
		auto* server = (TcpServer::Impl*)impl().m_owner.m_owner;
		meta.set_session_id(impl().m_session_id);
		impl().m_peer->async_send(meta, conn.protocol());
	}
	else
	{
		auto* client = (TcpClient::Impl*)impl().m_owner.m_owner;
		meta.set_session_id(impl().m_session_id);
		client->async_send(meta);
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpSessionInner::async_auth(IN MessageMeta& meta)
{
	SessionData::ptr lock = impl().m_session_wptr.lock();
	if (lock == nullptr || impl().m_owner.m_server)
	{
		return;
	}

	auto* client = (TcpClient::Impl*)impl().m_owner.m_owner;
	meta.set_session_id(impl().m_session_id);
	client->async_auth(*m_impl, meta);
}


////////////////////////////////////////////////////////////////////////////////
void TcpSessionInner::async_resp(
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


////////////////////////////////////////////////////////////////////////////////
}}