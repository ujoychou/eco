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
#include <eco/Project.h>
#include <eco/net/IoTimer.h>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/deadline_timer.hpp>



namespace eco{;
namespace net{;
namespace asio{;


////////////////////////////////////////////////////////////////////////////////
class IoTimer
{
public:
	IoTimer() : m_io_service(nullptr)
	{}

	// register "on_timer" event handler.
	inline void register_on_timer(IN OnTimer v)
	{
		m_on_timer = v;
	}

	// set io service that running timer depends on.
	inline void set_io_service(IN IoService& srv)
	{
		m_io_service = (boost::asio::io_service*)&srv;
	}

	// start timer.
	inline void set_timer(IN uint32_t tick_secs)
	{
		m_tick_timer.reset(new boost::asio::deadline_timer(
			*m_io_service, boost::posix_time::seconds(tick_secs)));
		m_tick_timer->async_wait(
			boost::bind(&IoTimer::on_timer, this,
			boost::asio::placeholders::error));
	}

	// cancel timer.
	inline size_t cancel()
	{
		if (m_tick_timer != nullptr)
		{
			boost::system::error_code ec;
			return m_tick_timer->cancel(ec);
		}
		return 0;
	}

private:
	/*@ when there is a tick timer event.*/
	inline void on_timer(
		IN const boost::system::error_code& e)
	{
		if (e) {
			eco::Error error(e.message(), e.value());
			m_on_timer(&error);
		}
		else {
			m_on_timer(nullptr);
		}
	}

	eco::net::OnTimer m_on_timer;
	boost::asio::io_service* m_io_service;
	std::shared_ptr<boost::asio::deadline_timer> m_tick_timer;
};


////////////////////////////////////////////////////////////////////////////////
}}}
#endif