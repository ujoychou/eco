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
#include "TcpPeer.ipp"


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class TcpPeerSet : public eco::Object<TcpPeerSet>
{
private:
	typedef std::unordered_map<int64_t, TcpPeer::ptr> TcpPeerMap;

	// max connection control.
	uint32_t m_max_conn_size;

	// connection pool.
	TcpPeerMap m_peer_map;
	eco::Mutex m_peer_map_mutex;

public:
	// constructor.
	inline TcpPeerSet(IN const uint32_t max_conn_size = 1024)
	{
		set_max_connection_size(max_conn_size);
	}

	// disconnect and clear all client peer.
	inline void clear()
	{
		m_peer_map.clear();
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

	/*@ verify whether the connection can be added to connection manager, and
	add it as it's valid.
	* @ para.peer: the connection to be added.
	*/
	inline bool add(IN TcpPeer::ptr& p)
	{
		eco::Mutex::ScopeLock lock(m_peer_map_mutex);
		// connection set is full.
		if (m_peer_map.size() > m_max_conn_size)
		{
			EcoError << "connections has reached max size: " << m_max_conn_size;
			return false;
		}
		// add to connection set.
		m_peer_map[p->get_id()] = p;
		return true;
	}

	/*@ remove connection from connection set.
	* @ para.peer: the connection to be removed.
	*/
	inline void erase(IN int64_t conn_id)
	{
		eco::Mutex::ScopeLock lock(m_peer_map_mutex);
		// find connnection and remove.
		auto it = m_peer_map.find(conn_id);
		if (it != m_peer_map.end())
		{
			EcoDebug << NetLog(conn_id, ECO_FUNC) <= it->second.use_count();
			m_peer_map.erase(it);
		}
	}

	/*@ send heartbeat to all connection in regular intervals.*/
	inline void send_rhythm_heartbeat(IN ProtocolHead& prot_head)
	{
		eco::Mutex::ScopeLock lock(m_peer_map_mutex);
		for (auto it = m_peer_map.begin(); it != m_peer_map.end(); ++it)
		{
			it->second->impl().async_send_heartbeat(prot_head);
		}
	}

	/*@ send heartbeat to all inactive connections.*/
	inline void send_live_heartbeat(IN ProtocolHead& prot_head)
	{
		eco::Mutex::ScopeLock lock(m_peer_map_mutex);
		for (auto it = m_peer_map.begin(); it != m_peer_map.end(); ++it)
		{
			it->second->impl().async_send_live_heartbeat(prot_head);
		}
	}

	/*@ clean the dead peer who has not been send heartbeat to me.*/
	inline void clean_dead_peer()
	{
		eco::Mutex::ScopeLock lock(m_peer_map_mutex);
		for (auto it = m_peer_map.begin(); it != m_peer_map.end(); )
		{
			if (it->second->get_state().peer_live())
			{
				it->second->state().set_peer_live(false);
				++it;
			}
			else
			{
				// 1.close state;2.close socket.3.remove.
				EcoDebug << NetLog(it->first, ECO_FUNC)
					<= it->second.use_count();
				it->second->close();
				it = m_peer_map.erase(it);
			}
		}// end for
	}

	/*@ clean the inactive peer who has not been send request to me.*/
	inline void clean_inactive_peer()
	{
		eco::Mutex::ScopeLock lock(m_peer_map_mutex);
		for (auto it = m_peer_map.begin(); it != m_peer_map.end(); )
		{
			if (it->second->get_state().peer_active())
			{
				it->second->state().set_peer_active(false);
				++it;
			}
			else
			{
				// 1.close state;2.close socket.3.remove.
				EcoInfo << NetLog(it->first, ECO_FUNC)
					<= it->second.use_count();
				it->second->close();
				it = m_peer_map.erase(it);
			}
		}// end for
	}

	/*@ send heartbeat to all inactive connections.*/
	inline void send_test()
	{
		eco::Mutex::ScopeLock lock(m_peer_map_mutex);
		for (auto it = m_peer_map.begin(); it != m_peer_map.end(); ++it)
		{
			it->second->impl().async_send(
				eco::String("hello ujoychou, this is a test message."), 0);
		}
	}
};


}}
////////////////////////////////////////////////////////////////////////////////
#endif