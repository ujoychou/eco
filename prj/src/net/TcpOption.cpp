#include "Pch.h"
#include <eco/rx/RxImpl.h>
#include <eco/net/TcpOption.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/String.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class TcpOption::Impl
{
	ECO_IMPL_INIT(TcpOption);
public:
	std::string m_name;
	std::string m_router;
	uint32_t	m_max_byte_size;

	// option.
	uint8_t m_no_delay;
	uint8_t m_websocket;
	// server tick time.
	uint32_t m_heartbeat_send_sec;
	uint32_t m_heartbeat_recv_sec;
	uint32_t m_send_capacity;
	uint32_t m_send_buffer_size;
	uint32_t m_recv_buffer_size;
	uint32_t m_send_low_watermark;
	uint32_t m_recv_low_watermark;

	// send heartbeat mode.
	uint16_t m_heartbeat_rhythm;
	// peer option.
	uint32_t m_context_capacity;

	// constructor.
	inline Impl()
	{
		m_no_delay = true;
		m_websocket = false;
		m_heartbeat_send_sec = 15;
		m_heartbeat_recv_sec = 45;
		m_send_capacity = 5000;
		m_context_capacity = 5000;
		m_send_buffer_size = 0;
		m_recv_buffer_size = 0;
		m_send_low_watermark = 0;
		m_recv_low_watermark = 0;
		// send heartbeat mode.
		m_heartbeat_rhythm = false;
		m_max_byte_size = std::numeric_limits<uint32_t>::max();
	}
};
ECO_VALUE_IMPL(TcpOption);
ECO_PROPERTY_STR_IMPL(TcpOption, name);
ECO_PROPERTY_STR_IMPL(TcpOption, router);
ECO_PROPERTY_BOL_IMPL(TcpOption, no_delay);
ECO_PROPERTY_BOL_IMPL(TcpOption, websocket);
ECO_PROPERTY_BOL_IMPL(TcpOption, heartbeat_rhythm);
ECO_PROPERTY_VAR_IMPL(TcpOption, uint32_t, send_capacity);
ECO_PROPERTY_VAR_IMPL(TcpOption, uint32_t, context_capacity);
ECO_PROPERTY_VAR_IMPL(TcpOption, uint32_t, send_buffer_size);
ECO_PROPERTY_VAR_IMPL(TcpOption, uint32_t, recv_buffer_size);
ECO_PROPERTY_VAR_IMPL(TcpOption, uint32_t, send_low_watermark);
ECO_PROPERTY_VAR_IMPL(TcpOption, uint32_t, recv_low_watermark);
ECO_PROPERTY_VAR_IMPL(TcpOption, uint32_t, heartbeat_send_sec);
ECO_PROPERTY_VAR_IMPL(TcpOption, uint32_t, heartbeat_recv_sec);
ECO_PROPERTY_VAR_IMPL(TcpOption, uint32_t, max_byte_size);


//##############################################################################
//##############################################################################
class TcpClientOption::Impl : public TcpOption::Impl
{
	ECO_IMPL_INIT(TcpClientOption);
public:
	std::string m_module_;
	uint16_t m_suspend;
	uint16_t m_auto_reconnect_sec;
	uint16_t m_load_event_sec;
	uint16_t m_balance;

	inline Impl() : TcpOption::Impl()
	{
		m_balance = TcpClientOption::balance_average;
		m_auto_reconnect_sec = 5;
		m_load_event_sec = 5;
		m_suspend = false;
	}
};
ECO_VALUE_IMPL(TcpClientOption, TcpOption);
ECO_PROPERTY_STR_IMPL(TcpClientOption, module_);
ECO_PROPERTY_BOL_IMPL(TcpClientOption, suspend);
ECO_PROPERTY_VAR_IMPL(TcpClientOption, uint16_t, balance);
ECO_PROPERTY_VAR_IMPL(TcpClientOption, uint16_t, load_event_sec);
ECO_PROPERTY_VAR_IMPL(TcpClientOption, uint16_t, auto_reconnect_sec);
void TcpClientOption::set_balance(IN const char* v)
{
	if (eco::iequal(v, "average"))
		impl().m_balance = balance_average;
	else if (eco::iequal(v, "order"))
		impl().m_balance = balance_order;
}


//##############################################################################
//##############################################################################
class TcpServerOption::Impl : public TcpOption::Impl
{
	ECO_IMPL_INIT(TcpServerOption);
public:
	uint32_t m_port;
	uint32_t m_max_session_size;
	uint32_t m_max_connection_size;

	// peer management sec.
	uint32_t m_clean_dos_peer_sec;

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
		m_clean_dos_peer_sec = 0;
		// server thread mode.
		m_io_thread_size = 2;
		m_business_thread_size = 4;
	}
};
ECO_VALUE_IMPL(TcpServerOption, TcpOption);
ECO_PROPERTY_VAR_IMPL(TcpServerOption, uint32_t, port);
ECO_PROPERTY_VAR_IMPL(TcpServerOption, uint32_t, max_session_size);
ECO_PROPERTY_VAR_IMPL(TcpServerOption, uint32_t, max_connection_size);
ECO_PROPERTY_VAR_IMPL(TcpServerOption, uint32_t, clean_dos_peer_sec);
ECO_PROPERTY_VAR_IMPL(TcpServerOption, uint16_t, io_thread_size);
ECO_PROPERTY_VAR_IMPL(TcpServerOption, uint16_t, business_thread_size);
////////////////////////////////////////////////////////////////////////////////
uint32_t TcpServerOption::horizontal_virtual_service_number() const
{
	return 8192;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);