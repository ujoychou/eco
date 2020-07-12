#include "PrecHeader.h"
#include <eco/net/IoTimer.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/asio/IoTimer.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class IoTimer::Impl
{
	ECO_IMPL_INIT(IoTimer);
public:
	asio::IoTimer m_timer;
};


ECO_OBJECT_IMPL(IoTimer);
////////////////////////////////////////////////////////////////////////////////
void IoTimer::register_on_timer(IN OnTimer handler)
{
	impl().m_timer.register_on_timer(handler);
}

void IoTimer::set_io_service(IN IoService& srv)
{
	impl().m_timer.set_io_service(srv);
}

void IoTimer::set_timer(IN uint32_t tick_secs)
{
	impl().m_timer.set_timer(tick_secs);
}

size_t IoTimer::cancel()
{
	return impl().m_timer.cancel();
}

void IoTimer::close()
{
	return impl().m_timer.close();
}


////////////////////////////////////////////////////////////////////////////////
}}