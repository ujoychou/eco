#include "Pch.h"
#include "TcpServer.ipp"
////////////////////////////////////////////////////////////////////////////////
#include <eco/rx/RxImpl.h>
#include <eco/service/dev/Cluster.h>
#include <eco/net/protocol/TcpProtocol2.h>
#include <net/protocol/WebSocketProtocol.h>
#include <eco/net/ProtobufHandler.h>
#include "TcpPeer.ipp"
#include "TcpOuter.h"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
void test_close_io_service(TcpAcceptor::Impl* impl);
////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::start()
{
	// verify data.
	if (m_option.port() == 0)
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

	// init data load func.
	server().dispatch<eco::net::GetLocaleHandler>();

	// set tcp peer handler.
	m_peer_handler.m_owner = this;
	m_peer_handler.m_option = &m_option;
	m_peer_handler.m_protocol = &m_protocol;
	m_peer_handler.m_websocket_key = websocket_key();
	m_peer_handler.m_on_close = std::bind(
		&TcpServer::Impl::on_close, this, std::placeholders::_1);
	m_peer_handler.m_on_read = std::bind(
		&TcpServer::Impl::on_read, this, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3);
	m_peer_handler.m_on_send = std::bind(
		&TcpServer::Impl::on_send, this, std::placeholders::_1,
		std::placeholders::_2);
	m_peer_handler.m_on_auth = std::bind(
		&TcpServer::Impl::on_auth, this, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3);

	// set default value.
	if (m_option.max_connection_size() == 0)
		m_option.set_max_connection_size(max_conn_size);
	if (m_option.max_session_size() == 0)
		m_option.set_max_session_size(
			m_option.max_connection_size() * 100);
	if (m_option.io_thread_size() == 0)
		m_option.set_io_thread_size(2);
	if (m_option.business_thread_size() == 0)
		m_option.set_business_thread_size(4);

	// start to receive request.
	m_dispatch_pool.set_capacity(m_option.context_capacity());
	m_dispatch_pool.run("business_thread", m_option.business_thread_size());

	// acceptor: start accept client tcp_connection.
	m_acceptor.listen(m_option.port(), m_option.io_thread_size());

	// register service to router.
	if (strlen(m_option.router()) == 0)
	{
		//eco::service::dev::cluster().register_service(
		//	m_option.name(), m_option.router(), m_option.port());
	}

	// logging.
	char log[1024] = { 0 };
	sprintf(log, "\n+{{tcp server %s %d}}\n"
		"-[mode] no_delay(%c), websocket(%c)\n"
		"-[tick] lost client %ds, heartbeat %ds rhythm(%c)\n"
		"-[capacity] %d connections, %d sessions\n"
		"-[parallel] %d io thread, %d business thread\n",
		m_option.name(), m_option.port(),
		eco::yn(m_option.no_delay()), eco::yn(m_option.websocket()),
		m_option.heartbeat_recv_sec(),
		m_option.heartbeat_send_sec(),
		eco::yn(m_option.heartbeat_rhythm()),
		m_option.max_connection_size(),
		m_option.max_session_size(),
		m_option.io_thread_size(), 
		m_option.business_thread_size());
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
	if (m_session_map.size() >= m_option.max_session_size())
	{
		ECO_ERROR << NetLog(conn.id(), __func__)
			<= "session has reached max size: "
			< m_option.max_session_size();
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
void TcpServer::Impl::on_accept(IN TcpPeer::ptr& peer, bool error)
{
	if (error)
	{
		ECO_FUNC(error) << eco::Error();
		return;
	}

	// hub verify the most tcp_connection num.
	peer->impl().on_accept_state(make_session_id());
	peer->impl().set_protocol(m_protocol.protocol_latest());
	if (m_statistics.accept(peer, DeadLockCheck::get_time()))
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
	IN void* impl, IN MessageTcp& head, IN eco::String& data)
{
	auto* peer = static_cast<TcpPeer::Impl*>(impl);

	// #.send heartbeat.
	peer->set_protocol(head.m_protocol);
	if (is_heartbeat(head.m_category))
	{
		m_statistics.on_live(*peer);
		ECO_DEBUG << NetLog(peer->id(), __func__) <= "heartbeat";
		return;
	}

	// #.dispatch data context.
	peer->post(head, data);
}


////////////////////////////////////////////////////////////////////////////////
void TcpServer::Impl::on_close(IN void* impl)
{
	auto* peer = static_cast<TcpPeer::Impl*>(impl);
	clear_conn_session(peer->id());
	ECO_DEBUG << NetLog(peer->id(), __func__);
	m_statistics.on_close(peer->id());
	if (m_on_close) m_on_close(peer->id());
}
void TcpServer::Impl::on_send(IN void* impl, IN uint32_t send_size)
{}
void TcpServer::Impl::on_auth(
	const char* lang, const char* user, TcpPeer::ptr& peer)
{
	m_statistics.to_live(peer);
}


//##############################################################################
//##############################################################################
ECO_SHARED_IMPL(TcpServer);
ECO_PROPERTY_VAL_IMPL(TcpServer, TcpOptionServer, option);
void TcpServer::set_connection_data(IN MakeConnectionData&& make)
{
	impl().m_statistics.set_make_connnection_data(make);
}
void TcpServer::set_session_data(IN MakeSessionData&& make)
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
void TcpServer::register_handler(IN int id, IN HandlerFunc&& hf)
{
	impl().m_dispatch_pool.register_handler(id, std::move(hf));
}
void TcpServer::register_default(IN HandlerFunc&& hf)
{
	impl().m_dispatch_pool.register_default(std::move(hf));
}
void TcpServer::set_event(OnAccept&& on_accept, OnClose&& on_close)
{
	impl().m_on_accept = on_accept;
	impl().m_on_close = on_close;
}
void TcpServer::set_recv_event(OnRecvData&& on_recv, OnDecodeHead&& on_decode)
{
	impl().m_dispatch_pool.get_handler().set_event(std::move(on_recv));
	impl().m_peer_handler.m_on_decode_head = on_decode;
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