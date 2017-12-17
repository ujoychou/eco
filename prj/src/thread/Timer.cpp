#include "PrecHeader.h"
#include <eco/net/asio/Worker.h>
#include <eco/thread/Timer.h>
#include <eco/Bobject.h>
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

	// asio timer
	timer_shared_ptr m_timer;

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
		if (m_timer != nullptr)
		{
			boost::system::error_code e;
			m_timer->cancel(e);
			m_timer.reset();
		}
		m_worker.stop();
	}

	/*@ add closure functor timer: date time format: 2015-05-21 12:21:12
	*/
	inline void add_closure_dt_timer(
		IN std::string& date_time,
		IN OnTimerFunc& task)
	{
		boost::posix_time::ptime universal_date_time;
		get_universal(universal_date_time, date_time);
		timer_shared_ptr timer(new boost::asio::deadline_timer(
			*m_worker.get_io_service(), universal_date_time));
		timer->async_wait(
			boost::bind(&Impl::on_closure_dt_timer, this,
			std::move(task), std::move(date_time), timer,
			boost::asio::placeholders::error));
	}
	inline void on_closure_dt_timer(
		IN OnTimerFunc& task,
		IN std::string& date_time,
		IN timer_shared_ptr& timer,
		IN const boost::system::error_code& ec)
	{
		try	{
			if (ec == boost::asio::error::operation_aborted)
				task(true);		// task has been cancelled.
			else if (!ec)
				task(false);
		}
		catch (std::exception& e) {
			e.what();
		}
	}

	/*@ add closure functor timer: date time format: 2015-05-21 12:21:12
	*/
	inline void add_task_dt_timer(
		IN std::string& date_time,
		IN std::shared_ptr<Btask>& task)
	{
		boost::posix_time::ptime universal_date_time;
		get_universal(universal_date_time, date_time);
		timer_shared_ptr timer(new boost::asio::deadline_timer(
			*m_worker.get_io_service(), universal_date_time));
		timer->async_wait(
			boost::bind(&Impl::on_task_dt_timer, this,
			std::move(task), std::move(date_time), timer,
			boost::asio::placeholders::error));
	}
	inline void on_task_dt_timer(
		IN std::shared_ptr<Btask>& task,
		IN std::string& date_time,
		IN timer_shared_ptr& timer,
		IN const boost::system::error_code& ec)
	{
		if (!ec)
			task->start();
	}

	/*@ add closure functor timer. */
	inline TimerObject add_closure_timer(
		IN const uint32_t millsecs,
		IN const bool repeated,
		IN OnTimerFunc& task)
	{
		timer_shared_ptr timer(new boost::asio::deadline_timer(
			*m_worker.get_io_service(),
			boost::posix_time::milliseconds(millsecs)));
		timer->async_wait(
			boost::bind(&Impl::on_closure_timer, this,
			std::move(task), millsecs, repeated, timer,
			boost::asio::placeholders::error));
		TimerObject obj;
		obj.impl().m_timer = timer;
		return obj;
	}
	inline void on_closure_timer(
		IN OnTimerFunc& task,
		IN const uint32_t millsecs,
		IN const bool repeated,
		IN timer_shared_ptr& timer,
		IN const boost::system::error_code& ec)
	{
		try {
			if (ec == boost::asio::error::operation_aborted)
				task(true);		// task has been cancelled.
			else if (!ec)
				task(false);
		}
		catch (std::exception& e) {
			e.what();
		}

		if (repeated)
			add_closure_timer(millsecs, true, task);
	}

	/*@ add task timer.*/
	inline TimerObject add_task_timer(
		IN const uint32_t millsecs,
		IN const bool repeated,
		IN std::shared_ptr<Btask>& task)
	{
		timer_shared_ptr timer(new boost::asio::deadline_timer(
			*m_worker.get_io_service(),
			boost::posix_time::milliseconds(millsecs)));
		timer->async_wait(
			boost::bind(&Impl::on_task_timer, this,
			task, millsecs, repeated, timer,
			boost::asio::placeholders::error));
		TimerObject obj;
		obj.impl().m_timer = timer;
		return obj;
	}
	inline void on_task_timer(
		IN std::shared_ptr<Btask>& task,
		IN const uint32_t millsecs,
		IN const bool repeated,
		IN timer_shared_ptr& timer,
		IN const boost::system::error_code& ec)
	{
		if (!ec)
			task->start();
		if (repeated)
			add_task_timer(millsecs, true, task);
	}

	// add daily timer that can be repeated.
	inline void add_task_daily_timer(
		IN std::string& time,
		IN const bool repeated,
		IN std::shared_ptr<Btask>& task)
	{
		boost::posix_time::ptime universal_date_time;
		get_universal_date_time(universal_date_time, time);
		timer_shared_ptr timer(new boost::asio::deadline_timer(
			*m_worker.get_io_service(), universal_date_time));
		timer->async_wait(
			boost::bind(&Timer::Impl::on_task_daily_timer, this,
			std::move(task), std::move(time), repeated, timer,
			boost::asio::placeholders::error));

	}
	inline void on_task_daily_timer(
		IN std::shared_ptr<Btask>& task,
		IN std::string& time,
		IN const bool repeated,
		IN timer_shared_ptr& timer,
		IN const boost::system::error_code& ec)
	{
		if (!ec)
			task->start();
		if (repeated)
			add_task_daily_timer(time, repeated, task);
	}
	inline void add_closure_daily_timer(
		IN std::string& time,
		IN const bool repeated,
		IN OnTimerFunc& task)
	{
		boost::posix_time::ptime universal_date_time;
		get_universal_date_time(universal_date_time, time);
		timer_shared_ptr timer(new boost::asio::deadline_timer(
			*m_worker.get_io_service(), universal_date_time));
		timer->async_wait(
			boost::bind(&Timer::Impl::on_closure_daily_timer, this,
			std::move(task), std::move(time), repeated, timer,
			boost::asio::placeholders::error));
	}
	inline void on_closure_daily_timer(
		IN OnTimerFunc& task,
		IN std::string& time,
		IN const bool repeated,
		IN timer_shared_ptr& timer,
		IN const boost::system::error_code& ec)
	{
		try	{
			if (ec == boost::asio::error::operation_aborted)
				task(true);		// task has been cancelled.
			else if (!ec)
				task(false);
		}
		catch (std::exception& e) {
			e.what();
		}

		if (repeated)
			add_closure_daily_timer(time, repeated, task);
	}

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
TimerObject Timer::add_timer(
	IN const uint32_t millsecs,
	IN const bool repeated,
	IN const Btask& task)
{
	std::shared_ptr<Btask> task_ptr(task.copy());
	return impl().add_task_timer(millsecs, repeated, task_ptr);
}
TimerObject Timer::add_timer(
	IN const uint32_t millsecs,
	IN const bool repeated,
	IN std::auto_ptr<Btask>& task)
{
	std::shared_ptr<Btask> task_ptr(task.release());
	return impl().add_task_timer(millsecs, repeated, task_ptr);
}
TimerObject Timer::add_timer(
	IN const uint32_t millsecs,
	IN const bool repeated,
	IN OnTimerFunc task)
{
	return impl().add_closure_timer(millsecs, repeated, task);
}


////////////////////////////////////////////////////////////////////////////////
void Timer::add_timer(
	IN const char* date_time,
	IN const Btask& task)
{
	std::shared_ptr<Btask> task_ptr(task.copy());
	std::string date_time_str = date_time;
	impl().add_task_dt_timer(date_time_str, task_ptr);
}
void Timer::add_timer(
	IN const char* date_time,
	IN std::auto_ptr<Btask>& task)
{
	std::shared_ptr<Btask> task_ptr(task.release());
	std::string date_time_str = date_time;
	impl().add_task_dt_timer(date_time_str, task_ptr);
}
void Timer::add_timer(
	IN const char* date_time,
	IN OnTimerFunc task)
{
	std::string dt = date_time;
	impl().add_closure_dt_timer(dt, task);
}


////////////////////////////////////////////////////////////////////////////////
void Timer::add_daily_timer(
	IN const char* time,
	IN const bool  repeated,
	IN const Btask& task)
{
	std::string time_str(time);
	std::shared_ptr<Btask> task_ptr(task.copy());
	impl().add_task_daily_timer(time_str, repeated, task_ptr);
}
void Timer::add_daily_timer(
	IN const char* time,
	IN const bool  repeated,
	IN std::auto_ptr<Btask>& task)
{
	std::string time_str(time);
	std::shared_ptr<Btask> task_ptr(task.release());
	impl().add_task_daily_timer(time_str, repeated, task_ptr);
}
void Timer::add_daily_timer(
	IN const char* time,
	IN const bool  repeated,
	IN OnTimerFunc task)
{
	std::string time_str(time);
	impl().add_closure_daily_timer(time_str, repeated, task);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);