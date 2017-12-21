#include "PrecHeader.h"
#include "TcpServer.ipp"
////////////////////////////////////////////////////////////////////////////////
#include <eco/log/Log.h>
#include <eco/service/dev/Cluster.h>
#include <eco/net/protocol/TcpProtocol.h>
#include "TcpPeer.ipp"
#include "TcpSession.ipp"


namespace eco{;
namespace net{;


//##############################################################################
//##############################################################################
inline uint32_t TcpServer::Impl::next_session_id()
{
	{
		eco::Mutex::ScopeLock lock(m_session_map.mutex());
		// reuse session id that has been recycled.
		if (m_left_session_ids.size() > 0)
		{
			uint32_t id = m_left_session_ids.back();
			m_left_session_ids.pop_back();
			return id;
		}
	}

	// session will not reach the "uint32_t" max value.
	return ++m_next_session_id;
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::start()
{
	// set default value.
	if (m_option.get_max_connection_size() == 0)
	{
		m_option.set_max_connection_size(max_conn_size);
	}
	if (m_option.get_max_session_size() == 0)
	{
		m_option.set_max_session_size(max_sess_size);
	}

	// start to receive request.
	m_dispatch.run(m_option.get_business_thread_size());

	// acceptor: start accept client tcp_connection.
	m_peer_set.set_max_connection_size(m_option.get_max_connection_size());
	listen(m_option.get_port(), m_option.get_io_thread_size());

	// tick thread: do some auxiliary work.
	m_timer.set_io_service(*m_acceptor.get_io_service());
	set_tick_timer();

	// register service to router.
	if (strlen(m_option.get_router()) == 0)
	{
		eco::service::dev::get_cluster().register_service(
			m_option.get_name(), m_option.get_router(), m_option.get_port());
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::listen(
	IN  const uint16_t port,
	IN  const uint32_t io_server_size)
{
	m_acceptor.listen(port, io_server_size);

	// accept client tcp_connection.
	async_accept();
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::async_accept()
{
	m_acceptor.async_accept();
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::set_tick_timer()
{
	// there is no need a tick timer.
	if (!m_option.has_tick_timer())
	{
		return;
	}

	// set a tick timer to do some work in regular intervals.
	uint32_t tick_secs = m_option.get_tick_time();
	m_timer.set_timer(tick_secs);
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::stop()
{
	m_acceptor.stop();
	m_dispatch.stop();
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::join()
{
	m_dispatch.join();
	m_acceptor.join();
}


////////////////////////////////////////////////////////////////////////////////
SessionData::ptr TcpServer::Impl::add_session(
	OUT SessionId& sess_id, IN TcpPeer& peer)
{
	// session overloaded.
	if (m_session_map.size() == m_option.get_max_session_size())
	{
		eco::Error e(e_session_over_max_size);
		e << "session has reached the max size: " 
			<< m_option.get_max_connection_size();
		EcoNet(EcoError, peer, "", e);
		return SessionData::ptr();
	}

	// create session: session id and session data.
	sess_id = next_session_id();
	SessionData::ptr new_sess(m_make_session(sess_id));
	m_session_map.set(sess_id, new_sess);
	peer.impl().add_session(sess_id);
	return new_sess;
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_timer(IN const eco::Error* e)
{
	if (e)	// error
	{
		EcoError << "tcp server on timer error: " << EcoFmt(*e);
		return;
	}

	//AfwDebug << "tcp hub on tick timer ..................................";
	m_option.add_tick();

	// heartbeat: global tick.
	if (m_option.get_heartbeat_send_tick() > 0 &&
		m_option.tick_count() % m_option.get_heartbeat_send_tick() == 0)
	{
		send_heartbeat();
	}

	// clean dead peer.
	if (m_option.get_heartbeat_recv_tick() > 0 &&
		m_option.tick_count() % m_option.get_heartbeat_recv_tick() == 0)
	{
		m_peer_set.clean_dead_peer();
	}

	// clean inactive connection.
	if (m_option.get_clean_inactive_peer_tick() > 0 &&
		m_option.tick_count() %
		m_option.get_clean_inactive_peer_tick() == 0)
	{
		m_peer_set.clean_inactive_peer();
	}

	// set next tick on.
	set_tick_timer();
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_accept(IN TcpPeer::ptr& p, IN const eco::Error* e)
{
	if (e != nullptr)
	{
		EcoError << "accept: " << e->what();
		return;
	}
	p->set_connected();
	
	// hub verify the most tcp_connection num.
	if (m_peer_set.add(p))
	{
		// tcp_connection start work and recv request.
		p->async_recv();
	}
	else
	{
		p->close();
	}

	// accept next tcp_connection.
	this->async_accept();
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_read(IN void* peer, IN eco::String& data)
{
	auto* peer_impl = static_cast<TcpPeer::Impl*>(peer);

	// #.parse message head.
	eco::Error e;
	MessageHead head;
	if (!m_prot_head->decode(head, data, e))
	{
		e << " tcp server decode head fail.";
		EcoNet(EcoError, *peer_impl, "on_read", e);
		return;
	}

	// #.get related protocol.
	Protocol* prot = nullptr;
	if (!eco::has(head.m_category, category_heartbeat))
	{
		prot = protocol(head.m_version);
		if (prot == nullptr)
		{
			e.id(e_message_unknown) << "tcp server have no protocol: "
				<< head.m_version;
			EcoNet(EcoError, *peer_impl, "on_read", e);
			return;
		}
	}
	
	// #.send heartbeat.
	if (eco::has(head.m_category, category_heartbeat) &&
		m_option.io_heartbeat() && m_option.response_heartbeat())
	{
		peer_impl->async_send_heartbeat(*m_prot_head);
		return;
	}

	// #.dispatch data context.
	TcpSessionHost host(*m_server);
	eco::net::DataContext dc(&host);
	peer_impl->get_data_context(dc, head.m_category, data, *prot, *m_prot_head);
	m_dispatch.post(dc);
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_close(IN uint64_t conn_id)
{
	m_peer_set.del(conn_id);
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_send(
	OUT void* peer,
	IN  const uint32_t send_size)
{
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::send_heartbeat()
{
	if (m_option.rhythm_heartbeat())
	{
		m_peer_set.send_rhythm_heartbeat(*m_prot_head);
	}
	else
	{
		m_peer_set.send_live_heartbeat(*m_prot_head);
	}
}


//##############################################################################
//##############################################################################
ECO_SHARED_IMPL(TcpServer);
ECO_PROPERTY_VAL_IMPL(TcpServer, TcpServerOption, option);
void TcpServer::set_session_data(IN MakeSessionDataFunc make)
{
	impl().set_session_data(make);
}
void TcpServer::start()
{
	impl().start();
}

void TcpServer::stop()
{
	impl().stop();
}

void TcpServer::join()
{
	impl().join();
}

void TcpServer::set_protocol_head(IN ProtocolHead* heap)
{
	impl().set_protocol_head(heap);
}

ProtocolHead& TcpServer::protocol_head() const
{
	return *impl().m_prot_head;
}

void TcpServer::register_protocol(IN Protocol* p)
{
	impl().register_protocol(p);
}

DispatchRegistry& TcpServer::dispatcher()
{
	return impl().m_dispatch;
}

////////////////////////////////////////////////////////////////////////////////
}}