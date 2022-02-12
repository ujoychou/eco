#ifndef ECO_NET_ASIO_WORKER_H
#define ECO_NET_ASIO_WORKER_H
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
#include <eco/rx/RxExport.h>
#include <eco/log/Log.h>
#include <eco/thread/Thread.h>
#include <boost/asio/io_service.hpp>
#include <eco/thread/State.h>
#include <map>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
namespace asio{;
////////////////////////////////////////////////////////////////////////////////
class Worker : public eco::Object<Worker>
{
////////////////////////////////////////////////////////////////////////////////
private:
	typedef std::shared_ptr<boost::asio::io_service> io_service_shared_ptr;
	typedef std::shared_ptr<boost::asio::io_service::work> work_shared_ptr;

	// io_service.
	io_service_shared_ptr m_io_service;

	// the work that keep the io_service running.
	work_shared_ptr m_work;

	// thread to run the io_service.
	eco::Thread m_thread;
	eco::atomic::State m_state;

////////////////////////////////////////////////////////////////////////////////
public:
	inline Worker()
	{
		m_state.add(eco::atomic::State::_a);
		m_io_service.reset(new boost::asio::io_service());
		m_work.reset(new boost::asio::io_service::work(*m_io_service));
	}
	
	// get work stopped state.
	inline bool stopped() const
	{
		return m_state.none();
	}
	inline bool running() const
	{
		return m_state.has(eco::atomic::State::_b);
	}
	inline bool initing() const
	{
		return m_state.has(eco::atomic::State::_a);
	}

	// io service run.
	inline void run(const char* name)
	{
		m_thread.run(name, [=]() {
			boost::system::error_code ec;
			m_io_service->run(ec);
			m_state.set_ok(false);
			if (ec) ECO_ERROR << name <= ec.value() <= ec.message();
		});
		m_state.add(eco::atomic::State::_b);
	}

	inline void join()
	{
		m_thread.join();
	}

	inline void stop()
	{
		// stop io service to stop receive request.
		m_work.reset();
		if (m_io_service != nullptr)
		{
			m_io_service->stop();
		}

		// wait to handle all request left.
		join();
		m_io_service.reset();
	}

	inline void async_stop()
	{
		// stop io service to stop receive request.
		if (m_io_service != nullptr)
		{
			m_io_service->stop();
		}
	}

	inline boost::asio::io_service* get_io_service()
	{
		return m_io_service.get();
	}
};


////////////////////////////////////////////////////////////////////////////////
}}}
#endif