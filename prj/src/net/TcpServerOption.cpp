#include "PrecHeader.h"
#include <eco/net/TcpServerOption.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class TcpServerOption::Impl
{
public:
	std::string m_router;
	std::string m_name;
	uint32_t m_port;
	uint32_t m_max_connection_size;
	uint32_t m_max_session_size;

	// server tick time.
	// tick unit: seconds.
	uint32_t m_tick_time;
	uint32_t m_tick_count;

	// heartbeat send and recv ticks.
	uint32_t m_heartbeat_send_tick;
	uint32_t m_heartbeat_recv_tick;

	// peer management tick.
	uint32_t m_clean_inactive_peer_tick;

	// send heartbeat mode.
	uint16_t m_rhythm_heartbeat;
	uint16_t m_response_heartbeat;

	// peer option.
	uint16_t m_no_delay;
	uint16_t m_io_heartbeat;
	uint16_t m_websocket;

	// io thread and business thread.
	uint16_t m_io_thread_size;
	uint16_t m_business_thread_size;

public:
	// constructor.
	void init(IN TcpServerOption&)
	{
		m_port = 0;
		m_max_session_size = 0;
		m_max_connection_size = 0;
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

		m_io_thread_size = 0;
		m_business_thread_size = 0;

		// option.
		m_no_delay = false;
		m_io_heartbeat = false;
		m_websocket = false;

		reset_tick();
	}

	// destructor.
	inline ~Impl()
	{}

	inline void reset_tick()
	{
		m_tick_count = 0;
	}
};
ECO_VALUE_IMPL(TcpServerOption);
ECO_PROPERTY_STR_IMPL(TcpServerOption, router);
ECO_PROPERTY_STR_IMPL(TcpServerOption, name);
ECO_PROPERTY_BOL_IMPL(TcpServerOption, no_delay);
ECO_PROPERTY_BOL_IMPL(TcpServerOption, websocket);
ECO_PROPERTY_BOL_IMPL(TcpServerOption, io_heartbeat);
ECO_PROPERTY_BOL_IMPL(TcpServerOption, rhythm_heartbeat);
ECO_PROPERTY_BOL_IMPL(TcpServerOption, response_heartbeat);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint32_t, port);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint32_t, tick_time);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint32_t, max_session_size);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint32_t, max_connection_size);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint32_t, heartbeat_send_tick);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint32_t, heartbeat_recv_tick);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint32_t, clean_inactive_peer_tick);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint16_t, io_thread_size);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint16_t, business_thread_size);



////////////////////////////////////////////////////////////////////////////////
void TcpServerOption::reset_tick()
{
	m_impl->reset_tick();
}
void TcpServerOption::step_tick(IN const uint32_t step)
{
	m_impl->m_tick_count += step;
}
uint32_t TcpServerOption::tick_count() const
{
	return m_impl->m_tick_count;
}
bool TcpServerOption::has_tick_timer() const
{
	return m_impl->m_tick_time > 0;
}


////////////////////////////////////////////////////////////////////////////////
}}