#ifndef ECO_NET_ASIO_WORKER_POOL_H
#define ECO_NET_ASIO_WORKER_POOL_H
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
#include <eco/net/asio/Worker.h>




namespace eco{;
namespace net{;
namespace asio{;


////////////////////////////////////////////////////////////////////////////////
class WorkerPool : public eco::Object<WorkerPool>
{
////////////////////////////////////////////////////////////////////////////////
private:
	struct WorkLoad
	{
		Worker*		m_worker;
		uint32_t	m_work_load;

		inline WorkLoad(Worker* worker, uint32_t work_load)
			: m_worker(worker), m_work_load(work_load) {}
	};
	typedef std::shared_ptr<boost::asio::io_service> io_service_shared_ptr;
	typedef std::shared_ptr<boost::asio::io_service::work> work_shared_ptr;
	typedef std::shared_ptr<Worker> TcpWorkerPtr;

	std::vector<TcpWorkerPtr> m_tcp_workers;
	std::vector<WorkLoad> m_balancer;

////////////////////////////////////////////////////////////////////////////////
public:
	// io service run.
	inline void run(IN size_t io_thread_size, const char* name)
	{
		std::string xname;
		m_balancer.reserve(io_thread_size);
		m_tcp_workers.reserve(io_thread_size);
		for (size_t i = 0; i < io_thread_size; ++i)
		{
			TcpWorkerPtr tcp_worker(new Worker);
			m_tcp_workers.push_back(tcp_worker);
			xname = name;
			xname += eco::cast<std::string>(i);
			m_balancer.push_back(WorkLoad(tcp_worker.get(), 0));
			tcp_worker->run(xname.c_str());
		}
	}

	inline void join()
	{
		// Wait for all threads in the pool to exit.
		for (size_t i = 0; i < m_tcp_workers.size(); ++i)
		{
			m_tcp_workers[i]->join();
		}
	}

	inline void stop()
	{
		// explicitly stop all io_services.
		for (size_t i = 0; i < m_tcp_workers.size(); ++i)
		{
			m_tcp_workers[i]->async_stop();
		}

		join();
	}

	inline void stop_test(size_t i, bool stop_io_service)
	{
		if (i < m_tcp_workers.size())
		{
			m_tcp_workers[i]->async_stop();
			m_tcp_workers[i]->join();
		}
	}

	inline Worker* get_io_worker()
	{
		// get the io service that has less connections and less workload.
		WorkLoad* get = nullptr;
		for (auto it = m_balancer.begin(); it != m_balancer.end(); ++it)
		{
			WorkLoad& wl = *it;
			if (!wl.m_worker->stopped() &&
				(!get || get->m_work_load > wl.m_work_load))
			{
				get = &wl;
			}
		}
		if (!get)
		{
			ECO_FUNC(error) << "all io workers is stopped.";
		}

		// connect on the io_service increase
		++get->m_work_load;
		return get->m_worker;
	}
};


////////////////////////////////////////////////////////////////////////////////
}}}
#endif