#include "PrecHeader.h"
#include <eco/log/Log.h>
#include <eco/net/asio/Worker.h>
#include <eco/thread/Timer.h>
#include <eco/thread/Mutex.h>
////////////////////////////////////////////////////////////////////////////////
#include <boost/asio/deadline_timer.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>


ECO_NS_BEGIN(eco);
using namespace boost;
using namespace boost::asio;
typedef std::shared_ptr<deadline_timer> deadline_timer_ptr;
////////////////////////////////////////////////////////////////////////////////
class AsioTimer
{
	ECO_OBJECT(AsioTimer);
public:
	TaskUnit task;
	deadline_timer timer;
	TimerServer::Impl* server;

	inline AsioTimer(io_service& io, TaskUnit& t, TimerServer::Impl* s)
		: timer(io), task(std::move(t)), server(s)
	{}

	virtual void cancel()
	{
		boost::system::error_code ec;
		timer.cancel(ec);
	}
};
class AsioTimerRepeated : public AsioTimer
{
	ECO_OBJECT(AsioTimerRepeated);
public:
	uint32_t canceled;
	eco::Mutex mutex;

	inline AsioTimerRepeated(
		io_service& io, TaskUnit& t, TimerServer::Impl* s)
		: AsioTimer(io, t, s), canceled(0)
	{}

	virtual void cancel() override
	{
		eco::Mutex::ScopeLock lock(mutex);
		canceled = true;
		AsioTimer::cancel();
	}
};


////////////////////////////////////////////////////////////////////////////////
class Timer::Impl
{
	ECO_IMPL_INIT(Timer);
public:
	AsioTimer::wptr m_timer;
};
ECO_VALUE_IMPL(Timer);
void Timer::cancel()
{
	auto timer = impl().m_timer.lock();
	if (timer)
	{
		timer->cancel();
		impl().m_timer.reset();
	}
}
bool Timer::empty()
{
	return impl().m_timer.expired();
}
void Timer::release()
{
	cancel();
}

////////////////////////////////////////////////////////////////////////////////
class TimerServer::Impl
{
	ECO_IMPL_INIT(TimerServer);
public:
	// timer io service.
	net::asio::Worker m_worker;
	std::map<size_t, AsioTimer::ptr> m_timers;

public:
	// timer work.
	inline void start()
	{
		m_worker.run("timer_server");
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
	inline bool execute(TaskUnit& task, const boost::system::error_code& ec)
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
				task();
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
	inline Timer add_task(uint32_t ms, bool repeated, TaskUnit& task)
	{
		Timer obj;	// timer object.
		if (repeated)
		{
			AsioTimerRepeated::ptr ti(new AsioTimerRepeated(
				*m_worker.get_io_service(), task, this));
			ti->timer.expires_from_now(posix_time::milliseconds(ms));
			add_repeated(ms, ti);
			obj.impl().m_timer = ti;
			//m_timers[ti.get()] = ti;
		}
		else
		{
			AsioTimer::ptr ti(new AsioTimer(
				*m_worker.get_io_service(), task, this));
			ti->timer.expires_from_now(posix_time::milliseconds(ms));
			ti->timer.async_wait([=](const system::error_code& ec) mutable {
				execute(ti->task, ec);
			});
			obj.impl().m_timer = ti;
		}
		return obj;
	}
	inline void add_repeated(uint32_t ms, AsioTimerRepeated::ptr& ptr)
	{
		eco::Mutex::ScopeLock lock(ptr->mutex);
		if (!ptr->canceled)
		{
			ptr->timer.async_wait([=](const system::error_code& ec) mutable {
				if (execute(ptr->task, ec)) {
					ptr->timer.expires_at(ptr->timer.expires_at() +
						posix_time::milliseconds(ms));
					add_repeated(ms, ptr);
				}
			});
		}// timer has been cancelled.
	}

public:
	// add dedicated time timer: date time format: 2015-05-21 12:21:12
	inline Timer add_time(std::string& date_time, TaskUnit& t)
	{
		AsioTimer::ptr ti(new AsioTimer(*m_worker.get_io_service(), t, this));
		boost::posix_time::ptime utime;
		get_universal(utime, date_time);
		ti->timer.expires_at(utime);
		ti->timer.async_wait([=](const system::error_code& ec) mutable {
			execute(ti->task, ec);
		});

		Timer obj;
		obj.impl().m_timer = ti;
		return obj;
	}

public:
	// add daily timer that can be repeated.
	inline Timer add_daily(std::string& time, TaskUnit& t)
	{
		AsioTimerRepeated::ptr ti(new AsioTimerRepeated(
			*m_worker.get_io_service(), t, this));
		boost::posix_time::ptime utime;
		get_universal_date_time(utime, time);
		ti->timer.expires_at(utime);
		add_daily_task(ti, utime);
		// timer object.
		Timer obj;
		obj.impl().m_timer = ti;
		return obj;
	}
	inline void add_daily_task(AsioTimerRepeated::ptr& p, posix_time::ptime& t)
	{
		eco::Mutex::ScopeLock lock(p->mutex);
		if (!p->canceled)
		{
			p->timer.async_wait([=](const system::error_code& ec) mutable {
				if (execute(p->task, ec)) {
					t += boost::gregorian::days(1);
					p->timer.expires_at(t);
					add_daily_task(p, t);
				}
			});
		}// timer has been cancelled.
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
ECO_OBJECT_IMPL(TimerServer);
void TimerServer::start()
{
	impl().start();
}
void TimerServer::stop()
{
	impl().stop();
}
void TimerServer::join()
{
	impl().join();
}


////////////////////////////////////////////////////////////////////////////////
Timer TimerServer::add(const char* date_time, Task::ptr& task)
{
	return impl().add_time(std::string(date_time), TaskUnit(task));
}
Timer TimerServer::add(const char* date_time, Closure&& task)
{
	return impl().add_time(std::string(date_time), TaskUnit(task));
}


////////////////////////////////////////////////////////////////////////////////
Timer TimerServer::add(uint32_t millsecs, bool repeated, Task::ptr& task)
{
	return impl().add_task(millsecs, repeated, TaskUnit(task));
}
Timer TimerServer::add(uint32_t millsecs, bool repeated, Closure&& task)
{
	return impl().add_task(millsecs, repeated, TaskUnit(task));
}


////////////////////////////////////////////////////////////////////////////////
Timer TimerServer::add_daily(const char* time, Task::ptr& task)
{
	return impl().add_daily(std::string(time), TaskUnit(task));
}
Timer TimerServer::add_daily(const char* time, Closure&& task)
{
	return impl().add_daily(std::string(time), TaskUnit(task));
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);