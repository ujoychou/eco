#include "PrecHeader.h"
#include "TcpServer.ipp"
////////////////////////////////////////////////////////////////////////////////
#include <eco/log/Log.h>
#include <eco/service/dev/Cluster.h>
#include <eco/codec/Zlib.h>
#include <eco/codec/ZlibFlate.h>
#include <eco/codec/ZlibFlateUnk.h>
#include <eco/net/protocol/Check.h>
#include <eco/net/protocol/Crypt.h>
#include <eco/net/protocol/WebSocketProtocol.h>
#include "TcpPeer.ipp"
#include "TcpOuter.h"


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::start()
{
	// verify data.
	if (m_option.get_port() == 0)
		ECO_THROW(e_server_no_port) << "server must dedicated server port.";

	// set protocol.
	if (m_option.websocket())
	{
		set_protocol_head(new WebSocketProtocolHeadEx());
		set_protocol(new WebSocketProtocol(false));	 // server must not mask.
	}
	else if (!m_prot_head.get() || m_protocol_set.empty())
	{
		set_protocol_head(new TcpProtocolHead());
		set_protocol(new TcpProtocol(new CheckAdler32(), new CryptFlate()));
	}

	// set default value.
	if (m_option.get_max_connection_size() == 0)
		m_option.set_max_connection_size(max_conn_size);
	if (m_option.get_max_session_size() == 0)
		m_option.set_max_session_size(
			m_option.get_max_connection_size() * 100);
	if (m_option.io_thread_size() == 0)
		m_option.set_io_thread_size(2);
	if (m_option.business_thread_size() == 0)
		m_option.set_business_thread_size(4);

	// start to receive request.
	m_dispatch_pool.run("disp_dc", m_option.get_business_thread_size());

	// acceptor: start accept client tcp_connection.
	m_peer_set.set_max_connection_size(m_option.get_max_connection_size());
	m_acceptor.listen(m_option.get_port(), m_option.get_io_thread_size());

	// tick thread: do some auxiliary work.
	m_timer.set_io_service(*m_acceptor.get_io_service());
	set_tick_timer();

	// register service to router.
	if (strlen(m_option.get_router()) == 0)
	{
		//eco::service::dev::get_cluster().register_service(
		//	m_option.get_name(), m_option.get_router(), m_option.get_port());
	}

	// logging.
	char log[1024] = { 0 };
	sprintf(log, "\n+[tcp server %s %d]\n"
		"-[mode] no_delay(%c), websocket(%c)\n"
		"-[tick] unit %ds, lost client %ds, heartbeat %ds\n"
		"-[beat] io(%c), rhythm(%c), response(%c)\n"
		"-[capacity] %d connections, %d sessions\n"
		"-[parallel] %d io thread, %d business thread\n",
		m_option.get_name(), m_option.get_port(),
		eco::yn(m_option.no_delay()), eco::yn(m_option.websocket()),
		m_option.get_tick_time(),
		m_option.get_heartbeat_recv_tick() * m_option.get_tick_time(),
		m_option.get_heartbeat_send_tick() * m_option.get_tick_time(),
		eco::yn(m_option.io_heartbeat()),
		eco::yn(m_option.rhythm_heartbeat()),
		eco::yn(m_option.response_heartbeat()),
		m_option.get_max_connection_size(),
		m_option.get_max_session_size(),
		m_option.get_io_thread_size(), 
		m_option.get_business_thread_size());
	ECO_LOGX(info) << log;
}


////////////////////////////////////////////////////////////////////////////////
SessionData::ptr TcpServer::Impl::add_session(
	OUT SessionId& sess_id, 
	IN  const TcpConnection& conn)
{
	if (m_make_session == nullptr)
	{
		ECO_WARN << NetLog(conn.get_id(), __func__)
			<= "this is connection mode, don't support session.";
		return SessionData::ptr();
	}
	// session overloaded.
	if (m_session_map.size() >= m_option.get_max_session_size())
	{
		ECO_ERROR << NetLog(conn.get_id(), __func__)
			<= "session has reached max size: "
			< m_option.get_max_session_size();
		return SessionData::ptr();
	}

	// create session: session id and session data.
	sess_id = next_session_id();
	SessionData::ptr new_sess(m_make_session(sess_id, conn));
	m_session_map.set(sess_id, new_sess);
	add_conn_session(conn.get_id(), sess_id);
	ECO_INFO << NetLog(conn.get_id(), __func__, sess_id);
	return new_sess;
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_timer(IN const eco::Error* e)
{
	if (e)	// error
	{
		ECO_ERROR << "tcp server on timer error: " << *e;
		return;
	}
	m_option.step_tick();

	// send rhythm heartbeat.
	if (m_option.get_heartbeat_send_tick() > 0 &&
		m_option.tick_count() % m_option.get_heartbeat_send_tick() == 0)
	{
		async_send_heartbeat();
	}

	// clean dead peer.
	if (m_option.get_heartbeat_recv_tick() > 0 &&
		m_option.tick_count() % m_option.get_heartbeat_recv_tick() == 0)
	{
		m_peer_set.clean_dead_peer();
	}

	// set next tick on.
	set_tick_timer();
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_accept(IN TcpPeer::ptr& p, IN const eco::Error* e)
{
	if (e != nullptr)
	{
		ECO_WARN << "accept: " << e->what();
		return;
	}
	p->set_connected();
	p->state().set_server();
	ECO_INFO << NetLog(p->get_id(), __func__);
	
	// hub verify the most tcp_connection num.
	if (m_peer_set.add(p))
	{
		// tcp_connection start work and recv request.
		if (m_option.websocket())
			p->async_recv_shakehand();
		else
			p->async_recv_by_server();
	}
	else
	{
		p->close();
	}

	// accept next tcp_connection.
	m_acceptor.async_accept();
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_read(IN void* impl, IN eco::String& data)
{
	auto* peer = static_cast<TcpPeer::Impl*>(impl);

	// #.parse message head.
	eco::Error e;
	MessageHead head;
	if (!m_prot_head->decode(head, data, e))
	{
		ECO_INFO << NetLog(peer->get_id(), __func__) <= e;
		return;
	}

	// #.get related protocol and make connection data.
	Protocol* prot = nullptr;
	if (!eco::has(head.m_category, category_heartbeat))
	{
		prot = protocol(head.m_version);
		if (prot == nullptr)
		{
			e.id(e_message_unknown) << "tcp server have no protocol: "
				<< head.m_version;
			ECO_WARN << NetLog(peer->get_id(), __func__) <= e;
			return;
		}
		// this is thread safe:
		// 1)one peer in one thread; 
		// 2)all connection data access after this sentense(create data).
		peer->make_connection_data(m_make_connection, prot);
	}
	
	// #.send heartbeat.
	if (m_option.io_heartbeat() &&
		eco::has(head.m_category, category_heartbeat))
	{
		peer->state().set_peer_live(true);
		if (m_option.response_heartbeat())
			peer->async_send_heartbeat(*m_prot_head);
		ECO_INFO << NetLog(peer->get_id(), __func__) <= "heartbeat";
		return;
	}

	// #.dispatch data context.
	TcpSessionOwner owner(*(TcpServerImpl*)this);
	peer->post(owner, head.m_category, data, prot);
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_close(IN const ConnectionId conn_id)
{
	m_peer_set.erase(conn_id);
	clear_conn_session(conn_id);
	ECO_DEBUG << NetLog(conn_id, __func__);
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_send(
	OUT void* peer,
	IN  const uint32_t send_size)
{
}


//##############################################################################
//##############################################################################
ECO_SHARED_IMPL(TcpServer);
ECO_PROPERTY_VAL_IMPL(TcpServer, TcpServerOption, option);
void TcpServer::set_connection_data(IN MakeConnectionDataFunc make)
{
	impl().m_make_connection = make;
}

void TcpServer::set_session_data(IN MakeSessionDataFunc make)
{
	impl().m_make_session = make;
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

void TcpServer::set_protocol(IN Protocol* p)
{
	impl().register_protocol(p);
}

Protocol* TcpServer::protocol(IN const uint32_t version) const
{
	return impl().protocol(version);
}

void TcpServer::register_handler(IN uint64_t id, IN HandlerFunc hf)
{
	impl().m_dispatch_pool.register_handler(id, hf);
}

void TcpServer::register_default_handler(IN HandlerFunc hf)
{
	impl().m_dispatch_pool.register_default_handler(hf);
}

////////////////////////////////////////////////////////////////////////////////
}}