#include "PrecHeader.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include "TcpSession.ipp"
#include "TcpServer.ipp"
#include "TcpClient.ipp"



////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace net{;
ECO_SHARED_IMPL(TcpSession);
////////////////////////////////////////////////////////////////////////////////
void TcpSession::set_host(IN TcpSessionHost& host)
{
	impl().m_host = host;
}
void TcpSession::set_protocol(IN Protocol& prot)
{
	impl().m_prot = &prot;
}


////////////////////////////////////////////////////////////////////////////////
bool TcpSession::open(IN const SessionId session_id)
{
	// create new session.
	if (session_id == none_session)
	{
		// note: client open a none session is invalid.
		if (impl().m_host.m_type == tcp_session_host_server)
		{
			TcpServer::Impl& server = reinterpret_cast<TcpServer*>(
				impl().m_host.m_host)->impl();
			SessionData::ptr sess = server.add_session(
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
		TcpClient::Impl& client = reinterpret_cast<TcpClient*>(
			impl().m_host.m_host)->impl();
		SessionDataPack::ptr pack = client.find_session(session_id);
		if (pack != nullptr)
		{
			impl().m_user = pack->m_user_observer.lock();
			if (impl().m_user != nullptr)
			{
				impl().m_session_wptr = pack->m_session;
				return true;
			}
		}
	}
	else if (impl().m_host.m_type == tcp_session_host_server)
	{
		TcpServer::Impl& server = reinterpret_cast<TcpServer*>(
			impl().m_host.m_host)->impl();
		auto sess = server.find_session(session_id);
		if (sess != nullptr)
		{
			impl().m_session_wptr = sess;
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
			TcpClient::Impl& client = reinterpret_cast<TcpClient*>(
				impl().m_host.m_host)->impl();
			client.erase_session(impl().m_session_id);
		}
		else if (impl().m_host.m_type == tcp_session_host_server)
		{
			TcpServer::Impl& server = reinterpret_cast<TcpServer*>(
				impl().m_host.m_host)->impl();
			server.erase_session(impl().m_session_id);
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


////////////////////////////////////////////////////////////////////////////////
void TcpSession::async_send(
	IN Codec& codec,
	IN const uint32_t type,
	IN const MessageModel model,
	IN const MessageCategory category)
{
	SessionData::ptr lock = impl().m_session_wptr.lock();
	if (lock == nullptr)
	{
		return;
	}

	if (impl().m_host.m_type == tcp_session_host_client)
	{
		TcpClient* client = reinterpret_cast<TcpClient*>(
			impl().m_host.m_host);
		client->async_send(codec, impl().m_session_id, type, model, category);
	}
	else if (impl().m_host.m_type == tcp_session_host_server)
	{
		TcpServer::Impl& server = reinterpret_cast<TcpServer*>(
			impl().m_host.m_host)->impl();
		MessageMeta meta(codec, impl().m_session_id, type, model);
		impl().m_host.m_peer->async_send(
			meta, *impl().m_prot, *server.m_prot_head);
	}
}


////////////////////////////////////////////////////////////////////////////////
}}