#include "PrecHeader.h"
#include "TcpServer.ipp"
////////////////////////////////////////////////////////////////////////////////
#include <eco/service/dev/Cluster.h>
#include <eco/net/protocol/TcpProtocol2.h>
#include <eco/net/protocol/WebSocketProtocol.h>
#include "TcpPeer.ipp"
#include "TcpOuter.h"


namespace eco{;
namespace net{;
void test_close_io_service(TcpAcceptor::Impl* impl);
////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::start()
{
	// verify data.
	if (m_option.get_port() == 0)
		ECO_THROW(e_server_no_port) << "server must dedicated server port.";

	// set default protocol.
	if (m_protocol.protocol_latest() == 0)
	{
		if (!m_option.websocket())
		{
			m_protocol.add_protocol(new TcpProtocol());
			m_protocol.add_protocol(new TcpProtocol2());
		}
		else
		{
			// server must not mask.
			m_protocol.add_protocol(new WebSocketProtocol(false));
			m_protocol.add_protocol(new WebSocketProtocol2(false));
		}
	}

	// set tcp peer handler.
	m_peer_handler.m_owner = this;
	m_peer_handler.m_option = &m_option;
	m_peer_handler.m_protocol = &m_protocol;
	m_peer_handler.m_websocket_key = websocket_key();
	m_peer_handler.m_on_close = std::bind(&TcpServer::Impl::on_close, this,
		std::placeholders::_1, std::placeholders::_2);
	m_peer_handler.m_on_read = std::bind(&TcpServer::Impl::on_read, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_peer_handler.m_on_send = std::bind(&TcpServer::Impl::on_send, this,
		std::placeholders::_1, std::placeholders::_2);

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
	m_dispatch_pool.set_capacity(m_option.context_capacity());
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
		"-[mode] no_delay(%c), websocket(%c), send_buff(%d), recv_buff(%d)\n"
		"-[tick] unit %ds, lost client %ds, heartbeat %ds\n"
		"-[beat] io(%c), rhythm(%c), response(%c)\n"
		"-[capacity] %d connections, %d sessions\n"
		"-[parallel] %d io thread, %d business thread\n",
		m_option.get_name(), m_option.get_port(),
		eco::yn(m_option.no_delay()), eco::yn(m_option.websocket()),
		m_option.get_send_buffer_size(), m_option.get_recv_buffer_size(),
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
	ECO_LOG(info) << log;
}


////////////////////////////////////////////////////////////////////////////////
SessionData::ptr TcpServer::Impl::add_session(
	OUT SessionId& sess_id, 
	IN  const TcpConnection& conn)
{
	if (m_make_session == nullptr)
	{
		ECO_WARN << NetLog(conn.id(), __func__)
			<= "this is connection mode, don't support session.";
		return SessionData::ptr();
	}
	// session overloaded.
	if (m_session_map.size() >= m_option.get_max_session_size())
	{
		ECO_ERROR << NetLog(conn.id(), __func__)
			<= "session has reached max size: "
			< m_option.get_max_session_size();
		return SessionData::ptr();
	}

	// create session: session id and session data.
	sess_id = make_session_id();
	SessionData::ptr new_sess(m_make_session(sess_id, conn));
	add_conn_session(conn.id(), sess_id, new_sess);
	ECO_INFO << NetLog(conn.id(), __func__, sess_id);
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
	ThreadCheck::get().set_time();

	// send rhythm heartbeat.
	if (m_option.is_time_to_heartbeat_send_tick())
	{
		async_send_heartbeat();
	}

	// clean dead peer.
	if (m_option.is_time_to_heartbeat_recv_tick())
	{
		m_peer_set.clean_dead_peer();
	}

	if (m_option.get_clean_dos_peer_tick() > 0)
	{
		m_peer_set.clean_dos_peer(
			ThreadCheck::get().get_time(),
			m_option.get_clean_dos_peer_tick());
	}

	/* for testing.
	if (m_option.tick_count() == 2)
	{
		test_close_io_service(&m_acceptor.impl());
	}*/

	// set next tick on.
	set_tick_timer();
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_accept(IN TcpPeer::ptr& peer, IN const eco::Error* e)
{
	if (e != nullptr)
	{
		ECO_FUNC(error) << e->what();
		return;
	}

	// hub verify the most tcp_connection num.
	peer->impl().on_accept_state(make_session_id());
	peer->impl().set_protocol(m_protocol.protocol_latest());
	if (m_peer_set.accept(peer, ThreadCheck::get().get_time()))
	{
		peer->impl().on_accept(m_option);
		if (m_on_accept)
		{
			m_on_accept(peer->id());
		}
	}
	else
	{
		peer->impl().close();
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_read(
	IN void* impl, IN MessageHead& head, IN eco::String& data)
{
	auto* peer = static_cast<TcpPeer::Impl*>(impl);

	// #.send heartbeat.
	peer->set_protocol(head.m_protocol);
	if (is_heartbeat(head.m_category))
	{
		peer->state().set_peer_live(true);
		if (m_option.response_heartbeat())
		{
			peer->send_heartbeat();
		}
		ECO_DEBUG << NetLog(peer->id(), __func__) <= "heartbeat";
		return;
	}

	// #.dispatch data context.
	peer->post(head, data);
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_close(IN SessionId id, IN bool erase_peer)
{
	// when "erase_peer=false" that because of close_and_notify called in
	// peerset, for avoid repeated peer.
	if (erase_peer)
	{
		m_peer_set.erase(id);
	}
	clear_conn_session(id);
	ECO_DEBUG << NetLog(id, __func__);

	if (m_on_close)
	{
		m_on_close(id);
	}
}
void TcpServer::Impl::on_send(IN void* impl, IN uint32_t send_size)
{}


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
uint64_t TcpServer::make_object_id(uint32_t& ts, uint32_t& seq, int ver)
{
	if (ver == 1)
		return eco::date_time::make_id_by_ver1(
			impl().m_option.horizental_number(), ts, seq);
	return 0;
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

////////////////////////////////////////////////////////////////////////////////
void TcpServer::add_protocol(IN Protocol* p)
{
	impl().m_protocol.add_protocol(p);
}
Protocol* TcpServer::protocol(IN int version) const
{
	return impl().m_protocol.protocol(version);
}
Protocol* TcpServer::protocol_latest() const
{
	return impl().m_protocol.protocol_latest();
}

////////////////////////////////////////////////////////////////////////////////
void TcpServer::register_handler(IN uint64_t id, IN HandlerFunc hf)
{
	impl().m_dispatch_pool.register_handler(id, std::move(hf));
}
void TcpServer::register_default(IN HandlerFunc hf)
{
	impl().m_dispatch_pool.register_default(std::move(hf));
}
void TcpServer::set_event(ServerAcceptFunc on_accept, ServerCloseFunc on_close)
{
	impl().m_on_accept = on_accept;
	impl().m_on_close = on_close;
}
void TcpServer::set_recv_event(
	OnRecvDataFunc on_recv, OnDecodeHeadFunc on_decode)
{
	impl().m_peer_handler.m_on_decode_head = on_decode;
	impl().m_dispatch_pool.message_handler().set_event(on_recv);
}
bool TcpServer::receive_mode() const
{
	return impl().receive_mode();
}
bool TcpServer::dispatch_mode() const
{
	return impl().dispatch_mode();
}


////////////////////////////////////////////////////////////////////////////////
}}