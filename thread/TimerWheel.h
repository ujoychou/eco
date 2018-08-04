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


ECO_NS_BEGIN(eco);
template<typename Task> class TimerWheel;
////////////////////////////////////////////////////////////////////////////////
template<typename Task = eco::Closure>
class TimerWheelTask
{
private:
	friend class TimerWheel<Task>;
	Task m_task;
	uint32_t m_count;
	uint32_t m_timeout;
	uint16_t m_repeated;
	
	// timer position.
	typedef std::shared_ptr<TimerWheelTask> Timer;
	typedef std::list<Timer> TimerList;
	TimerList* m_timer_list;
	TimerWheel<Task>* m_wheel;
	typename TimerList::iterator m_it;
	
public:
	inline TimerWheelTask(
		IN Task& task,
		IN const uint32_t count,
		IN const uint32_t timeout,
		IN const bool repeated)
		: m_task(std::move(task)), m_repeated(repeated)
		, m_count(count), m_timeout(timeout)
		, m_wheel(nullptr), m_timer_list(nullptr)
	{}

	inline Task& task()
	{
		return m_task;
	}

	inline bool expired() const
	{
		return expired(m_count);
	}
	inline static bool expired(IN uint32_t count)
	{
		return count == -1;
	}

	inline void restart(
		IN const uint32_t timeout = -1,
		IN const uint16_t repeated = -1)
	{
		if (timeout != uint32_t(-1))
			m_timeout = timeout;
		if (repeated != uint16_t(-1))
			m_repeated = repeated;
		m_wheel->restart(m_timeout, m_repeated, *m_timer_list, m_it);
	}

	inline void cancel()
	{
		if (!expired())
		{
			m_timer_list->erase(m_it);
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename Task = eco::Closure>
class TimerWheel
{
	ECO_OBJECT(TimerWheel);
public:
	typedef TimerWheelTask<Task> TimerTask;
	typedef std::shared_ptr<TimerTask> Timer;
	typedef std::list<Timer> TimerList;

	// construct & destruct.
	inline TimerWheel() : m_curr(0), m_precision(0), m_max_timeout(0)
	{}
	inline ~TimerWheel()
	{
		stop();
	}

	/*@start timer wheel.
	* @param.precision: tick precision.
	* @param.wheel_size: size of wheel.
	*/
	inline void start(
		IN const uint32_t precision,
		IN const uint32_t wheel_size)
	{
		m_precision = precision;
		m_wheel.resize(wheel_size);
	}

	// stop wheel.
	inline void stop()
	{
		m_curr = 0;
		m_precision = 0;
		m_wheel.clear();
	}

	// turn the wheel in duration time.
	inline uint32_t step(IN const uint32_t duration)
	{
		uint32_t tick = duration / m_precision;
		for (uint32_t i = 0; i < tick; ++i)
		{
			on_tick(0);
		}
		return tick * m_precision;
	}

	// clear wheel.
	inline uint32_t step_to(IN const uint32_t duration)
	{
		if (m_max_timeout == 0)
		{
			return duration;
		}
		if (duration > m_max_timeout)
		{
			call_once_and_restart_repeated();
			return duration;
		}

		uint32_t tick = duration / m_precision;
		for (uint32_t i = 0; i < tick; ++i)
		{
			on_tick(tick - 1 - i);
		}
		return tick * m_precision;
	}

	/*@add timer to wheel.
	* @param.timeout: timer timeout, unit is undefined.
	* @param.wheel_size: size of wheel.
	*/
	inline Timer add_timer(
		IN const uint32_t timeout,
		IN const bool repeated,
		IN Task& task)
	{
		auto result = get_wheel_count(timeout);
		auto& timer_list = m_wheel[result.first];
		Timer timer(new TimerTask(task, result.second, timeout, repeated));
		timer_list.push_back(timer);
		timer->m_wheel = this;
		timer->m_timer_list = &timer_list;
		timer->m_it = --timer_list.end();
		return timer;
	}

	// add timer.
	inline Timer add_timer_v(
		IN const uint32_t timeout,
		IN const bool repeated,
		IN Task task)
	{
		return add_timer(timeout, repeated, task);
	}

	// get max timeout.
	inline uint32_t max_timeout() const
	{
		return m_max_timeout;
	}

	// get tick precision.
	inline uint32_t precision() const
	{
		return m_precision;
	}

protected:
	// trigger tick event.
	inline void on_tick(IN const uint32_t delay = 0)
	{
		// step to next wheel slot.
		m_curr = (m_curr + 1) % m_wheel.size();
		m_max_timeout = (m_max_timeout > m_precision)
			? m_max_timeout - m_precision : 0;
		EcoInfo << "on_tick:" <= m_max_timeout <= m_curr;

		TimerList& timer_list = m_wheel[m_curr];
		for (auto it = timer_list.begin(); it != timer_list.end(); )
		{
			// if timer hasn't reached, reduce timer count.
			auto& timer = **it;
			if (!TimerTask::expired(--timer.m_count))
			{
				++it;
				continue;
			}
			// if timer has reached, call timer.
			timer.m_task(timer);

			// remove once timer.
			if (!timer.m_repeated)
			{
				it = timer_list.erase(it);
				continue;
			}
			// move repeated timer to new wheel pos.
			auto timeout = timer.m_timeout + delay * m_precision;
			restart(timeout, true, timer_list, it++);
		}
	}

	// trigger all timer event.
	inline void call_once_and_restart_repeated()
	{
		uint32_t curr_end = (m_curr + 1) % m_wheel.size();
		do
		{
			m_curr = (m_curr + 1) % m_wheel.size();
			TimerList& timer_list = m_wheel[m_curr];
			for (auto it = timer_list.begin(); it != timer_list.end(); )
			{
				auto& timer = **it;
				timer.m_task(timer);
				it = (!timer.m_repeated) ? timer_list.erase(it) : ++it;
			}
		} while (m_curr != curr_end);

		// reset repeated timer.
		uint32_t curr = m_curr = 0;
		m_max_timeout = 0;
		do
		{
			TimerList& timer_list = m_wheel[curr];
			for (auto it = timer_list.begin(); it != timer_list.end(); )
			{
				auto& timer = **it;
				restart(timer.m_timeout, true, timer_list, it++);
			}
			curr = (curr + 1) % m_wheel.size();
		} while (curr != 0);
	}

	// move timer to new place.
	inline void restart(
		IN const uint32_t timeout,
		IN const uint16_t repeated,
		IN TimerList& timer_list,
		IN typename TimerList::iterator& it)
	{
		auto& timer = **it;
		auto pos = get_wheel_count(timeout);
		auto& timer_list_new = m_wheel[pos.first];
		// timer second changed.
		if (&timer_list != &timer_list_new || timer.m_count != pos.second)
		{
			timer_list_new.splice(timer_list_new.end(), timer_list, it);
			timer.m_it = --timer_list_new.end();
			timer.m_count = pos.second;
			timer.m_repeated = repeated ? 1 : 0;
			timer.m_timer_list = &timer_list_new;
		}
		EcoInfo << "restart:" <= m_max_timeout <= m_curr;
	}

	// calculate wheel and count.
	inline std::pair<uint32_t, uint32_t>
		get_wheel_count(IN uint32_t timeout)
	{
		std::pair<uint32_t, uint32_t> result;
		auto tick = timeout / m_precision;
		result.second = tick / m_wheel.size();	// count
		result.first = (tick + m_curr) % m_wheel.size();	// wheel
		// count the max timeout.
		// max_timeout = tick + 1; timeout = (tick + 0.n)
		if (m_max_timeout < (tick * m_precision))
			m_max_timeout = (tick + 1) * m_precision;
		return result;
	}

private:
	friend class TimerTask;
	uint32_t m_curr;
	uint32_t m_precision;
	uint32_t m_max_timeout;
	std::vector<TimerList> m_wheel;
};


////////////////////////////////////////////////////////////////////////////////
template<typename Task = eco::Closure>
class WheelTimer : public TimerWheel<Task>
{
public:
	/*@start timer wheel.
	* @param.tick: tick precision, unit is "milli second".
	* @param.wheel_size: size of wheel.
	*/
	inline void start(
		IN const uint32_t tick,
		IN const uint32_t wheel_size)
	{
		TimerWheel<Task>::start(tick, wheel_size);
		m_timer.start();
		m_timer.add_timer(precision, true, std::bind(
			&WheelTimer::on_tick, this, std::placeholders::_1));
	}

	// stop wheel.
	inline void stop()
	{
		TimerWheel<Task>::stop();
		m_timer.stop();
	}

private:
	// disable "step".
	inline void step(IN const uint32_t duration);

	// trigger tick event.
	inline void on_tick(IN bool is_cancel)
	{
		if (!is_cancel)
			TimerWheel<Task>::on_tick();
	}

	eco::Timer m_timer;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco)
#endif