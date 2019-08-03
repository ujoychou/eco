#include "PrecHeader.h"
#include <eco/net/asio/Worker.h>
#include <eco/thread/Timer.h>
#include <eco/log/Log.h>
////////////////////////////////////////////////////////////////////////////////
#include <boost/asio/deadline_timer.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
typedef std::shared_ptr<boost::asio::deadline_timer> timer_shared_ptr;
class TimerObject::Impl
{
	ECO_IMPL_INIT(TimerObject);
public:
	timer_shared_ptr m_timer;
};
ECO_VALUE_IMPL(TimerObject);
void TimerObject::cancel()
{
	if (impl().m_timer != nullptr)
	{
		boost::system::error_code ec;
		impl().m_timer->cancel(ec);
	}
}
bool TimerObject::empty()
{
	return impl().m_timer == nullptr;
}
void TimerObject::release()
{
	if (impl().m_timer != nullptr)
	{
		boost::system::error_code ec;
		impl().m_timer->cancel(ec);
		impl().m_timer.reset();
	}
}

////////////////////////////////////////////////////////////////////////////////
class Timer::Impl
{
	ECO_IMPL_INIT(Timer);
public:
	// timer io service.
	net::asio::Worker m_worker;

public:
	// timer work.
	inline void start()
	{
		m_worker.run();
	}

	inline void join()
	{
		m_worker.join();
	}

	inline void stop()
	{
		// first release timer, then release io service, else it will throw
		// exception.
		m_worker.stop();
	}

	// execute task, return false if cancelled.
	inline bool execute(Task::ptr& task, const boost::system::error_code& ec)
	{
		try
		{
			if (ec == boost::asio::error::operation_aborted)
			{
				ECO_FUNC(debug) << "timer canceled";
				return false;
			}
			else if (!ec)
			{
				(*task)();
			}
		}
		catch (std::exception& e)
		{
			ECO_FUNC(error) << e.what();
		}
		return true;
	}

public:
	/*@ add task timer.*/
	inline TimerObject add_task(uint32_t ms, bool repeated, Task::ptr& task)
	{
		timer_shared_ptr timer(new boost::asio::deadline_timer(
			*m_worker.get_io_service(),
			boost::posix_time::milliseconds(ms)));
		timer->async_wait(
			boost::bind(&Impl::on_task, this,
				task, ms, repeated, timer,
				boost::asio::placeholders::error));
		TimerObject obj;
		obj.impl().m_timer = timer;
		return obj;
	}
	inline void on_task(
		IN Task::ptr& task,
		IN uint32_t millsecs,
		IN bool repeated,
		IN timer_shared_ptr& timer,
		IN const boost::system::error_code& ec)
	{
		if (execute(task, ec) && repeated)
			add_task(millsecs, true, task);
	}

public:
	// add dedicated time timer: date time format: 2015-05-21 12:21:12
	inline void add_time(std::string& date_time, Task::ptr& task)
	{
		boost::posix_time::ptime universal_date_time;
		get_universal(universal_date_time, date_time);
		timer_shared_ptr timer(new boost::asio::deadline_timer(
			*m_worker.get_io_service(), universal_date_time));
		timer->async_wait(
			boost::bind(&Impl::on_time, this,
			std::move(task), std::move(date_time), timer,
			boost::asio::placeholders::error));
	}
	inline void on_time(
		IN Task::ptr& task,
		IN std::string& date_time,
		IN timer_shared_ptr& timer,
		IN const boost::system::error_code& ec)
	{
		execute(task, ec);
	}

public:
	// add daily timer that can be repeated.
	inline void add_daily(std::string& time, bool repeated, Task::ptr& task)
	{
		boost::posix_time::ptime universal_date_time;
		get_universal_date_time(universal_date_time, time);
		timer_shared_ptr timer(new boost::asio::deadline_timer(
			*m_worker.get_io_service(), universal_date_time));
		timer->async_wait(
			boost::bind(&Timer::Impl::on_daily, this,
			std::move(task), std::move(time), repeated, timer,
			boost::asio::placeholders::error));
	}
	inline void on_daily(
		IN Task::ptr& task,
		IN std::string& time,
		IN bool repeated,
		IN timer_shared_ptr& timer,
		IN const boost::system::error_code& ec)
	{
		if (execute(task, ec) && repeated)
			add_daily(time, true, task);
	}

public:
	inline void get_universal(
		OUT boost::posix_time::ptime& universal_date_time,
		IN  const std::string& local_date_time_str)
	{
		// get timer universal date time.
		using namespace boost::posix_time;
		using namespace boost::gregorian;
		ptime now_loc = second_clock::local_time();
		ptime now_uni = second_clock::universal_time();
		auto uni_dist = now_uni - now_loc;
		universal_date_time = 
			boost::posix_time::time_from_string(local_date_time_str);
		universal_date_time += uni_dist;
	}
	inline void get_universal_date_time(
		OUT boost::posix_time::ptime& universal_date_time,
		IN  const std::string& local_time_str)
	{
		// get timer universal date time.
		using namespace boost::posix_time;
		using namespace boost::gregorian;
		ptime now_loc = second_clock::local_time();
		ptime now_uni = second_clock::universal_time();
		auto uni_dist = now_uni - now_loc;
		auto date = now_loc.date();
		time_duration now_loc_time = now_loc.time_of_day();
		time_duration loc_time = duration_from_string(local_time_str);
		// second clock precision, use ">=" to avoid add today timer repeatly.
		if (now_loc_time >= loc_time)
		{
			date += boost::gregorian::days(1);
		}
		universal_date_time = ptime(date, loc_time);
		universal_date_time += uni_dist;
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_OBJECT_IMPL(Timer);
void Timer::start()
{
	impl().start();
}
void Timer::stop()
{
	impl().stop();
}
void Timer::join()
{
	impl().join();
}


////////////////////////////////////////////////////////////////////////////////
void Timer::add(const char* date_time, Task::ptr& task)
{
	impl().add_time(std::string(date_time), task);
}
TimerObject Timer::add(uint32_t millsecs, bool repeated, Task::ptr& task)
{
	return impl().add_task(millsecs, repeated, task);
}
void Timer::add_daily(const char* time, bool repeated, Task::ptr& task)
{
	impl().add_daily(std::string(time), repeated, task);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);