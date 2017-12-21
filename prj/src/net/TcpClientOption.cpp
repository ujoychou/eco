#include "PrecHeader.h"
#include <eco/net/TcpClientOption.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class TcpClientOption::Impl
{
public:
	std::string m_name;

	// server tick time.
	// tick unit: seconds.
	uint32_t m_tick_time;
	uint32_t m_tick_count;

	// heartbeat send and recv ticks.
	uint32_t m_heartbeat_send_tick;
	uint32_t m_heartbeat_recv_tick;

	// peer management tick.
	uint32_t m_clean_inactive_peer_tick;
	uint32_t m_auto_reconnect_tick;

	// send heartbeat mode.
	uint16_t m_rhythm_heartbeat;
	uint16_t m_response_heartbeat;

	// option.
	uint16_t m_no_delay;
	uint16_t m_session_mode;

	// io thread and business thread.
	uint16_t m_io_thread_size;
	uint16_t m_business_thread_size;

public:
	// constructor.
	void init(IN TcpClientOption&)
	{
		m_tick_time = 0;
		m_tick_count = 0;

		// heartbeat send and recv ticks.
		m_heartbeat_send_tick = 0;
		m_heartbeat_recv_tick = 0;

		// send heartbeat mode.
		m_rhythm_heartbeat = true;
		m_response_heartbeat = false;

		// peer management tick.
		m_clean_inactive_peer_tick = 0;
		m_auto_reconnect_tick = 0;

		m_io_thread_size = 0;
		m_business_thread_size = 0;

		// option.
		m_no_delay = false;
		m_session_mode = false;

		reset_tick();
	}

	inline void reset_tick()
	{
		m_tick_count = 0;
	}
};
ECO_VALUE_IMPL(TcpClientOption);
ECO_PROPERTY_STR_IMPL(TcpClientOption, name);
ECO_PROPERTY_BOL_IMPL(TcpClientOption, no_delay);
ECO_PROPERTY_BOL_IMPL(TcpClientOption, session_mode);
ECO_PROPERTY_BOL_IMPL(TcpClientOption, rhythm_heartbeat);
ECO_PROPERTY_BOL_IMPL(TcpClientOption, response_heartbeat);
ECO_PROPERTY_VAL_IMPL(TcpClientOption, uint32_t, tick_time);
ECO_PROPERTY_VAL_IMPL(TcpClientOption, uint32_t, heartbeat_send_tick);
ECO_PROPERTY_VAL_IMPL(TcpClientOption, uint32_t, heartbeat_recv_tick);
ECO_PROPERTY_VAL_IMPL(TcpClientOption, uint32_t, clean_inactive_peer_tick);
ECO_PROPERTY_VAL_IMPL(TcpClientOption, uint32_t, auto_reconnect_tick);
ECO_PROPERTY_VAL_IMPL(TcpClientOption, uint16_t, io_thread_size);
ECO_PROPERTY_VAL_IMPL(TcpClientOption, uint16_t, business_thread_size);



////////////////////////////////////////////////////////////////////////////////
void TcpClientOption::reset_tick()
{
	m_impl->reset_tick();
}
void TcpClientOption::add_tick()
{
	++m_impl->m_tick_count;
}
uint32_t TcpClientOption::tick_count() const
{
	return m_impl->m_tick_count;
}
bool TcpClientOption::has_tick_timer() const
{
	return m_impl->m_tick_time > 0;
}


////////////////////////////////////////////////////////////////////////////////
}}