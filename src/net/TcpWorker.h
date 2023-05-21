#pragma once
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2023 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include "linux/EpollWorker.h"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class TcpWorkerPool
{
public:
    struct WorkLoad
	{
		TcpWorker*  worker;
		uint32_t	work_load;

		inline WorkLoad(TcpWorker& w) : worker(&w), work_load(0) {}
	};

    // socket workers
	inline void run(const char* name, size_t socket_thread_size)
	{
		this->workers.resize(socket_thread_size);
        this->workloads.resize(socket_thread_size);
		for (size_t i = 0; i < socket_thread_size; ++i)
		{
            std::string name_i = name;
            name_i += eco::cast(i);
			this->workers[i] = std::make_shared<TcpWorker>();
			this->workloads[i] = WorkLoad(*this->workers[i]);
			this->workers[i]->run(name_i.c_str());
		}
	}

	inline void join()
	{
		// Wait for all threads in the pool to exit.
		for (size_t i = 0; i < this->workers.size(); ++i)
		{
			this->workers[i]->join();
		}
	}

	inline void stop()
	{
		// explicitly stop all io_services.
		for (size_t i = 0; i < this->workers.size(); ++i)
		{
			this->workers[i]->async_stop();
		}

		join();
	}

	inline Worker* get_worker()
	{
		// get the io service that has less connections and less workload.
		WorkLoad* get = 0;
		for (WorkLoad& it : this->workloads)
		{
			if (!it.worker->stopped() &&
               (!get || get->work_load > it.work_load))
			{
				get = &it;
			}
		}
		if (!get)
		{
			ECO_FUNC(error) << "all socket workers is stopped.";
		}

		// socket workload increase
		++get->work_load;
		return get->worker;
	}

public:
    inline void stop_test(size_t i)
	{
		if (i < this->workers.size())
		{
			this->workers[i]->async_stop();
			this->workers[i]->join();
		}
	}

private:
    std::vector<WorkLoad> workloads;
    std::vector<TcpWorker::ptr> workers;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)