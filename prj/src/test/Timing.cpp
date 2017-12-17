#include "PrecHeader.h"
#include <eco/test/Timing.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>



ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(test);
////////////////////////////////////////////////////////////////////////////////
class Timing::Impl
{
	ECO_IMPL_INIT(Timing);
public:
	boost::posix_time::ptime m_start;
	boost::posix_time::time_duration m_td;
};


ECO_OBJECT_IMPL(Timing);
////////////////////////////////////////////////////////////////////////////////
void Timing::start()
{
	impl().m_start = boost::posix_time::microsec_clock::universal_time();
}
Timing& Timing::timeup()
{
	boost::posix_time::ptime time_end =
		boost::posix_time::microsec_clock::universal_time();
	impl().m_td = time_end - impl().m_start;
	impl().m_start = time_end;
	return *this;
}
int64_t Timing::seconds() const
{
	return impl().m_td.seconds();
}
int64_t Timing::milliseconds() const
{
	return impl().m_td.total_milliseconds();
}
int64_t Timing::microseconds() const
{
	return impl().m_td.total_microseconds();
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(test);
ECO_NS_END(eco);