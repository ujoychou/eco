#ifndef VISION_MARKET_TIMER_WHEEL_H
#define VISION_MARKET_TIMER_WHEEL_H
/*******************************************************************************
@ name


@ function
1.using uint64_t for support high presicion.


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2017-03-10.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2018 - 2020, ujoy, reserved all right.

*******************************************************************************/
#include <eco/thread/Timer.h>
#include <eco/thread/AutoRef.h>


ECO_NS_BEGIN(eco)
////////////////////////////////////////////////////////////////////////////////
template<typename Task = eco::Closure>
class TimerWheelTask
{
public:
	Task m_task;
	uint16_t m_repeated;
	uint32_t m_count;
	uint32_t m_timeout;
	
public:
	inline TimerWheelTask(
		IN const Task& task,
		IN const uint32_t count,
		IN const uint32_t timeout,
		IN const bool repeated)
		: m_task(task), m_repeated(repeated)
		, m_count(count), m_timeout(timeout)
	{}

	inline bool expired() const
	{
		return expired(m_count);
	}
	inline static bool expired(IN uint32_t count)
	{
		return count == -1;
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename Task = eco::Closure>
class TimerWheel
{
public:
	typedef std::shared_ptr<TimerWheelTask<Task> > TimerWheelTaskPtr;
	typedef std::list<TimerWheelTaskPtr> TimerWheelList;
	class Timer
	{
		TimerWheelTaskPtr m_data;
		typename TimerWheelList::iterator m_it;
		TimerWheel* m_wheel;
		TimerWheelList* m_timer_list;
		friend class TimerWheel;

	public:
		inline Timer() : m_wheel(nullptr), m_timer_list(nullptr)
		{}

		inline Timer(
			IN TimerWheelTaskPtr& data,
			IN typename TimerWheelList::iterator& it,
			IN TimerWheel* wheel,
			IN TimerWheelList* timer_list)
			: m_data(std::move(data)), m_it(it)
			, m_wheel(wheel), m_timer_list(timer_list)
		{}

		inline bool expired() const
		{
			return (m_data == nullptr || m_data->expired());
		}

		inline void reset(
			IN const uint32_t timeout = -1,
			IN const uint16_t repeated = -1)
		{
			if (timeout != -1)
				m_data->m_timeout = timeout;
			if (repeated != -1)
				m_data->m_repeated = -1;

			if (!expired())
			{
				m_wheel->reset_timer(m_data->m_timeout,
					m_data->m_repeated, *m_timer_list, m_it);
			}
			else
			{
				m_wheel->add_timer(*this);
			}
		}

		inline void cancel()
		{
			if (!expired())
			{
				m_timer_list->erase(m_it);
			}
		}
	};

public:
	// construct & destruct.
	inline TimerWheel() : m_curr(0), m_precision(0)
	{}
	inline ~TimerWheel()
	{
		stop();
	}

	// release wheel.
	inline void stop()
	{
		m_curr = 0;
		m_precision = 0;
		m_wheel.clear();
		m_timer.stop();
	}

	/*@start timer wheel.
	* @param.precision: tick precision, unit is "milli second".
	* @param.wheel_size: size of wheel.
	*/
	inline void start(
		IN const uint32_t precision,
		IN const uint32_t wheel_size = 30)
	{
		m_precision = precision;
		m_wheel.resize(wheel_size);
		m_timer.start();
		m_timer.add_timer(precision, true, std::bind(
			&TimerWheel::on_tick, this, std::placeholders::_1));
	}

	/*@add timer to wheel.
	* @param.timeout: timer timeout, unit is "milli second".
	* @param.wheel_size: size of wheel.
	*/
	inline Timer add_timer(
		IN const uint32_t timeout,
		IN const bool repeated,
		IN const Task& task)
	{
		auto result = get_wheel_count(timeout);
		auto& timer_list = m_wheel[result.first];
		TimerWheelTaskPtr data(new TimerWheelTask<Task>(
			task, result.second, timeout, repeated));
		timer_list.push_back(data);
		return Timer(data, --timer_list.end(), this, &timer_list);
	}

	// add timer.
	inline Timer add_timer_v(
		IN const uint32_t timeout,
		IN const bool repeated,
		IN Task task)
	{
		return add_timer(timeout, repeated, task);
	}

private:
	// trigger tick event.
	inline void on_tick(IN bool is_cancel)
	{
		if (is_cancel) return;

		TimerWheelList& timer_list = m_wheel[m_curr++];
		for (auto it = timer_list.begin(); it != timer_list.end(); )
		{
			auto& timer = **it;

			// if timer hasn't reached, reduce timer count.
			if (!TimerWheelTask<Task>::expired(--timer.m_count))
			{
				++it;
				continue;
			}
			// if timer has reached, call timer.
			timer.m_task();

			// remove once timer.
			if (!timer.m_repeated)
			{
				it = timer_list.erase(it);
				continue;
			}
			// repeated add timer to new timer list.
			reset_timer(timer.m_timeout, true, timer_list, it++);
		}
	}

	inline void add_timer(IN Timer& timer)
	{
		auto result = get_wheel_count(timer.m_data->m_timeout);
		auto& timer_list = m_wheel[result.first];
		timer_list.push_back(timer.m_data);
		timer.m_data->m_count = result.second;
		timer.m_it = --timer_list.end();
		timer.m_timer_list = &timer_list;
	}

	// move timer to new place.
	inline void reset_timer(
		IN const uint32_t timeout,
		IN const uint16_t repeated,
		IN TimerWheelList& timer_list,
		IN typename TimerWheelList::iterator& it)
	{
		auto pos = get_wheel_count(timeout);
		(**it).m_count = pos.second;
		(**it).m_repeated = repeated ? 1 : 0;
		m_wheel[pos.first].splice(m_wheel[pos.first].end(), timer_list, it);
	}

	// calculate wheel and count.
	inline std::pair<uint32_t, uint32_t>
		get_wheel_count(IN uint32_t timeout)
	{
		std::pair<uint32_t, uint32_t> result;
		auto tick = timeout / m_precision;
		result.first = (tick + m_curr) % m_wheel.size();	// wheel
		result.second = (tick + m_curr) / m_wheel.size();	// count
		return result;
	}

private:
	friend class Timer;
	uint32_t m_curr;
	uint32_t m_precision;
	std::vector<TimerWheelList> m_wheel;
	eco::Timer m_timer;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco)
#endif