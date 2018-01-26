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
	typename Queue = MessageQueue<Message> >
class MessageServer
{
	ECO_OBJECT(MessageServer);
public:
	typedef MessageServer<Message, Handler, Queue> MessageServerT;


////////////////////////////////////////////////////////////////////////////////
public:
	MessageServer()
	{}

	/*@ stop message server: stop message queue and thread pool. */
	~MessageServer()
	{
		stop();
	}

	/*@ set message handler. */
	template<typename T>
	void set_message_handler(IN T h)
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
	void run(
		IN uint32_t thread_size = 1,
		IN const char* name = nullptr)
	{
		if (thread_size == 0)
		{
			thread_size = 1;
		}
		m_message_queue.open();
		m_thread_pool.run(std::bind(&MessageServerT::work, this),
			thread_size, name);
	}

	/*@ wait server stop.*/
	inline void join()
	{
		m_thread_pool.join();
	}

	/*@ stop message server: stop message queue and thread pool. and this
	function is not thread safe.
	*/
	void stop()
	{
		// stop receive message.
		m_message_queue.close();
		
		// stop server thread.
		join();
	}

	/*@ release message server: release message queue and thread pool. and this
	function is not thread safe.
	*/
	void release()
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
	template<typename Message>
	inline void post(IN Message& msg)
	{
		m_message_queue.post(msg);
	}
	template<typename Message, typename UniqueChecker>
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

////////////////////////////////////////////////////////////////////////////////
protected:
	/*@ work thread method.	*/
	virtual void work() = 0
	{}

	Queue m_message_queue;
	Handler m_message_handler;
	eco::ThreadPool m_thread_pool;
};
}// ns.detail



////////////////////////////////////////////////////////////////////////////////
template<typename Message, typename Handler = std::function<void(Message&)> >
class MessageServer : public detail::MessageServer<Message, Handler>
{
protected:
	/*@ work thread method.	*/
	virtual void work() override
	{
		while (true)
		{
			// don't declare message before while.
			Message msg;
			if (!m_message_queue.pop(msg))
			{
				break;	// message queue has been closed.
			}

			// handler message.
			try {
				m_message_handler(msg);
			} catch (eco::Error& e) {
				EcoError << "message server: " << e;
			} catch (std::exception& e) {
				EcoLogStr(error, 512) << "message server: " << e.what();
			}
		}// end while
	}
};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif