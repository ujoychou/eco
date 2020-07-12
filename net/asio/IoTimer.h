#ifndef ECO_NET_ASIO_IO_TIMER_H
#define ECO_NET_ASIO_IO_TIMER_H
/*******************************************************************************
@ name


@ function


@ exception


@ remark


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Export.h>
#include <eco/net/IoTimer.h>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <mutex>



namespace eco{;
namespace net{;
namespace asio{;


////////////////////////////////////////////////////////////////////////////////
class IoTimer
{
public:
	// register "on_timer" event handler.
	inline void register_on_timer(IN OnTimer v)
	{
		m_on_timer = v;
	}

	// set io service that running timer depends on.
	inline void set_io_service(IN IoService& srv)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		boost::asio::io_service* service = (boost::asio::io_service*)&srv;
		m_tick_timer.reset(new boost::asio::deadline_timer(*service));
	}

	// start timer.
	inline void set_timer(IN uint32_t secs)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_tick_timer.get())
		{
			m_tick_timer->expires_from_now(boost::posix_time::seconds(secs));
			m_tick_timer->async_wait(
				boost::bind(&IoTimer::on_timer, this,
					boost::asio::placeholders::error));
		}
	}

	// cancel timer.
	inline size_t cancel()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_tick_timer.get())
		{
			boost::system::error_code ec;
			return m_tick_timer->cancel(ec);
		}
		return 0;
	}

	// cancel timer.
	inline void close()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tick_timer.reset();
	}

private:
	/*@ when there is a tick timer event.*/
	inline void on_timer(IN const boost::system::error_code& e)
	{
		if (e)
		{
			eco::Error error(e.message(), e.value());
			m_on_timer(&error);
		}
		else
		{
			m_on_timer(nullptr);
		}
	}

	eco::net::OnTimer m_on_timer;
	std::auto_ptr<boost::asio::deadline_timer> m_tick_timer;
	std::mutex m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
}}}
#endif