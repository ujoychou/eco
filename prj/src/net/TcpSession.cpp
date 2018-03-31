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
bool TcpSessionInner::auth()
{
	// note: client open a none session is invalid.
	if (impl().m_session_id == none_session && impl().m_owner.m_server)
	{
		// create new session in server.
		auto* srv = (TcpServer::Impl*)impl().m_owner.m_owner;
		auto sess = srv->add_session(impl().m_session_id, impl().m_conn);
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
			server->erase_session(impl().m_session_id, impl().m_conn.get_id());
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
void TcpSessionInner::async_auth(IN const MessageMeta& meta_v)
{
	SessionData::ptr sess = impl().m_session_wptr.lock();
	if (sess != nullptr && !impl().m_owner.m_server)
	{
		MessageMeta& meta = const_cast<MessageMeta&>(meta_v);
		auto* client = (TcpClient::Impl*)impl().m_owner.m_owner;
		client->async_auth(*m_impl, meta);
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpSessionInner::async_response(
	IN Codec& codec,
	IN const uint32_t type,
	IN const Context& c,
	IN const bool last,
	IN const bool encrypted)
{
	if (impl().m_session_id != none_session)
	{
		SessionData::ptr sess = impl().m_session_wptr.lock();
		if (sess != nullptr)
		{
			return impl().m_conn.async_response(codec, type, c, last, encrypted);
		}
	}
	else
	{
		impl().m_conn.async_response(codec, type, c, last, encrypted);
	}
}


////////////////////////////////////////////////////////////////////////////////
}}