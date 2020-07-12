#ifndef ECO_THREAD_MESSAGE_QUEUE_H
#define ECO_THREAD_MESSAGE_QUEUE_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/thread/State.h>
#include <eco/thread/ConditionVariable.h>
#include <eco/date_time/Time.h>
#include <eco/log/Log.h>
#include <deque>
#include <mutex>
#include <atomic>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class MessageQueueFlow : public eco::date_time::SizeFlow
{
public:
	typedef eco::date_time::SizeFlow SizeFlow;
	inline MessageQueueFlow() : SizeFlow()
	{}

	inline void count(
		IN size_t size,
		IN const std::string& name,
		IN const size_t capacity)
	{
		if (!SizeFlow::count()) return;

		if (size * 10 >= capacity)
		{
			ECO_ERROR << name <= size <= m_size_flow < "/s" <= m_size;
		}
		else if (size * 20 >= capacity)
		{
			ECO_WARN << name <= size <= m_size_flow < "/s" <= m_size;
		}
	}
};

////////////////////////////////////////////////////////////////////////////////
template<typename Message>
class MessageQueue
{
	ECO_OBJECT(MessageQueue);
////////////////////////////////////////////////////////////////////////////////
public:
	// max size.
	enum { default_capacity = 5000 };

	/*@ the message queue max message size.*/
	inline MessageQueue(IN uint32_t capacity = default_capacity)
		: m_open(false)
	{
		set_capacity(capacity);
		open();
	}

	// set name of this message queue.
	inline void set_name(const char* name)
	{
		m_name = name;
	}
	inline void set_name(const std::string& name)
	{
		m_name = name;
	}
	inline const std::string& name() const
	{
		return m_name;
	}

	/*@ set message queue capacity.*/
	inline void set_capacity(IN const uint32_t capacity)
	{
		m_capacity = (capacity > 0) ? capacity : default_capacity;
	}
	inline uint32_t capacity() const
	{
		return m_capacity;
	}

	/*@ open message queue so that it can recv message.*/
	inline void open()
	{
		m_open = true;
	}
	inline bool is_open() const
	{
		return m_open;
	}

	/*@ close message queue so that it stop to recv message. but it will wait 
	all message be handled.
	*/
	inline void close()
	{
		if (is_close()) return;
		m_open = false;

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_deque.clear();
		}

		// notify all thread to exit message queue.
		m_empty_cond_var.notify_all();
		m_full_cond_var.notify_all();
	}
	inline bool is_close() const
	{
		return !m_open;
	}

	/*@ post message to message queue.
	* @ para.msg: message type is like "std::function", "std::shared_ptr",
	and some can be operated by "std::move()".
	*/
	inline void post(IN Message& msg)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_full_cond_var.wait(lock, [=] { return !is_full(); });
			m_deque.push_back(std::move(msg));
			m_flow.count(m_deque.size(), m_name, m_capacity);
		}
		m_empty_cond_var.notify_one();
	}

	/*@ post message to message queue and ensure that this message is unique.
	if this message is exist, replace it.
	std::function<bool(const Message& m1, const Message& m2)>
	*/
	template<typename UniqueChecker>
	inline void post(IN Message& msg, IN UniqueChecker& check)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			auto it = std::find_if(m_deque.begin(), m_deque.end(), check);
			if (it != m_deque.end())
			{
				*it = std::move(msg);
				return;
			}
			m_full_cond_var.wait(lock, [=] { return !is_full(); });
			m_deque.push_back(std::move(msg));
			m_flow.count(m_deque.size(), m_name, m_capacity);
		}
		m_empty_cond_var.notify_one();
	}

	/*@ pop message from this message queue.*/
	inline size_t pop(OUT Message& msg)
	{
		size_t size = 0;
		{
			std::unique_lock<std::mutex> lk(m_mutex);
			m_empty_cond_var.wait(lk, [=] {
				return !is_empty() || is_close();
			});
			if (is_close()) return -1;
			msg = std::move(m_deque.front());
			m_deque.pop_front();
			size = m_deque.size();
		}
		m_full_cond_var.notify_one();
		return size;
	}

	// is message queue empty.
	inline bool empty() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_deque.empty();
	}

	// is message queue empty.
	inline uint32_t size() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return (uint32_t)m_deque.size();
	}

	// message queue mutex.
	inline std::mutex& mutex()
	{
		return m_mutex;
	}

////////////////////////////////////////////////////////////////////////////////
private:
	inline bool is_empty() const
	{
		return m_deque.empty();
	}

	inline bool is_full() const
	{
		return (m_deque.size() == m_capacity);
	}

	// name of this message queue.
	std::string m_name;
	MessageQueueFlow m_flow;

	//  message queue max size.
	uint32_t m_capacity;

	// message queue data.
	std::deque<Message> m_deque;

	// message queue state.
	std::atomic<bool> m_open;

	// when message queue is full and empty, synchronous notify.
	mutable std::mutex m_mutex;
	std::condition_variable m_full_cond_var;
	std::condition_variable m_empty_cond_var;
};


////////////////////////////////////////////////////////////////////////////////
}// ns.eco
#endif