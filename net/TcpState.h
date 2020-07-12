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
#include <eco/thread/State.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpState
{
public:
	/*
	1.socket mode: closed->connected->ready->live\closed.
	2.websocket: closed->conneted->ready(websocket)->live\closed.
	*/
	// constructor.
	inline TcpState() : m_state(eco::atomic::State::_no)
	{}

	// set server state.
	inline void set_server()
	{
		m_state.add(eco::atomic::State::_a);
	}
	// whether it is a server.
	inline bool server() const
	{
		return m_state.has(eco::atomic::State::_a);
	}

	// set ready state: use in websocket.
	inline void set_websocket()
	{
		m_state.add(eco::atomic::State::_b);
	}
	// whether it is ready state.
	inline bool websocket() const
	{
		return m_state.has(eco::atomic::State::_b);
	}

public:
	// set connected state.
	inline void set_connected(bool is = true)
	{
		m_state.del_high();
		m_state.set(is, eco::atomic::State::_aa);
	}
	// whether it is connected state.
	inline bool connected() const
	{
		return m_state.has(eco::atomic::State::_aa);
	}

	// set close state, and this is a prepare state.
	inline void set_closed()
	{
		set_connected(false);
	}
	// whether it is close state.
	inline bool closed() const
	{
		return !connected();
	}

	// set ready state: use in websocket, and websocket connected.
	inline void set_ready()
	{
		m_state.add(eco::atomic::State::_bb);
	}
	// whether it is ready state.
	inline bool ready() const
	{
		return m_state.has(eco::atomic::State::_bb);
	}

	// set valid state, which mean it isn't a dos peer.
	inline void set_valid(bool is = true)
	{
		m_state.set(is, eco::atomic::State::_cc);
	}
	// whether it is valid state.
	inline bool valid() const
	{
		return m_state.has(eco::atomic::State::_cc);
	}

	// whether this connection is alive, this is help to avoid to send heartbeat
	// to a live connection, that will improve performance.
	inline void set_self_live(IN bool is)
	{
		m_state.set(is, eco::atomic::State::_dd);
	}
	inline bool self_live() const
	{
		return m_state.has(eco::atomic::State::_dd);
	}

	// whether peer of this connection is alive, is help to clean dead peer.
	inline void set_peer_live(IN bool is)
	{
		if (!is)
		{
			m_state.del(eco::atomic::State::_ee);
			set_peer_active(false);
			return;
		}
		m_state.add(eco::atomic::State::_ee);
	}
	inline bool peer_live() const
	{
		return m_state.has(eco::atomic::State::_ee);
	}

	// whether peer of this connection is active, is help to clean unactive
	// peer.
	inline void set_peer_active(IN bool is)
	{
		if (is)
		{
			m_state.add(eco::atomic::State::_ff);
			set_peer_live(true);
			return;
		}
		m_state.del(eco::atomic::State::_ff);
	}
	inline bool peer_active() const
	{
		return m_state.has(eco::atomic::State::_ff);
	}

private:
	eco::atomic::State m_state;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif