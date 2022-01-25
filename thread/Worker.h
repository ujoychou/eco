#ifndef ECO_THREAD_WORKER_H
#define ECO_THREAD_WORKER_H
/*******************************************************************************
@ name
message server.

@ function
1.asynchronous handle message.(thread size > 0)
2.synchronous handle message.(thread size = 0)


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/log/Log.h>
#include <eco/thread/Work.h>
#include <eco/thread/Thread.h>
#include <eco/thread/Channel.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<
	typename Message,
	typename Handler = std::function<void (Message&)>,
	typename ThreadPool = eco::ThreadPool,
	typename Channel = eco::Channel<Message>>
class Worker
{
	ECO_OBJECT(Worker);
protected:
	// thread work: handle channel message.
	inline void work()
	{
		while (true)
		{
			// don't declare message before while.
			Message msg;
			size_t queue_size = m_channel.pop(msg);
			// message queue has been closed.
			if (size_t(-1) == queue_size) { break; }

			// handler message.
			eco::work_with_catch_exception([&](){ m_handler(msg); });
		}// end while
	}

public:
	// default thread size = 1.
	inline Worker() : m_thread_size(1) {}

	// stop message server: stop message queue and thread pool.
	inline ~Worker() { stop(); }

	// message handler. 
	template<typename T>
	inline void set_handler(IN T h) { m_handler = h; }
	inline Handler& get_handler() { return m_handler; }
	inline const Handler& handler() const { return m_handler; }

	// server/channel name.
	inline const std::string& name() { return m_channel.name(); }

	// thread size.
	inline uint32_t thread_size() const { return m_thread_size; }

	// message queue capacity.
	inline void set_capacity(IN const uint32_t capacity)
	{
		m_channel.set_capacity(capacity);
	}

	// run thread: with no parameter.
	inline void run(IN const char* name)
	{
		typedef Worker<Message, Handler, ThreadPool, Channel> THIS;
		m_channel.set_name(name);
		m_thread_pool.run(name, m_thread_size, std::bind(&THIS::work, this));
	}

	// run thread: with thread_size.
	inline void run(IN const char* name, IN uint32_t thread_size)
	{
		m_thread_size = (thread_size == 0) ? 1 : thread_size;
		m_channel.open();
		run(name);
	}

	// run thread: with thread_size and handler.
	inline void run(
		IN const char* name, IN uint32_t thread_size, IN Handler&& handler)
	{
		m_handler = handler;
		m_thread_size = (thread_size == 0) ? 1 : thread_size;
		m_channel.open();
		run(name);
	}

	// wait server stop.
	inline void join() { m_thread_pool.join(); }

	// close channel: the message queue.
	inline void close() { m_channel.close(); }

	// stop message server: stop message queue and thread pool.
	inline void stop(bool release = false)
	{
		// stop receive message.
		if (release)
			m_channel.release();
		else
			m_channel.close();
		
		// stop server thread.
		join();
	}

	// get message channel.
	inline Channel& channel() { return m_channel; }
	inline std_mutex& mutex() { return m_channel.mutex(); }

	/*@ post message to message server, using template is for parameter of 
	message and const message.
	*/
	inline void post(IN Message&& msg)
	{
		m_channel.post(msg);
	}
	inline void post(IN const Message& msg)
	{
		m_channel.post(Message(msg));
	}

protected:
	uint32_t	m_thread_size;
	ThreadPool	m_thread_pool;
	Channel		m_channel;
	Handler		m_handler;
};


////////////////////////////////////////////////////////////////////////////////
template<typename Message, typename Handler = std::function<void(Message&)> >
class WorkPool
{
	ECO_OBJECT(WorkPool);
public:
	struct WorkerHandler
	{
		inline WorkerHandler(IN Handler* v = nullptr) : m_pool_handler(v) {}
		inline void operator()(IN Message& msg) { (*m_pool_handler)(msg); }
		Handler* m_pool_handler;
	};

	class Worker : public eco::Worker<Message, WorkerHandler, eco::Thread>
	{
		ECO_OBJECT(Worker);
	public:
		inline Worker() : work(0) {}
		inline void attach(IN uint32_t v = 1) { work += v; }
		inline void detach(IN uint32_t v = 1) { work -= v; }
		std_atomic_uint32_t work;
	};

public:
	// constructor.
	inline WorkPool() {}

	// handler.
	inline Handler& get_handler() { return m_handler; }
	inline const Handler& handler() const { return m_handler; }

	inline void set_capacity(IN uint32_t capacity)
	{
		for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
		{
			(**it).set_capacity(capacity);
		}
	}

	// run thread: with thread_size.
	inline void run(IN const std::string& name, IN uint32_t thread_size)
	{
		std::string namei;
		m_pool.resize(thread_size == 0 ? 1 : thread_size);
		for (auto i = 0; i != m_pool.size(); ++i)
		{
			namei = name;
			namei += eco::cast(i);
			m_pool[i] = std::make_shared<Worker>();
			m_pool[i]->run(namei.c_str(), 1, WorkerHandler(&m_handler));
		}
	}

	// run thread: with thread_size and handler.
	inline void run(
		IN const char* name, IN uint32_t thread_size, IN Handler&& handler)
	{
		m_handler = handler;
		run(name, thread_size);
	}

	// use "close" to notify all message exit.
	inline void close()
	{
		for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
		{
			(**it).close();
		}
	}

	inline void join()
	{
		for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
		{
			(**it).join();
		}
	}

	// use "stop" to wait all message handled.
	inline void stop(IN bool release = false)
	{
		for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
		{
			(**it).stop(release);
		}
	}

	inline Worker* attach(IN uint32_t work = 1)
	{
		auto it_min = m_pool.begin();
		uint32_t min_work = (**it_min).work;
		for (auto it = it_min; it != m_pool.end(); ++it)
		{
			if (min_work > (**it).work)
			{
				min_work = (**it).work;
				it_min = it;
			}
		}
		(**it_min).attach(work);
		return it_min->get();
	}

private:
	Handler m_handler;
	std::vector<typename Worker::ptr> m_pool;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif