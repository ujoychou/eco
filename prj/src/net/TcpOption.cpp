#include "PrecHeader.h"
#include <eco/Implement.h>
#include <eco/net/TcpOption.h>
////////////////////////////////////////////////////////////////////////////////


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpOption::Impl
{
	ECO_IMPL_INIT(TcpOption);
public:
	std::string m_name;
	std::string m_router;
	uint32_t	m_max_byte_size;

	// option.
	uint16_t m_no_delay;
	uint16_t m_websocket;
	// server tick time.
	uint32_t m_tick_time;
	uint32_t m_tick_count;
	uint32_t m_heartbeat_send_tick;
	uint32_t m_heartbeat_recv_tick;
	uint32_t m_send_capacity;
	uint32_t m_send_buffer_size;
	uint32_t m_recv_buffer_size;
	uint32_t m_send_low_watermark;
	uint32_t m_recv_low_watermark;

	// send heartbeat mode.
	uint16_t m_rhythm_heartbeat;
	uint16_t m_response_heartbeat;
	// peer option.
	uint16_t m_io_heartbeat;
	uint32_t m_context_capacity;

	// constructor.
	inline Impl()
	{
		m_no_delay = true;
		m_websocket = false;
		m_tick_time = 5;
		m_heartbeat_send_tick = 3;
		m_heartbeat_recv_tick = 9;
		m_send_capacity = 5000;
		m_context_capacity = 5000;
		m_send_buffer_size = 0;
		m_recv_buffer_size = 0;
		m_send_low_watermark = 0;
		m_recv_low_watermark = 0;
		// send heartbeat mode.
		m_rhythm_heartbeat = false;
		m_response_heartbeat = false;
		m_io_heartbeat = false;
		m_max_byte_size = std::numeric_limits<uint32_t>::max();
		reset_tick();
	}

	inline void reset_tick()
	{
		m_tick_count = 0;
	}
};
ECO_VALUE_IMPL(TcpOption);
ECO_PROPERTY_STR_IMPL(TcpOption, name);
ECO_PROPERTY_STR_IMPL(TcpOption, router);
ECO_PROPERTY_BOL_IMPL(TcpOption, no_delay);
ECO_PROPERTY_BOL_IMPL(TcpOption, websocket);
ECO_PROPERTY_BOL_IMPL(TcpOption, io_heartbeat);
ECO_PROPERTY_BOL_IMPL(TcpOption, rhythm_heartbeat);
ECO_PROPERTY_BOL_IMPL(TcpOption, response_heartbeat);
ECO_PROPERTY_VAV_IMPL(TcpOption, uint32_t, tick_time);
ECO_PROPERTY_VAV_IMPL(TcpOption, uint32_t, send_capacity);
ECO_PROPERTY_VAV_IMPL(TcpOption, uint32_t, context_capacity);
ECO_PROPERTY_VAV_IMPL(TcpOption, uint32_t, send_buffer_size);
ECO_PROPERTY_VAV_IMPL(TcpOption, uint32_t, recv_buffer_size);
ECO_PROPERTY_VAV_IMPL(TcpOption, uint32_t, send_low_watermark);
ECO_PROPERTY_VAV_IMPL(TcpOption, uint32_t, recv_low_watermark);
ECO_PROPERTY_VAV_IMPL(TcpOption, uint32_t, heartbeat_send_tick);
ECO_PROPERTY_VAV_IMPL(TcpOption, uint32_t, heartbeat_recv_tick);
ECO_PROPERTY_VAV_IMPL(TcpOption, uint32_t, max_byte_size);


////////////////////////////////////////////////////////////////////////////////
void TcpOption::reset_tick()
{
	m_impl->reset_tick();
}
void TcpOption::step_tick(IN const uint32_t step)
{
	m_impl->m_tick_count += step;
}
uint32_t TcpOption::tick_count() const
{
	return m_impl->m_tick_count;
}
bool TcpOption::has_tick_timer() const
{
	return m_impl->m_tick_time > 0;
}


//##############################################################################
//##############################################################################
class TcpClientOption::Impl : public TcpOption::Impl
{
	ECO_IMPL_INIT(TcpClientOption);
public:
	uint32_t m_auto_reconnect_tick;

	inline Impl() : TcpOption::Impl()
	{
		m_auto_reconnect_tick = 1;
	}
};
ECO_VALUE_IMPL(TcpClientOption, TcpOption);
ECO_PROPERTY_VAV_IMPL(TcpClientOption, uint32_t, auto_reconnect_tick);


//##############################################################################
//##############################################################################
class TcpServerOption::Impl : public TcpOption::Impl
{
	ECO_IMPL_INIT(TcpServerOption);
public:
	uint32_t m_port;
	uint32_t m_max_connection_size;
	uint32_t m_max_session_size;

	// peer management tick.
	uint32_t m_clean_dos_peer_tick;

	// io thread and business thread.
	uint16_t m_io_thread_size;
	uint16_t m_business_thread_size;

public:
	// constructor.
	inline Impl()
	{
		m_port = 0;
		m_max_session_size = 50000;
		m_max_connection_size = 5000;
		// peer management tick.
		m_clean_dos_peer_tick = 0;
		// server thread mode.
		m_io_thread_size = 2;
		m_business_thread_size = 4;
	}
};
ECO_VALUE_IMPL(TcpServerOption, TcpOption);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint32_t, port);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint32_t, max_session_size);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint32_t, max_connection_size);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint32_t, clean_dos_peer_tick);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint16_t, io_thread_size);
ECO_PROPERTY_VAV_IMPL(TcpServerOption, uint16_t, business_thread_size);
////////////////////////////////////////////////////////////////////////////////
}}