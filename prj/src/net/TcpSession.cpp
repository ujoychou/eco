#include "Pch.h"
#include <eco/net/TcpSession.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/Context.h>
#include "TcpClient.ipp"
#include "TcpServer.ipp"
#include "TcpOuter.h"



////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
namespace net{;
////////////////////////////////////////////////////////////////////////////////
void TcpSessionInner::close()
{
	if (impl().m_session_id != none_session)
	{
		if (impl().m_owner.m_server)
		{
			auto* server = (TcpServer::Impl*)impl().m_owner.m_owner;
			server->erase_session(impl().m_session_id, impl().m_conn.id());
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
void TcpSessionInner::response(MessageMeta& meta, const Context& c)
{
	if (impl().m_session_id != none_session)
	{
		SessionData::ptr sess = impl().m_session_wptr.lock();
		if (sess) impl().m_conn.response(meta, c);
		return;
	}
	impl().m_conn.response(meta, c);
}


////////////////////////////////////////////////////////////////////////////////
}}