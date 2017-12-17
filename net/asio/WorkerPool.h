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
#include <eco/Project.h>
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
	inline void run(IN size_t io_thread_size)
	{
		m_balancer.reserve(io_thread_size);
		m_tcp_workers.reserve(io_thread_size);
		for (size_t i = 0; i < io_thread_size; ++i)
		{
			TcpWorkerPtr tcp_worker(new Worker);
			m_tcp_workers.push_back(tcp_worker);
			m_balancer.push_back(WorkLoad(tcp_worker.get(), 0));
			tcp_worker->run();
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

	inline boost::asio::io_service* get_io_service()
	{
		// get the io service that has less connections and less workload.
		size_t max_size = 0;
		auto it_get = m_balancer.begin();
		for (auto it = it_get; it != m_balancer.end(); ++it)
		{
			if (it->m_work_load > max_size)
			{
				max_size = it->m_work_load;
				it_get = it;
			}
		}

		// connect on the io_service increase
		++it_get->m_work_load;
		return it_get->m_worker->get_io_service();
	}
};


////////////////////////////////////////////////////////////////////////////////
}}}
#endif