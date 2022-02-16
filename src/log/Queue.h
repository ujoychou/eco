#ifndef ECO_LOG_QUEUE_H
#define ECO_LOG_QUEUE_H
/*******************************************************************************
@ name
log queue.

@ function

@ exception

@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/log/Type.h>
#include <eco/std/thread.h>
#include <eco/thread/State.h>
#include <eco/std/condition_variable.h>
#include <boost/ptr_container/ptr_deque.hpp>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(log);
typedef eco::Buffer<pack_size> Pack;
////////////////////////////////////////////////////////////////////////////////
class Queue
{
	ECO_OBJECT(Queue);
public:
	typedef boost::ptr_deque<Pack> PackArray;
	typedef boost::ptr_deque<Pack>::auto_type PackPtr;

public:
	/*@ constructor: set log queue bytes capacity.
	*/
	inline explicit Queue()
		: m_cur_size(0)
		, m_capacity(queue_size)
		, m_max_sync_interval(eco::log::sync_interval)
	{
		open();
	}

	// queue name.
	inline void set_name(IN const char* name)
	{
		m_name = name;
	}
	inline const char* name() const
	{
		return m_name.c_str();
	}

	// set logging sync max interval.
	inline void set_sync_interval(IN uint32_t millsecs)
	{
		m_max_sync_interval = millsecs;
	}

	// log queue bytes capacity.
	inline void set_capacity(IN uint32_t size)
	{
		m_capacity = size;
	}

	inline uint32_t capacity() const
	{
		return m_capacity;
	}

	inline uint32_t size() const
	{
		return m_cur_size;
	}

	inline void open()
	{
		m_state.set_ok(true);
	}

	/*@ when close logging, it should log all the data in queue.
	and stop post log before close queue.
	*/
	inline void close()
	{
		m_state.set_ok(false);
		m_avail_cond_var.notify_all();
		m_logging_cond_var.notify_all();
	}

	// compatible with worker.
	inline void release()
	{
		assert(false);
	}

	template<typename Text>
	inline void post(IN const Text& text)
	{
		// text size is too large.
		if (text.size() > Pack::capacity())
		{
			throw std::logic_error("text size is larger than log pack.");
		}

		std_unique_lock lock(m_mutex);
		// if current pack is full, move avail to logging buffer.
		if (!m_avail.empty() && m_avail.back().avail() < text.size())
		{
			move_avail();
			m_logging_cond_var.notify_one();
		}

		// if all buffer is using logging, waiting.
		Pack* pack = get_vail(text.size());
		while (pack == nullptr)
		{
			m_avail_cond_var.wait(lock);
			pack = get_vail(text.size());
		}
		
		// if current pack is full, move avail to logging buffer.
		pack->append(text);
		if (pack->is_full())
		{
			move_avail();
			m_logging_cond_var.notify_one();
		}
	}

	// compatible with eco::work<>()
	inline size_t pop(eco::Bytes& pack) { return 0; }

	inline void pop(PackPtr& pack)
	{
		std_unique_lock lock(m_mutex);
		
		// recycle avail pack.
		if (pack.get() != nullptr)
		{
			recycle_avail(pack);
		}

		// logging when: 1) has log pack; 2) wait for 3 seconds and has log.
		while (m_logging.empty())
		{
			if (is_close())
			{
				pack.reset();
				return ;
			}
			if (std_cv_status::timeout == m_logging_cond_var.wait_for(
				lock, std_chrono::milliseconds(m_max_sync_interval)))
			{
				move_avail();
			}
		}
		pack = m_logging.pop_front();
	}

	inline int is_open()
	{
		return m_state.ok();
	}

	inline int is_close()
	{
		return !is_open();
	}

private:
	Pack* get_vail(IN uint32_t text_size)
	{
		if (!m_avail.empty())
		{
			return &m_avail.back();
		}

		// all buffer is logging, no avail buffer to use.
		if (m_cur_size + Pack::capacity() > m_capacity)
		{
			return nullptr;
		}

		// allocate new buffer.
		m_avail.push_back(new Pack);
		m_cur_size += Pack::capacity();
		return &m_avail.back();
	}

	inline void move_avail()
	{
		if (!m_avail.empty() && m_avail.back().size() > 0)
		{
			m_logging.push_back(m_avail.pop_back().release());
		}
	}

	void recycle_avail(IN PackPtr& pack)
	{
		PackArray::iterator it;
		it = !m_avail.empty() ? (m_avail.end() - 1) : m_avail.end();
		pack->clear();
		m_avail.insert(it, pack.release());
		m_avail_cond_var.notify_all();
	}

private:
	friend class Recycle;

	// vail buffer: as the empty space in water pool.
	PackArray m_avail;
	// used buffer: as the water space in water pool.
	PackArray m_logging;

	// bytes control.
	uint32_t m_cur_size;
	uint32_t m_capacity;

	// synchronous mutex
	std_mutex m_mutex;
	std_condition_variable m_avail_cond_var;
	std_condition_variable m_logging_cond_var;
	uint32_t m_max_sync_interval;

	// queue status.
	eco::atomic::State m_state;
	std::string m_name;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(log);
ECO_NS_END(eco);
#endif