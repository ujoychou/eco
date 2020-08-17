#ifndef ECO_NET_TCP_SERVER_PEER_SET_H
#define ECO_NET_TCP_SERVER_PEER_SET_H
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
#include <eco/thread/Mutex.h>
#include <eco/net/Log.h>
#include <unordered_map>
#include <mutex>
#include "TcpPeer.ipp"


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpPeerSet : public eco::Object<TcpPeerSet>
{
private:
	typedef std::unordered_map<int64_t, TcpPeer::ptr> TcpPeerMap;
	typedef std::unordered_map<int64_t, int64_t> TcpPeerDos;

	// max connection control.
	uint32_t m_max_conn_size;

	// connection pool.
	TcpPeerMap m_peer_map;
	TcpPeerDos m_peer_dos;
	std::vector<TcpPeer::ptr> m_clean_set;
	mutable std::mutex m_peer_map_mutex;

public:
	// constructor.
	inline TcpPeerSet(IN const uint32_t max_conn_size = 1024)
	{
		set_max_connection_size(max_conn_size);
	}

	// disconnect and clear all client peer.
	inline void clear()
	{
		std::lock_guard<std::mutex> lock(m_peer_map_mutex);
		m_peer_map.clear();
		m_peer_dos.clear();
	}

	/*@ set max connection number.
	* @ para.max_conn_size: this server suport how many connections.
	*/
	inline void set_max_connection_size(IN const uint32_t max_conn_size)
	{
		m_max_conn_size = max_conn_size;
	}
	inline uint32_t get_max_connection_size() const
	{
		return m_max_conn_size;
	}

	// add accepted peer.
	inline bool accept(IN TcpPeer::ptr& p, int64_t ts)
	{
		size_t psize = 0, dsize = 0;
		{
			std::lock_guard<std::mutex> lock(m_peer_map_mutex);
			if (m_peer_map.size() > m_max_conn_size)
			{
				ECO_ERROR << "over max conn size:" <= m_max_conn_size
					<= p->impl().ip() <= p->impl().port();
				return false;
			}
			// add to connection set.
			m_peer_map[p->impl().id()] = p;
			m_peer_dos[p->impl().id()] = ts;
			psize = m_peer_map.size();
			dsize = m_peer_dos.size();
		}
		ECO_INFO << "accept "< p->impl().id() <= psize <= dsize
			<= p->impl().ip() <= p->impl().port();
		return true;
	}

	/*@ tcp peer confirm as a validate peer. remove from dos peers.
	*/
	inline void set_valid_peer(IN int64_t conn_id)
	{
		std::lock_guard<std::mutex> lock(m_peer_map_mutex);
		m_peer_dos.erase(conn_id);
	}

	/*@ remove connection from connection set.
	* @ para.peer: the connection to be removed.
	*/
	inline void erase(IN int64_t conn_id)
	{
		std::lock_guard<std::mutex> lock(m_peer_map_mutex);
		// find connnection and remove.
		auto it = m_peer_map.find(conn_id);
		if (it != m_peer_map.end())
		{
			ECO_DEBUG << "erase " << conn_id <= it->second.use_count();
			m_peer_map.erase(it);
			m_peer_dos.erase(conn_id);
		}
	}

	// get current connection size.
	inline size_t get_connection_size() const
	{
		std::lock_guard<std::mutex> lock(m_peer_map_mutex);
		return m_peer_map.size();
	}

	/*@ send heartbeat to all connection in regular intervals.*/
	inline void send_rhythm_heartbeat()
	{
		std::lock_guard<std::mutex> lock(m_peer_map_mutex);
		for (auto it = m_peer_map.begin(); it != m_peer_map.end(); ++it)
		{
			it->second->impl().send_heartbeat();
		}
	}

	/*@ send heartbeat to all inactive connections.*/
	inline void send_live_heartbeat()
	{
		std::lock_guard<std::mutex> lock(m_peer_map_mutex);
		for (auto it = m_peer_map.begin(); it != m_peer_map.end(); ++it)
		{
			it->second->impl().send_live_heartbeat();
		}
	}

	/*@ clean the dead peer who has not been send heartbeat to me.*/
	inline void clean_dead_peer()
	{
		{
			std::lock_guard<std::mutex> lock(m_peer_map_mutex);
			for (auto it = m_peer_map.begin(); it != m_peer_map.end(); )
			{
				auto& peer_impl = it->second->impl();
				if (!peer_impl.check_peer_live())
				{
					m_clean_set.push_back(it->second);
					it = m_peer_map.erase(it);
					continue;
				}
				++it;
			}
		}
		// don't close this dead peer in "lock(m_peer_map_mutex)", this would
		// easily cause dead lock.
		for (size_t i = 0; i < m_clean_set.size(); ++i)
		{
			auto& it = m_clean_set[i];
			eco::Error e(e_peer_lost, "peer lost client heartbeat.");
			ECO_FUNC(warn) < it->impl().id() <= it.use_count() <= e;
			it->impl().close_and_notify(e, false);
		}
		m_clean_set.clear();
	}

	/*@ clean the dos peer.*/
	inline void clean_dos_peer(uint64_t curr, int timeout)
	{
		{
			std::lock_guard<std::mutex> lock(m_peer_map_mutex);
			for (auto it = m_peer_dos.begin(); it != m_peer_dos.end(); )
			{
				auto diff = curr - it->second;
				if (diff < timeout) { ++it; continue; }
				auto itp = m_peer_map.find(it->first);
				if (itp != m_peer_map.end())
				{
					m_clean_set.push_back(itp->second);
					m_peer_map.erase(itp);
				}
				it = m_peer_dos.erase(it);
			}
		}
		// don't close this dead peer in "lock(m_peer_map_mutex)", this would
		// easily cause dead lock.
		for (size_t i = 0; i < m_clean_set.size(); ++i)
		{
			auto& it = m_clean_set[i];
			eco::Error e(e_peer_dos_lost, "peer dos lost.");
			ECO_FUNC(error) << it->impl().id() <= timeout <= e;
			it->impl().close_and_notify(e, false);
		}
		m_clean_set.clear();
	}
};


}}
////////////////////////////////////////////////////////////////////////////////
#endif