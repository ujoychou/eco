#ifndef ECO_TIMING_WHEEL_H
#define ECO_TIMING_WHEEL_H
/*******************************************************************************
@ name


@ function
1.TimingWheelT min prec unit: "milliseconds". 
2.using uint64 calculate time improve speed.

@ key thought:
#.时间轮的格子计算
1."[0,1)"时刻表示已过期，不加入时间轮。
2."[1,2)"时刻加入时间轮第1格，"[2,3)"时刻加入时间轮第2格，以此类推。
#.时间轮上的任务移动规则：由粗精度时间轮移动到细精度时间轮，并在最细精度的时间轮
上执行任务。
1.例子：有一个"时/分/秒"精度的层级时间轮，则任务将从"时"时间轮移动到"分"时间轮，
再移动到"秒"时间轮，然后在"秒"时间轮上执行。
2.基于以上的时刻加入时间轮规则，父级时间轮的第一格对应了其子级时间轮，两者的时
间范围相同。因为父级时间轮的精度=子级时间轮的总时间。
3.所以：父级时间轮滚动一格时，需要将滚动后这一格的任务全部移动到子级时间轮上。
4.父级时间轮与子级时间轮开始时间相同，走过的时间相同。
5.子轮盘转动一圈时，触发父轮盘，并执行完父轮盘的移动，再处理子轮盘的任务，否则先
处理子轮盘的任务，在移动父轮盘在边界位置会导致任务未触发。
6.定时器cancel后不能restart。

@ process of wheel roll
(推动时间轮走动1格。)
1.有子级时间轮：
1.1.移动标记到下个格子。（重置当前时间）
1.2.N级时间轮走完一圈，推动父级时间轮，递归执行1过程。
    N级时间轮走完一圈的判定标准：tick_cur >= wheel.size().
1.3.移动N级下一个格子的任务到子级时间轮。
2.无子级时间轮：
2.1.移动当前格子的任务到等待队列。
2.2.移动标记到下个格子。（重置当前时间）
2.3.N级时间轮走完一圈，推动父级时间轮，递归执行2过程。
	N级时间轮走完一圈的判定标准：tick_cur >= wheel.size().
2.4.执行等待队列中的任务。

@ concurrency handle.
1.m_mutex_wheel: for all "m_tick/m_time" in wheel/parent wheel/...;
2.m_mutex_wheel: for "ent->m_expire", write by restart/cancel/step_task-repeat.
注意：[m_expire]在cancel中读与其他地方写存在理论并发问题，但并不影响最终结果。
3.m_mutex_parent: for parent wheel ptr.

@ bugs record.
1.崩溃"m_wheel[pos].add(ent);"，由于内存不够导致的崩溃。[20201121 by ujoy]
原因：由于行情太快，每100毫秒可能有很多数据，导致频繁调用restart，从而一个定时器
被重复的添加到时间轮中。
方案：restart判定任务是否会移入其他bucket，若还在相同bucket中，则不重新添加。

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2020-09-10.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2018 - 2020, ujoy, reserved all right.

*******************************************************************************/
#include <eco/thread/State.h>
#include <eco/thread/Thread.h>
#include <eco/date_time/Time.h>
#include <eco/cpp/Thread.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename task_t, typename task_handler_t = eco::Handler>
class TimerWheelT
{
	ECO_OBJECT(TimerWheelT);
private:
	class TaskEntry : public eco::Object<TaskEntry>
	{
	public:
		inline TaskEntry(
			IN task_t& task,
			IN uint64_t expire,
			IN uint32_t expire_dur,
			IN TimerWheelT* wheel)
			: m_task(std::move(task))
			, m_expire(expire)
			, m_expire_dur(expire_dur)
			, m_wheel(wheel)
		{}

		inline ~TaskEntry()
		{
			if (!canceled()) run();
		}

		inline void run()
		{
			task_handler_t()(m_task);
		}

		inline void cancel()
		{
			m_expire = 0;
		}

		inline bool canceled() const
		{
			return m_expire == 0;
		}

		inline bool repeated() const
		{
			return m_wheel != 0;
		}

		task_t			m_task;			// task to execute.
		uint64_t		m_expire;		// expire time.
		TimerWheelT*	m_wheel;		// for add repeated timer.
		uint32_t		m_expire_dur;	// run_after mode, for restart using.
	};

public:
	// the object return by TimerWheelT add.
	class Timer
	{
    ECO_NONCOPYABLE(Timer);
	public:
		inline Timer() : m_wheel(nullptr)
		{}

		inline Timer(typename TaskEntry::wptr& t, TimerWheelT* wheel)
			: m_entry(std::move(t)), m_wheel(wheel)
		{}

		inline Timer(Timer&& obj)
			: m_entry(std::move(obj.m_entry))
			, m_wheel(obj.m_wheel)
		{
			obj.m_wheel = nullptr;
		}

		inline Timer& operator=(Timer&& obj)
		{
			m_entry = std::move(obj.m_entry);
			m_wheel = obj.m_wheel;
			obj.m_wheel = nullptr;
			return *this;
		}

		// restart timer, for both run_after mode.
		inline bool restart()
		{
			TaskEntry::ptr ent = m_entry.lock();
			if (!ent) return false;
			if (ent->m_expire_dur == 0) return false;
			return m_wheel->run_restart(ent);
		}

		inline bool cancel()
		{
			TaskEntry::ptr ent = m_entry.lock();
			if (ent) m_wheel->run_cancel(ent);
			return ent != nullptr;
		}

		inline void stop()
		{
			cancel();
			eco::thread::release(m_entry);
		}

		inline operator bool() const
		{
			return !canceled();
		}

		inline bool canceled() const
		{
			TaskEntry::ptr task = m_entry.lock();
			return (task == nullptr || task->canceled());
		}

		inline bool expired() const
		{
			return m_entry.expired();
		}

	private:
		TimerWheelT* m_wheel;
		typename TaskEntry::wptr m_entry;
	};

	class Wheel
	{
	public:
		typedef std::vector<typename TaskEntry::ptr> TaskEntryArray;
		class Bucket
		{
		public:
			inline Bucket() : m_entrys(0)
			{}

			inline uint32_t size() const
			{
				std_lock_guard lock(m_mutex);
				return (uint32_t)m_entrys.size();
			}

			inline void add(typename TaskEntry::ptr& v)
			{
				std_lock_guard lock(m_mutex);
				m_entrys.push_back(std::move(v));
			}

			inline void cancel()
			{
				std_lock_guard lock(m_mutex);
				for (auto it = m_entrys.begin(); it != m_entrys.end(); ++it)
				{
					typename TaskEntry::ptr& t = *it;
					t->cancel();
				}
				m_entrys.clear();
			}

			inline void move_to_child(TimerWheelT& tw, Wheel& child)
			{
				std_lock_guard lock(m_mutex);
				for (auto it = m_entrys.begin(); it != m_entrys.end(); ++it)
				{
					typename TaskEntry::ptr& ent = *it;
					if (!ent->canceled()) child.run_task(tw, std::move(ent));
				}
				m_entrys.clear();
			}

			inline void move_to_work(TaskEntryArray& set)
			{
				std_lock_guard lock(m_mutex);
				std::move(m_entrys.begin(), m_entrys.end(), back_inserter(set));
				m_entrys.clear();
			}

			mutable std_mutex m_mutex;
			TaskEntryArray m_entrys;
		};

	public:
		inline Wheel() : m_time(0), m_prec(0), m_tick(0), m_size(0), m_wheel(0)
		{}

		inline ~Wheel()
		{
			release();
		}

		// release task data.
		inline void release()
		{
			cancel();
			delete[] m_wheel;
			m_size = 0;
			m_wheel = nullptr;
		}

		inline bool empty() const
		{
			return m_size == 0;
		}
		inline uint32_t size() const
		{
			return m_size;
		}

		// wheel total cycle milliseconds.
		inline uint32_t prec_ms() const
		{
			return m_prec;
		}
		inline uint32_t wheel_ms() const
		{
			return m_prec * m_size;
		}

		inline void init(uint64_t t, uint32_t p, uint32_t s)
		{
			m_prec = p;
			m_size = s > 1 ? s : 2;
			m_time = t;
			m_wheel = new Bucket[m_size];
		}

		// add timer of task entry to bucket.
		inline void run_task(TimerWheelT& tw, typename TaskEntry::ptr& ent)
		{
			uint64_t expire = ent->m_expire;
			if (expire >= m_time + wheel_ms())
			{
				parent(tw).run_task(tw, ent);
			}
			else if (expire <= m_time)
			{
				//ECO_FUNC(info) <= m_tick <= m_tick <= expire;
				m_wheel[m_tick].add(ent);
			}
			else
			{
				//ECO_FUNC(info) <= m_tick <= pos <= expire;
				uint32_t pos = get_pos(expire);
				m_wheel[pos].add(ent);
			}
		}

		inline Timer run_at(
			TimerWheelT& tw, task_t& task,
			uint64_t expire, uint32_t expire_dur, bool repeat)
		{
			// has been expired
			if (expire < m_time + m_prec) return Timer();

			// add task entry to wheel bucket.
			TaskEntry::ptr ent = std::make_shared<TaskEntry>(
				task, expire, expire_dur, (repeat ? &tw : 0));
			TaskEntry::wptr ent_wptr = ent;
			run_task(tw, ent);
			return Timer(ent_wptr, &tw);
		}

		inline Timer run_at(
			TimerWheelT& tw, task_t& task,
			const std_chrono::system_clock::time_point& tp)
		{
			using namespace std_chrono;
			auto t = duration_cast<milliseconds>(tp.time_since_epoch());
			if (t.count() < 0) return Timer();
			return run_at(tw, task, t.count(), 0, false);
		}

		inline Timer run_after(
			TimerWheelT& tw, task_t& task, uint32_t expire_dur, bool repeat)
		{
			auto ms = TimerWheelT::now_ms();
			if (expire_dur < m_prec) expire_dur = m_prec;
			return run_at(tw, task, ms + expire_dur, expire_dur, repeat);
		}

		inline Timer run_daily(
			TimerWheelT& tw, task_t& task, const eco::date_time::Time& time)
		{
			// get now time_point.
			using namespace std_chrono;
			system_clock::time_point tp = system_clock::now();
			std::time_t tt = system_clock::to_time_t(tp);
			struct std::tm* t = std::localtime(&tt);
			uint32_t daily_sec = date_time::Time(t).seconds();

			// get daily millseconds.
			auto mills = duration_cast<milliseconds>(tp.time_since_epoch());
			if (daily_sec >= time.seconds())
			{
				mills += std_chrono::hours(24);
				mills -= std_chrono::seconds(daily_sec - time.seconds());
			}
			else
			{
				mills += std_chrono::seconds(time.seconds() - daily_sec);
			}
			auto mills_repeat = duration_cast<milliseconds>(hours(24));
			return run_at(tw, task, mills.count(),
				(uint32_t)mills_repeat.count(), true);
		}

		// make parent timer wheel.
		inline Wheel& parent(TimerWheelT& tw)
		{
			std_lock_guard lock(m_mutex_parent);
			if (m_parent == nullptr)
			{
				uint32_t prec = wheel_ms();
				uint64_t time = m_time - (m_time - tw.start_ms()) % prec;
				m_parent.reset(new Wheel());
				m_parent->init(time, prec, m_size);
			}
			return *m_parent;
		}
		inline Wheel* get_parent()
		{
			std_lock_guard lock(m_mutex_parent);
			return m_parent.get();
		}

		// cancel all tasks in the timer wheel
		inline void cancel()
		{
			Wheel* wheel = get_parent();
			if (wheel) wheel->cancel();
			for (size_t i = 0; i < m_size; ++i) m_wheel[i].cancel();
		}

	public:
		inline uint32_t count_tick(uint64_t now) const
		{
			if (now < m_time + m_prec) return 0;
			return uint32_t(now - m_time) / m_prec;
		}

		inline uint32_t get_tick(uint64_t expire) const
		{
			return uint32_t((expire - m_time) / m_prec);
		}
		inline uint32_t get_pos(uint64_t expire) const
		{
			return (m_tick + get_tick(expire)) % m_size;
		}

		inline void step(TimerWheelT& tw)
		{
			// update wheel time and tick.
			writer_lock lock(tw.m_mutex_wheel);
			uint32_t pos = m_tick;
			bool roll_parent = next_pos();

			// 1.1.roll task.
			TaskEntryArray ent_set(0);
			step_task(tw, ent_set, pos);

			// 1.2.roll parent wheel when reach one loop.
			if (roll_parent) roll_parent_wheel(tw);

			// 1.3.execute the task.
			lock.unlock();
			for (auto it = ent_set.begin(); it != ent_set.end(); ++it)
			{
				// #.execute repeated task.
				TaskEntry::ptr& ent = *it;
				if (ent && ent->repeated() && !ent->canceled())
				{
					// "use_count == 2", because of has been repeated.
					if (ent.use_count() == 2) ent->run();
				}
				// #.execute none repeated task.
				ent.reset();
			}
		}

	private:
		// move to next tick pos, return true when finish one loop.
		inline bool next_pos()
		{
			m_time += m_prec;
			if (++m_tick >= m_size) m_tick -= m_size;
			return m_tick == 0;	// finish one loop.
		}

		// roll parent wheel.
		inline void roll_parent_wheel(TimerWheelT& tw)
		{
			Wheel* wheel = get_parent();
			if (wheel) wheel->step_move_to_child(tw, *this);
		}

		inline void step_move_to_child(TimerWheelT& tw, Wheel& child)
		{
			// 1.2.1.update wheel time and tick.
			bool roll_parent = next_pos();

			// 1.2.2.roll parent wheel when reach one loop.
			if (roll_parent) roll_parent_wheel(tw);

			// 1.2.3.move task to child wheel.
			m_wheel[m_tick].move_to_child(tw, child);
		}

		// 1.move task for waiting to execute.
		inline void step_task(TimerWheelT& tw, TaskEntryArray& set, uint32_t pos)
		{
			// 1.1.1.move task for waiting to execute.
			set.reserve(m_wheel[pos].size());
			m_wheel[pos].move_to_work(set);
			
			// 1.1.2.add repeated task entry.
			for (auto it = set.begin(); it != set.end(); ++it)
			{
				TaskEntry::ptr& ent = *it;
				// those follow scene will no need to add repeated task: a/b/c.
				if (!ent->repeated()) continue;
				if (!ent->canceled() && ent.use_count() == 1)
				{
					ent->m_expire += ent->m_expire_dur;
					run_task(*ent->m_wheel, TaskEntry::ptr(ent));
					continue;
				}
				// [use_count!=1] scene:
				// a.restarting task; b.canceling task; c.a restart task.
				ent.reset();
			}// end for.
		}

	private:
		uint32_t m_tick;	// tick position of wheel.
		uint32_t m_prec;	// timer precision milliseconds.
		uint64_t m_time;	// time of the tick.
		// wheel bucket lock.
		uint32_t m_size;	// wheel size.
		Bucket* m_wheel;

		// parent timer wheel.
		std_mutex m_mutex_parent;
		std::unique_ptr<Wheel> m_parent;
	};

public:
	// timer wheel size.
	enum
	{
		def_wheel_size = 30,
		def_prec_mills = 100,
	};

	inline TimerWheelT() : m_start(0)
	{}

	inline ~TimerWheelT()
	{
		release();
	}

	inline void release()
	{
		m_wheel.release();
	}

	inline static uint64_t now_ms()
	{
		auto pt = std_chrono::system_clock::now();
		auto time = std_chrono::duration_cast<
			std_chrono::milliseconds>(pt.time_since_epoch());
		return time.count();
	}
	
	// start timer wheel.
	inline void start(
		uint64_t time = 0,
		uint32_t prec = def_prec_mills,
		uint32_t size = def_wheel_size)
	{
		if (!m_wheel.empty())	// can be only init once.
			ECO_THROW("timer wheel size cann't be reset.");
		if (time == 0) time = now_ms();
		if (prec == 0) prec = def_prec_mills;
		if (size == 0) size = def_wheel_size;
		m_start = time;
		m_wheel.init(time, prec, size);
	}

	// get precision milliseconds.
	inline uint32_t prec_ms() const
	{
		return m_wheel.prec_ms();
	}

	// get wheel milliseconds.
	inline uint32_t wheel_ms() const
	{
		return m_wheel.wheel_ms();
	}

	// get wheel start milliseconds.
	inline uint64_t start_ms() const
	{
		return m_start;
	}

	// wheel size.
	inline uint32_t size() const
	{
		return m_wheel.size();
	}

	/*@add timer to wheel after millseconds duration.
	* @expire: when timer trigger, the duration time from now.
	* @repeat: a repeated timer.
	*/
	inline Timer run_after(
		task_t&& task, uint32_t expire_ms, bool repeat)
	{
		reader_lock lock(m_mutex_wheel);
		return m_wheel.run_after(*this, task, expire_ms, repeat);
	}
	inline Timer run_after(
		const task_t& task, uint32_t expire_ms, bool repeat)
	{
		reader_lock lock(m_mutex_wheel);
		return m_wheel.run_after(*this, task_t(task), expire_ms, repeat);
	}

	/*@add timer to wheel after chrono time duration.
	* @expire: when timer trigger, the duration time from now.
	* @repeat: is a repeated timer.
	*/
	inline Timer run_after(
		IN task_t&& task,
		IN const std_chrono::milliseconds& expire, IN bool repeat = true)
	{
		reader_lock lock(m_mutex_wheel);
		uint32_t expire_dur = (uint32_t)expire.count();
		if (expire_dur <= 0) return Timer();
		return m_wheel.run_after(*this, task, expire_dur, repeat);
	}
	inline Timer run_after(
		IN const task_t& task,
		IN const std_chrono::milliseconds& expire, IN bool repeat = true)
	{
		reader_lock lock(m_mutex_wheel);
		return m_wheel.run_after(*this, task_t(task), expire, repeat);
	}

	/*@add timer to wheel at chrono time point.
	* @tp: chrono time point.
	*/
	inline Timer run_at(
		IN task_t&& task,
		IN const std_chrono::system_clock::time_point& tp)
	{
		reader_lock lock(m_mutex_wheel);
		return m_wheel.run_at(*this, task, tp);
	}
	inline Timer run_at(
		IN const task_t& task,
		IN const std_chrono::system_clock::time_point& tp)
	{
		reader_lock lock(m_mutex_wheel);
		return m_wheel.run_at(*this, task_t(task), tp);
	}
	inline Timer run_at(IN task_t&& task, IN const char* tp)
	{
		reader_lock lock(m_mutex_wheel);
		return m_wheel.run_at(*this, task, eco::date_time::make_time_point(tp));
	}
	inline Timer run_at(IN const task_t& task, IN const char* tp)
	{
		reader_lock lock(m_mutex_wheel);
		return m_wheel.run_at(*this, task_t(task),
			eco::date_time::make_time_point(tp));
	}

	/*@add timer to wheel at daily time.
	* @time: time_point of daily.
	*/
	inline Timer run_daily(IN task_t&& task, IN const date_time::Time& time)
	{
		reader_lock lock(m_mutex_wheel);
		return m_wheel.run_daily(*this, task, time);
	}
	inline Timer run_daily(IN const task_t& task, const date_time::Time& time)
	{
		reader_lock lock(m_mutex_wheel);
		return m_wheel.run_daily(*this, task_t(task), time);
	}

	// rolling the timer wheel to the time point.
	inline void roll(const std_chrono::system_clock::time_point& now)
	{
		auto time = std_chrono::duration_cast<
			std_chrono::milliseconds>(now.time_since_epoch());
		roll(time.count());
	}

	// rolling the timer wheel to the time point.
	inline void roll(uint64_t now)
	{
		uint32_t tik = m_wheel.count_tick(now);
		for (uint32_t i = 0; i < tik; ++i)
		{
			m_wheel.step(*this);
		}
	}

private:
	// restart timer of task entry.
	inline bool run_restart(typename TaskEntry::ptr& ent)
	{
		reader_lock lock(m_mutex_wheel);
		if (ent->canceled()) return false;		// canceled timer can't be restart.

		// avoid call "run_restart" repeatedly in the same bucket time.
		// exp: call 100 times in one prec.
		uint64_t expire = now_ms() + ent->m_expire_dur;
		auto tik_now = m_wheel.get_tick(expire);
		auto tik_old = m_wheel.get_tick(ent->m_expire);
		if (tik_now == tik_old)	return false;	// in the same bucket.

		ent->m_expire = expire;
		m_wheel.run_task(*this, ent);
		return true;
	}

	// cancel timer of task entry.
	inline void run_cancel(typename TaskEntry::ptr& ent)
	{
		writer_lock lock(m_mutex_wheel);
		ent->cancel();
	}

	// clock drive this timer wheel.
	friend class Timer;
	friend class Wheel;
	uint64_t		m_start;
	Wheel			m_wheel;
	shared_mutex	m_mutex_wheel;
};
typedef TimerWheelT<Closure> TimerWheel;


////////////////////////////////////////////////////////////////////////////////
template<typename task_t, typename task_handler_t = eco::Handler>
class TimingWheelT
{
	ECO_OBJECT(TimingWheelT);
public:
	typedef TimerWheelT<task_t, task_handler_t> timer_wheel;
	typedef typename timer_wheel::Timer Timer;

	inline TimingWheelT()
	{}

	inline ~TimingWheelT()
	{
		stop();
	}

	// get tick precision milliseconds.
	inline uint32_t prec_ms() const
	{
		return m_wheel.prec_ms();
	}

	// wheel total cycle milliseconds.
	inline uint32_t wheel_ms() const
	{
		return m_wheel.wheel_ms();
	}

	// get wheel start milliseconds.
	inline uint64_t start_ms() const
	{
		return m_wheel.start_ms();
	}

	// wheel size.
	inline uint32_t size() const
	{
		return m_wheel.size();
	}

	/*@start timing wheel and thread.
	* @prec_mill: tick precision, millisecond unit.
	* @wheel_siz: size of wheel.
	*/
	inline void start(
		IN const char* name,
		IN uint32_t prec_mill = TimerWheel::def_prec_mills,
		IN uint32_t wheel_siz = TimerWheel::def_wheel_size)
	{
		m_wheel.start(0, prec_mill, wheel_siz);
		m_running.set_ok(true);
		m_thread.run(std::bind(&TimingWheelT::work, this), name);
	}

	// stop wheel.
	inline void stop()
	{
		m_running.set_ok(false);
		m_thread.join();
		m_wheel.release();
	}

	// timer wheel.
	timer_wheel& get_wheel()
	{
		return m_wheel;
	}
	const timer_wheel& wheel() const
	{
		return m_wheel;
	}

	/*@add timer to wheel after millseconds duration.
	* @expire: when timer trigger, the duration time from now.
	* @repeat: a repeated timer.
	*/
	inline typename timer_wheel::Timer run_after(
		task_t&& task, uint32_t expire_ms, bool repeat = false)
	{
		return m_wheel.run_after(std::forward<task_t>(task), expire_ms, repeat);
	}
	inline typename timer_wheel::Timer run_after(
		const task_t& task, uint32_t expire_ms, bool repeat = false)
	{
		return m_wheel.run_after(task, expire_ms, repeat);
	}

	/*@add timer to wheel after chrono time duration.
	* @expire: when timer trigger, the duration time from now.
	* @repeat: is a repeated timer.
	*/
	inline typename timer_wheel::Timer run_after(
		IN task_t&& task,
		IN const std_chrono::milliseconds& expire, IN bool repeat = false)
	{
		return m_wheel.run_after(std::forward<task_t>(task), expire, repeat);
	}
	inline typename timer_wheel::Timer run_after(
		IN const task_t& task,
		IN const std_chrono::milliseconds& expire, IN bool repeat = false)
	{
		return m_wheel.run_after(task, expire, repeat);
	}

	/*@add timer to wheel at chrono time point.
	* @expire: when timer trigger, the duration time from now.
	* @repeat: is a repeated timer.
	*/
	inline typename timer_wheel::Timer run_at(
		IN task_t&& task,
		IN const std_chrono::system_clock::time_point& tp)
	{
		return m_wheel.run_at(std::forward<task_t>(task), tp);
	}
	inline typename timer_wheel::Timer run_at(
		IN const task_t& task,
		IN const std_chrono::system_clock::time_point& tp)
	{
		return m_wheel.run_at(task, tp);
	}
	inline Timer run_at(IN task_t&& task, IN const char* tp)
	{
		return m_wheel.run_at(task, tp);
	}
	inline Timer run_at(IN const task_t& task, IN const char* tp)
	{
		return m_wheel.run_at(task, tp);
	}

	/*@add timer to wheel at daily time.
	* @time: time_point of daily.
	*/
	inline Timer run_daily(IN task_t&& task, IN const date_time::Time& time)
	{
		return m_wheel.run_daily(std::forward<task_t>(task), time);
	}
	inline Timer run_daily(IN const task_t& task, IN const date_time::Time& time)
	{
		return m_wheel.run_daily(task, time);
	}

	// rolling the timer wheel to the time point.
	inline void roll(const std_chrono::system_clock::time_point& now)
	{
		m_wheel.roll(now);
	}

	// rolling the timer wheel to the time point.
	inline void roll(uint64_t now)
	{
		m_wheel.roll(now);
	}

private:
	// trigger tick event.
	inline void work()
	{
		while (m_running.ok1())
		{
			eco::this_thread::sleep(m_wheel.prec_ms());
			m_wheel.roll(std_chrono::system_clock::now());
		}
	}

	eco::Thread m_thread;
	timer_wheel m_wheel;
	eco::atomic::State m_running;
};
typedef TimingWheelT<Closure> TimingWheel;


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco)
#endif