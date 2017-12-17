#ifndef ECO_NET_TCP_STATE_H
#define ECO_NET_TCP_STATE_H
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
#include <eco/Project.h>
#include <eco/thread/State.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpState
{
public:
	// constructor.
	TcpState() : m_state(eco::atomic::State::_a)
	{}

	// set close state, and this is a prepare state.
	inline void close()
	{
		m_state = eco::atomic::State::_a;
	}
	// whether it is close state.
	inline bool is_close() const
	{
		return (m_state == eco::atomic::State::_a);
	}

	// set connected state.
	inline void connected()
	{
		m_state.add(eco::atomic::State::_b);
	}
	// whether it is connected state.
	inline bool is_connected() const
	{
		return (m_state & eco::atomic::State::_b) > 0;
	}

	// whether this connection is alive, this is help to avoid to send heartbeat
	// to a live connection, that will improve performance.
	inline void self_live(IN bool is)
	{
		m_state.set(is, eco::atomic::State::_c);
	}
	inline bool is_self_live() const
	{
		return m_state.has(eco::atomic::State::_c);
	}

	// whether peer of this connection is alive, is help to clean dead peer.
	inline void peer_live(IN bool is)
	{
		if (is) {
			m_state.add(eco::atomic::State::_d);
		}
		else {
			m_state.del(eco::atomic::State::_d);
			m_state.del(eco::atomic::State::_e);
		}
	}
	inline bool is_peer_live() const
	{
		return m_state.has(eco::atomic::State::_d);
	}

	// whether peer of this connection is active, is help to clean unactive
	// peer.
	inline void peer_active(IN bool is)
	{
		if (is) {
			m_state.add(eco::atomic::State::_d);
			m_state.add(eco::atomic::State::_e);
		} else {
			m_state.del(eco::atomic::State::_e);
		}
	}
	inline bool is_peer_active() const
	{
		return m_state.has(eco::atomic::State::_e);
	}

private:
	eco::atomic::State m_state;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif