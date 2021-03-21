#ifndef ECO_THREAD_MESSAGE_SERVER_H
#define ECO_THREAD_MESSAGE_SERVER_H
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
////////////////////////////////////////////////////////////////////////////////
#include <eco/log/Log.h>
#include <eco/thread/Thread.h>
#include <eco/thread/MessageQueue.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<
	typename Message,
	typename Handler = std::function<void (Message&)>,
	typename ThreadPool = eco::ThreadPool,
	typename Queue = MessageQueue<Message>>
class MessageServerT
{
	ECO_OBJECT(MessageServerT);
protected:
	/*@ work thread method.	*/
	virtual void work() = 0;

	template<typename Message>
	inline void work_impl()
	{
		while (true)
		{
			// don't declare message before while.
			Message msg;
			size_t queue_size = m_message_queue.pop(msg);
			if (-1 == queue_size)
			{
				break;	// message queue has been closed.
			}

			// handler message.
			try
			{
				eco::this_thread::lock().set_object(sizeof(msg));
				m_message_handler(msg);
				eco::this_thread::lock().unlock();
			}
			catch (std::exception& e)
			{
				ECO_LOG(error, "message server") << e.what();
			}
		}// end while
	}

public:
	typedef MessageServerT<Message, Handler, ThreadPool, Queue> T;

	inline MessageServerT() : m_thread_size(0)
	{}

	/*@ stop message server: stop message queue and thread pool. */
	inline ~MessageServerT()
	{
		stop();
	}

	/*@ set message handler. */
	template<typename T>
	inline void set_message_handler(IN T h)
	{
		m_message_handler = h;
	}
	/*@ get message handler.*/
	inline Handler& message_handler()
	{
		return m_message_handler;
	}
	inline const Handler& get_message_handler() const
	{
		return m_message_handler;
	}

	/*@ set message queue capacity.*/
	inline void set_capacity(IN const uint32_t capacity)
	{
		m_message_queue.set_capacity(capacity);
	}

	/*@ start message server: start receive message and process it. and this
	function is not thread safe.
	* @ para.thread_size: how many threads to start, if equal to "0", message 
	server will run in a synchronous mode, else it will start number of threads 
	(decicated by "thread_size")and message server run in a asynchronous mode.
	*/
	inline void run(
		IN const char* name, 
		IN uint32_t thread_size = 1,
		IN bool suspend = false)
	{
		m_thread_size = thread_size;
		if (m_thread_size == 0) m_thread_size = 1;
		m_message_queue.set_name(name);
		m_message_queue.open();
		if (suspend) return;
		m_thread_pool.run(std::bind(&T::work, this), m_thread_size, name);
	}

	/*@ resume suspened thread server.
	*/
	inline void resume()
	{
		m_thread_pool.run(std::bind(&T::work, this),
			m_thread_size, m_message_queue.name().c_str());
	}

	/*@ wait server stop.*/
	inline void join()
	{
		m_thread_pool.join();
	}

	/*@ stop message server: stop message queue and thread pool. and this
	function is not thread safe.
	*/
	inline void stop(bool release = false)
	{
		// stop receive message.
		if (release)
			m_message_queue.release();
		else
			m_message_queue.close();
		
		// stop server thread.
		join();
	}

	inline void close()
	{
		m_message_queue.close();
	}

	/*@ release message server: release message queue and thread pool. and this
	function is not thread safe.
	*/
	inline void release()
	{
		m_message_queue.release();

		// stop server thread.
		join();
	}

	// get message queue.
	inline Queue& queue()
	{
		return m_message_queue;
	}
	inline const Queue& get_queue() const
	{
		return m_message_queue;
	}
	inline std_mutex& mutex()
	{
		return m_message_queue.mutex();
	}

	/*@ post message to message server, using template is for parameter of 
	message and const message.
	*/
	inline void post(IN Message&& msg)
	{
		m_message_queue.post(msg);
	}
	inline void post(IN const Message& msg)
	{
		m_message_queue.post(Message(msg));
	}

protected:
	uint32_t	m_thread_size;
	ThreadPool	m_thread_pool;
	Queue		m_message_queue;
	Handler		m_message_handler;
	MessageQueueFlow m_flow;
};


////////////////////////////////////////////////////////////////////////////////
template<typename Message, typename Handler = std::function<void(Message&)>>
class MessagePool : public MessageServerT<Message, Handler, ThreadPool>
{
protected:
	/*@ work thread method.	*/
	virtual void work() override
	{
		work_impl<Message>();
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename Message, typename Handler = std::function<void(Message&)>>
class MessageServer : public MessageServerT<Message, Handler, Thread>
{
protected:
	/*@ work thread method.	*/
	virtual void work() override
	{
		work_impl<Message>();
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename Message, typename Handler = std::function<void(Message&)> >
class MessageServerPool
{
	ECO_OBJECT(MessageServerPool);
public:
	class WorkerHandler
	{
	public:
		inline void operator()(IN Message& msg)
		{
			(*m_pool_handler)(msg);
		}

		inline explicit WorkerHandler(IN Handler* v = nullptr)
			: m_pool_handler(v)
		{}

		inline Handler& handler()
		{
			return *m_pool_handler;
		}

		Handler* m_pool_handler;
	};

	class MessageWorker : public eco::MessageServer<Message, WorkerHandler>
	{
		ECO_OBJECT(MessageWorker);
	public:
		inline MessageWorker() : work(0)
		{}

		inline void attach(IN const uint32_t v = 1)
		{
			work += v;
		}

		inline void detach(IN const uint32_t v = 1)
		{
			work -= v;
		}

		Atomic<uint32_t> work;
	};

public:
	inline MessageServerPool()
	{}

	/*@ message handler. */
	template<typename T>
	inline void set_message_handler(IN T h)
	{
		m_pool_handler = h;
	}
	inline Handler& message_handler()
	{
		return m_pool_handler;
	}
	inline const Handler& get_message_handler() const
	{
		return m_pool_handler;
	}

	inline void set_capacity(uint32_t capacity)
	{
		for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
		{
			(**it).set_capacity(capacity);
		}
	}

	inline void run(IN const char* name, IN uint32_t thread_size)
	{
		m_pool.resize(thread_size);
		std::string queue_name;
		for (auto i = 0; i != m_pool.size(); ++i)
		{
			auto& it = m_pool[i];
			it.reset(new MessageWorker());
			it->set_message_handler(WorkerHandler(&m_pool_handler));
			queue_name = name;
			queue_name += eco::cast<std::string>(i);
			it->run(queue_name.c_str(), 1);
		}
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
	inline void stop()
	{
		for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
		{
			(**it).stop();
		}
	}

	inline MessageWorker* attach(IN const uint32_t work = 1)
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
	Handler m_pool_handler;
	std::vector<typename MessageWorker::ptr> m_pool;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif