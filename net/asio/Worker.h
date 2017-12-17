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
#include <eco/Project.h>
#include <eco/thread/Thread.h>
#include <boost/asio/io_service.hpp>
#include <map>



namespace eco{;
namespace net{;
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

////////////////////////////////////////////////////////////////////////////////
public:
	// io service run.
	inline void run()
	{
		using namespace boost::asio;
		typedef void (io_service::*RunFunc)();
	
		m_io_service.reset(new boost::asio::io_service());
		m_work.reset(new io_service::work(*m_io_service));
		std::size_t(io_service::*func)() = &io_service::run;
		m_thread.run(std::bind((RunFunc)func, m_io_service.get()));
	}

	inline void join()
	{
		m_thread.join();
	}

	inline void stop()
	{
		// stop io service to stop receive request.
		if (m_io_service != nullptr)
		{
			m_io_service->stop();
		}

		// wait to handle all request left.
		join();

		m_work.reset();
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