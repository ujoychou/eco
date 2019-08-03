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
#include <eco/thread/MessageQueue.h>
#include <eco/thread/ThreadPool.h>
#include <eco/log/Log.h>


namespace eco{;
namespace detail{;
////////////////////////////////////////////////////////////////////////////////
template<
	typename Message,
	typename Handler = std::function<void (Message&)>,
	typename Queue = MessageQueue<Message>>
class MessageServer
{
	ECO_OBJECT(MessageServer);
protected:
	/*@ work thread method.	*/
	virtual void work() = 0;

public:
	typedef MessageServer<Message, Handler, Queue> T;

	inline MessageServer()
	{}

	/*@ stop message server: stop message queue and thread pool. */
	inline ~MessageServer()
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
	inline void run(IN const char* name, IN uint32_t thread_size = 1)
	{
		if (thread_size == 0)
		{
			thread_size = 1;
		}
		m_message_queue.set_name(name);
		m_message_queue.open();
		m_thread_pool.run(std::bind(&T::work, this), thread_size, name);
	}

	/*@ wait server stop.*/
	inline void join()
	{
		m_thread_pool.join();
	}

	/*@ stop message server: stop message queue and thread pool. and this
	function is not thread safe.
	*/
	inline void stop()
	{
		// stop receive message.
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
	inline const Queue& get_queue() const
	{
		return m_message_queue;
	}

	/*@ post message to message server, using template is for parameter of 
	message and const message.
	*/
	inline void post(IN Message& msg)
	{
		m_message_queue.post(msg);
	}
	template<typename UniqueChecker>
	inline void post_unique(IN Message& msg, IN UniqueChecker& unique_check)
	{
		m_message_queue.post_unique(msg, unique_check);
	}

	// post a value just like raw ptr(object*).
	inline void post_value(IN Message msg)
	{
		m_message_queue.post(msg);
	}
	template<typename UniqueChecker>
	inline void post_unique_value(
		IN Message msg, IN UniqueChecker& unique_check)
	{
		m_message_queue.post_unique(msg, unique_check);
	}

protected:
	Queue m_message_queue;
	Handler m_message_handler;
	eco::ThreadPool m_thread_pool;
};
}// ns.detail


////////////////////////////////////////////////////////////////////////////////
template<typename Message, typename Handler = std::function<void(Message&)> >
class MessageServer : public detail::MessageServer<Message, Handler>
{
	ECO_OBJECT(MessageServer);
public:
	inline MessageServer()
	{}

protected:
	/*@ work thread method.	*/
	virtual void work() override
	{
		eco::this_thread::init();

		while (true)
		{
			// don't declare message before while.
			Message msg;
			size_t queue_size = m_message_queue.pop(msg);
			if (-1 == queue_size)
			{
				break;	// message queue has been closed.
			}
			m_flow.count(queue_size, m_message_queue.name(), "o");

			// handler message.
			try
			{
				m_message_handler(msg);
			}
			catch (std::exception& e)
			{
				ECO_LOGX(error, "message server") << e.what();
			}
		}// end while
	}

	MessageQueueFlow m_flow;
};


////////////////////////////////////////////////////////////////////////////////
template<typename Message, typename Handler = std::function<void(Message&)> >
class MessageServerPool
{
	ECO_OBJECT(MessageServerPool);
public:
	class HandlerPtr
	{
	public:
		inline void operator()(IN Message& msg)
		{
			(*m_handler_ptr)(msg);
		}

		inline explicit HandlerPtr(IN Handler* v = nullptr) : m_handler_ptr(v)
		{}

		inline Handler& handler()
		{
			return *m_handler_ptr;
		}

		Handler* m_handler_ptr;
	};
	typedef eco::MessageServer<Message, HandlerPtr> MessageServer;

	class MessageWorker
	{
	public:
		inline MessageWorker() : work(0)
		{}

		inline void init()
		{
			server.reset(new MessageServer());
		}

		inline void attach(IN const uint32_t v = 1)
		{
			work += v;
		}

		inline void detach(IN const uint32_t v = 1)
		{
			work -= v;
		}

		inline void post(IN Message& msg)
		{
			server->post(msg);
		}

		Atomic<uint32_t> work;
		typename MessageServer::ptr server;
	};

public:
	inline MessageServerPool()
	{}

	/*@ message handler. */
	template<typename T>
	inline void set_message_handler(IN T h)
	{
		m_message_handler = h;
	}
	inline Handler& message_handler()
	{
		return m_message_handler;
	}

	inline void run(IN const char* name, IN uint32_t thread_size)
	{
		m_pool.resize(thread_size);
		std::string queue_name;
		for (auto i = 0; i != m_pool.size(); ++i)
		{
			auto* it = &m_pool[i];
			it->init();
			it->server->set_message_handler(HandlerPtr(&m_message_handler));
			queue_name = name;
			queue_name += eco::cast<std::string>(i);
			it->server->run(queue_name.c_str(), 1);
		}
	}

	// use "close" to wait all message handled.
	inline void close()
	{
		for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
		{
			it->server->close();
		}
	}

	inline void release()
	{
		for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
		{
			it->server->release();
		}
	}

	inline void join()
	{
		for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
		{
			it->server->join();
		}
	}

	inline MessageWorker* attach(IN const uint32_t work = 1)
	{
		auto it_min = m_pool.begin();
		uint32_t min_work = it_min->work;
		for (auto it = it_min; it != m_pool.end(); ++it)
		{
			if (min_work > it->work)
			{
				min_work = it->work;
				it_min = it;
			}
		}
		it_min->attach(work);
		return &(*it_min);
	}

private:
	Handler m_message_handler;
	std::vector<MessageWorker> m_pool;
};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif