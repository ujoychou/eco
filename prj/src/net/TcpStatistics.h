#ifndef ECO_NET_TCP_STATISTICS_H
#define ECO_NET_TCP_STATISTICS_H
/*******************************************************************************
@ name


@ function


@ exception


@ remark


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <unordered_map>
#include <eco/App.h>
#include <eco/ops/Ops.h>
#include <eco/cpp/Thread.h>
#include <eco/thread/topic/TopicServer.h>
#include "TcpPeer.ipp"


ECO_NS_BEGIN(eco);
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpStatistics : public eco::Object<TcpStatistics>
{
private:
	std_mutex				m_mutex;
	std::unordered_map<uint64_t, TcpPeer::ptr> m_peer_map;

	// tcp server option.
	TcpServerOption&		m_option;
	MakeConnectionData		m_make_conn;

	// session statistic.
	proto::SessionExcept	m_stat_except;
	proto::SessionCount		m_stat_sess;

	// network statistic.
	proto::NetworkCount		m_stat_net;

	// topic server.
	typedef eco::SeqTopic WarningTopic;			// proto::Error.
	typedef eco::OneTopic SessionCountTopic;	// proto::SessionCount.
	typedef eco::SetTopic<uint64_t, proto::Session> SessionTopic;
	typedef eco::SetTopic<uint64_t, proto::NetworkCount> SessionNetworkTopic;
	typedef eco::SetTopic<uint64_t, proto::SessionExcept> SessionExceptTopic;
	eco::TopicServer		m_topic;
	std::unordered_map<uint64_t, proto::Session> m_session_map;
	std::unordered_map<uint64_t, proto::SessionExcept> m_session_except_map;
	std::unordered_map<uint64_t, proto::NetworkCount>  m_session_network_map;

public:
	// constructor.
	inline TcpStatistics(IN TcpServerOption& opt)
		: m_option(opt), m_make_conn(nullptr)
	{}
	inline void set_make_connnection_data(IN MakeConnectionData& make_conn)
	{
		m_make_conn = make_conn;
	}

	// disconnect and clear all client peer.
	inline void clear()
	{
		std_lock_guard lock(m_mutex);
		m_peer_map.clear();
	}

	// add accepted peer: thread safe.
	inline bool accept(IN TcpPeer::ptr& peer, uint64_t ts)
	{
		{
			std_lock_guard lock(m_mutex);
			auto max_siz = m_option.get_max_connection_size();
			if (m_stat_sess.conn_size() >= max_siz)
			{
				ECO_THIS_ERROR(101) << max_siz;
				m_topic.publish<WarningTopic>(ops::topic_warning,
					eco::this_thread::proto::error());
				return false;
			}
			m_stat_sess.set_conn_size(m_stat_sess.conn_size() + 1);
			m_stat_sess.set_ddos_size(m_stat_sess.ddos_size() + 1);
			m_peer_map[peer->id()] = peer;
			ECO_FUNC(key) << peer->id() <= peer->ip()
				<= "conn=" < m_stat_sess.conn_size()
				<= "ddos=" < m_stat_sess.ddos_size();
		}
		
		// create a session info to topic.
		proto::Session sess;
		sess.set_id(peer->id());
		sess.set_ip(peer->ip().c_str());
		sess.set_port(peer->port());
		sess.set_state(peer->state().value());
		sess.set_stamp_open(ts);
		eco::TopicId tid(ops::ttype_session, ops::tprop_session_id, sess.id());
		m_topic.publish<SessionTopic>(tid, sess);

		// add to ddos peer.
		if (m_option.get_clean_dos_peer_sec() > 0)
		{
			peer->impl().m_timer_recv = eco::App::get()->timer().run_after(
				std::bind(&TcpStatistics::on_ddos, this, TcpPeer::wptr(peer)),
				std_chrono::seconds(m_option.get_clean_dos_peer_sec()), false);
		}
		else
		{
			to_live(peer);
		}
		return true;
	}

	// ddos peer trans into live peer.
	inline void to_live(IN TcpPeer::ptr& peer)
	{
		{
			std_lock_guard lock(m_mutex);
			m_stat_sess.set_ddos_size(m_stat_sess.ddos_size() - 1);
		}

		// create connection data.
		if (m_make_conn && !peer->impl().m_data.get())
		{
			peer->impl().make_connection_data(m_make_conn);
		}
		peer->impl().get_state().set_valid(true);

		// cancel ddos timer.
		peer->impl().m_timer_recv.cancel();
		// heartbeat live peer.
		if (m_option.get_heartbeat_recv_sec() > 0)
		{
			peer->impl().m_timer_recv = eco::App::get()->timer().run_after(
				std::bind(&TcpStatistics::on_live_timeout, this,
					TcpPeer::wptr(peer), peer->impl().id()),
				std_chrono::seconds(m_option.get_heartbeat_recv_sec()), false);
		}

		// heartbeat send peer.
		if (m_option.get_heartbeat_send_sec() > 0)
		{
			peer->impl().m_timer_send = eco::App::get()->timer().run_after(
				std::bind(&TcpStatistics::on_send_heartbeat,
					this, TcpPeer::wptr(peer)),
				std_chrono::seconds(m_option.get_heartbeat_send_sec()), true);
		}
	}

	// live peer.
	inline void on_live(IN TcpPeer::Impl& peer, bool active = false)
	{
		if (active)
			peer.get_state().set_peer_active(true);
		else
			peer.get_state().set_peer_live(true);

		if (m_option.heartbeat_recv_sec() > 0)
		{
			std_lock_guard lock(m_mutex);
			peer.m_timer_recv.restart();
		}
	}

public:
	// ddos peer timeout, and close the peer.
	inline void on_ddos(IN TcpPeer::wptr& wp)
	{
		TcpPeer::ptr peer = wp.lock();
		if (peer == nullptr) return;
		peer->impl().close_and_notify();

		std_lock_guard lock(m_mutex);
		m_stat_sess.set_ddos_size(m_stat_sess.ddos_size() - 1);
	}

	// send live heartbeat to remote peer.
	inline void on_send_heartbeat(IN TcpPeer::wptr& wp)
	{
		TcpPeer::ptr peer = wp.lock();
		if (peer == nullptr) return;

		if (m_option.heartbeat_rhythm())
			peer->impl().send_heartbeat();
		else
			peer->impl().send_live_heartbeat();
	}

	// on live timeout.
	inline void on_live_timeout(IN TcpPeer::wptr& wp, IN SessionId id)
	{
		TcpPeer::ptr peer = wp.lock();
		if (peer == nullptr) return;

		ECO_FUNC(warn) < id;
		peer->impl().close_and_notify();

		// except network closed.
		/*proto::SessionExcept except;
		except.set_except_mode(1);
		except.set_except_size();
		except.set_except_total();
		except.set_ip();
		except.set_stamp();
		m_topic.publish<>(except, );
		*/
	}

	// close peer.
	inline void on_close(IN SessionId id)
	{
		std_lock_guard lock(m_mutex);
		m_stat_sess.set_conn_size(m_stat_sess.conn_size() - 1);
		m_peer_map.erase(id);
	}
};


}}
////////////////////////////////////////////////////////////////////////////////
#endif