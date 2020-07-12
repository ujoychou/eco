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
#include <eco/Any.h>
#include <eco/thread/Timer.h>
#include <eco/thread/AutoRef.h>


ECO_NS_BEGIN(eco);
template<typename Task> class TimerWheel;
typedef std::function<void(eco::Any&)> TimerWheelTaskFunc;
////////////////////////////////////////////////////////////////////////////////
template<typename Task = TimerWheelTaskFunc>
class TimerWheelTask
{
private:
	friend class TimerWheel<Task>;
	Task m_task;
	uint32_t m_count;
	uint32_t m_timeout;
	uint16_t m_repeated;
	uint16_t m_paused;
	
	// position in timer wheel.
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
		: m_task(std::move(task))
		, m_repeated(repeated)
		, m_paused(false)
		, m_count(count)
		, m_timeout(timeout)
		, m_wheel(nullptr)
		, m_timer_list(nullptr)
	{}

	inline Task& task()
	{
		return m_task;
	}

	// null task.
	inline bool null() const
	{
		return m_wheel == nullptr || m_timer_list == nullptr;
	}

	inline bool expired() const
	{
		return expired(m_count);
	}
	inline static bool expired(IN uint32_t count)
	{
		return count == 0;
	}

	inline void pause(bool is_v)
	{
		m_paused = is_v;
	}
	inline bool paused() const
	{
		return m_paused;
	}

	inline void restart(
		IN const uint32_t timeout = -1,
		IN const uint16_t repeated = -1)
	{
		if (timeout != uint32_t(-1)) m_timeout = timeout;
		if (repeated != uint16_t(-1)) m_repeated = repeated;
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
template<typename Task = TimerWheelTaskFunc>
class TimerWheel
{
	ECO_OBJECT(TimerWheel);
public:
	typedef TimerWheelTask<Task> TimerTask;
	typedef std::shared_ptr<TimerTask> TimerTaskPtr;
	typedef std::list<TimerTaskPtr> TimerList;

	struct Position
	{
		uint32_t wheel;
		uint32_t count;
		inline Position() : wheel(0), count(0) {}
	};

	// construct & destruct.
	inline TimerWheel()
	{
		reset();
	}
	inline ~TimerWheel()
	{
		reset();
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
		reset();
	}

	// get tick precision.
	inline uint32_t precision() const
	{
		return m_precision;
	}

	/*@add timer to wheel.
	* @param.timeout: timer timeout, unit is undefined.
	* @param.wheel_size: size of wheel.
	*/
	inline TimerTaskPtr add_timer(
		IN const uint32_t timeout,
		IN const bool repeated,
		IN Task& task)
	{
		auto pos = get_wheel_pos(timeout);
		auto& timer_list = m_wheel[pos.wheel];
		TimerTaskPtr timer(new TimerTask(task, pos.count, timeout, repeated));
		timer->m_wheel = this;
		timer_list.push_back(timer);
		timer->m_it = --timer_list.end();
		timer->m_timer_list = &timer_list;
		return timer;
	}

	// add timer.
	inline TimerTaskPtr add_timer_value(
		IN const uint32_t timeout,
		IN const bool repeated,
		IN Task task)
	{
		return add_timer(timeout, repeated, task);
	}

public:
	// turn the wheel in duration time.
	inline void step(IN const uint32_t tick, eco::Any& param = eco::Any())
	{
		for (uint32_t i = 0; i < tick; ++i)
		{
			on_tick(param);
		}
	}

	// turn the wheel in duration time.
	inline void step_time(IN const uint32_t duration, eco::Any& param = eco::Any())
	{
		uint32_t left = m_clock % m_precision;
		m_clock += duration;
		uint32_t tick = (left + duration) / m_precision;
		step(tick, param);
	}

	// turn the wheel to the time.
	inline void step_time_to(IN const uint64_t now, eco::Any& param = eco::Any())
	{
		// init clock.
		if (m_clock == 0)
		{
			m_clock = now;
			return;
		}

		// clock drive timer wheel.
		if (now > m_clock)
		{
			uint32_t left = m_clock % m_precision;
			uint32_t tick = uint32_t(now - m_clock + left);
			m_clock = now;
			tick /= m_precision;
			step(tick, param);
		}
	}

	// restart timer clock.
	inline void reset_clock()
	{
		m_clock = 0;
	}

	inline void log_wheel()
	{
		for (size_t i = 0; i < m_wheel.size(); i++)
		{
			if (m_wheel[i].size() > 0)
			{
				eco::Stream<> fstream;
				for (auto it = m_wheel[i].begin(); it != m_wheel[i].end(); ++it)
				{
					fstream <= m_wheel_curr <= (**it).m_count;
				}
				std::cout << i << fstream.c_str() << std::endl;
			}
		}
	}

protected:
	// stop wheel.
	inline void reset()
	{
		m_clock = 0;
		m_wheel_curr = 0;
		m_precision = 0;
		m_wheel.clear();
	}

	// trigger tick event.
	inline void on_tick(eco::Any& param)
	{
		// step to next wheel slot.
		m_wheel_curr = (m_wheel_curr + 1) % m_wheel.size();
		TimerList& timer_list = m_wheel[m_wheel_curr];
		for (auto it = timer_list.begin(); it != timer_list.end(); )
		{
			// if timer hasn't reached, reduce timer count.
			auto& timer = **it;
			if (!TimerTask::expired(--timer.m_count))
			{
				++it;
				continue;
			}
			if (timer.paused())
			{
				++it;
				continue;
			}

			// if timer has reached, call timer.
			timer.m_task(param);

			// remove once timer.
			if (!timer.m_repeated)
			{
				it = timer_list.erase(it);
				continue;
			}
			// move repeated timer to new wheel pos.
			restart(timer.m_timeout, 1, timer_list, it++);
		}
	}

	// move timer to new place.
	inline void restart(
		IN const uint32_t timeout,
		IN const uint16_t repeated,
		IN TimerList& timer_list,
		IN typename TimerList::iterator& it)
	{
		auto& timer = **it;
		auto pos = get_wheel_pos(timeout);
		auto& timer_list_new = m_wheel[pos.wheel];
		timer.m_count = pos.count;
		timer.m_repeated = repeated;
		timer.m_paused = false;
		if (&timer_list != &timer_list_new)	// slots changed.
		{
			timer_list_new.splice(timer_list_new.end(), timer_list, it);
			timer.m_it = --timer_list_new.end();
			timer.m_timer_list = &timer_list_new;
		}
	}

	// calculate wheel and count.
	inline Position get_wheel_pos(IN uint32_t timeout)
	{
		/*
		[precision=1000, wheel_size=10, curr=0]
		timeout=1000 tick=1
		timeout=1500 tick=2
		timeout=2000 tick=2
		*/
		Position pos;
		auto tick = (timeout / m_precision);
		auto left = (timeout - tick * m_precision);
		if (left > 0) ++tick;
		pos.count = (tick - 1) / (uint32_t)m_wheel.size() + 1;
		pos.wheel = (tick + m_wheel_curr) % (uint32_t)m_wheel.size();
		return pos;
	}

private:
	friend class TimerTask;
	// clock drive this timer wheel.
	uint64_t m_clock;
	uint32_t m_precision;
	uint32_t m_wheel_curr;
	std::vector<TimerList> m_wheel;
};


////////////////////////////////////////////////////////////////////////////////
template<typename Task = eco::Closure>
class WheelTimer : public TimerWheel<Task>
{
public:
	/*@start timer wheel.
	* @param.precision: tick precision, millisecond unit.
	* @param.wheel_size: size of wheel.
	*/
	inline void start(
		IN const uint32_t precision,
		IN const uint32_t wheel_size)
	{
		TimerWheel<Task>::start(precision, wheel_size);
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
		if (!is_cancel) TimerWheel<Task>::on_tick();
	}

	eco::TimerServer m_timer;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco)
#endif