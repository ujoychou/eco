#include "PrecHeader.h"
#include <eco/net/IoTimer.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/asio/IoTimer.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class IoTimer::Impl
{
public:
	asio::IoTimer m_timer;

	void init(IoTimer&) {}
};


ECO_MOVABLE_IMPL(IoTimer);
////////////////////////////////////////////////////////////////////////////////
void IoTimer::register_on_timer(IN OnTimer handler)
{
	m_impl->m_timer.register_on_timer(handler);
}

void IoTimer::set_io_service(IN IoService& srv)
{
	m_impl->m_timer.set_io_service(srv);
}

void IoTimer::set_timer(IN uint32_t tick_secs)
{
	m_impl->m_timer.set_timer(tick_secs);
}

size_t IoTimer::cancel()
{
	return m_impl->m_timer.cancel();
}


////////////////////////////////////////////////////////////////////////////////
}}