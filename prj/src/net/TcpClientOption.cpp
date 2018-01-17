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
	std::string m_service_name;
	// option.
	uint16_t m_no_delay;
	uint16_t m_websocket;
	// server tick time.
	uint32_t m_tick_time;
	uint32_t m_tick_count;
	uint32_t m_heartbeat_send_tick;
	uint32_t m_heartbeat_recv_tick;
	uint32_t m_auto_reconnect_tick;
	
public:
	// constructor.
	void init(IN TcpClientOption&)
	{
		m_no_delay = true;
		m_websocket = false;
		m_tick_time = 5;			// 5 seconds.
		m_tick_count = 0;
		m_heartbeat_send_tick = 0;
		m_heartbeat_recv_tick = 0;
		m_auto_reconnect_tick = 1;	// auto reconnect 5 seconds.
		reset_tick();
	}

	inline void reset_tick()
	{
		m_tick_count = 0;
	}
};


ECO_VALUE_IMPL(TcpClientOption);
ECO_PROPERTY_STR_IMPL(TcpClientOption, service_name);
ECO_PROPERTY_BOL_IMPL(TcpClientOption, no_delay);
ECO_PROPERTY_BOL_IMPL(TcpClientOption, websocket);
ECO_PROPERTY_VAV_IMPL(TcpClientOption, uint32_t, tick_time);
ECO_PROPERTY_VAV_IMPL(TcpClientOption, uint32_t, heartbeat_send_tick);
ECO_PROPERTY_VAV_IMPL(TcpClientOption, uint32_t, heartbeat_recv_tick);
ECO_PROPERTY_VAV_IMPL(TcpClientOption, uint32_t, auto_reconnect_tick);
////////////////////////////////////////////////////////////////////////////////
void TcpClientOption::reset_tick()
{
	m_impl->reset_tick();
}
void TcpClientOption::step_tick(IN const uint32_t step)
{
	m_impl->m_tick_count += step;
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