#ifndef ECO_THREAD_BOBJECT_H
#define ECO_THREAD_BOBJECT_H
/*******************************************************************************
@ name
business object.
@ function
1.apply scene: client api(like ctp\esunny) control request once a time.
2.apply scene: all people just finish a task, who ocuppy first who do it.
@ exception
@ note
--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.
--------------------------------------------------------------------------------
* copyright(c) 2017 - 2019, ujoy, reserved all right.
*******************************************************************************/
#include <eco/Prec.h>
#include <eco/std/mutex.h>
#include <eco/thread/State.h>


namespace eco{;
class Btask;
////////////////////////////////////////////////////////////////////////////////
enum TaskState
{
	// 任务未就绪（执行Prepare任务）
	task_no_ready		= 0x01,
	// 任务可执行（任务线程执行当前任务）
	task_occupied		= 0x02,
	// 任务已完成，不需要再执行。（不处理）
	task_finished		= 0x03,
	// 正在执行相同任务，所以不用再执行当前任务。（不处理）
	task_working_self	= 0x04,
	// 正在执行其他任务，该任务与当前任务无关联。（不处理）
	task_working_other	= 0x05,
};
enum
{
	// task type must start from 1
	task_type_start = 0x00000001,
};

////////////////////////////////////////////////////////////////////////////////
class Bobject
{
	ECO_OBJECT(Bobject);
public:
	class Lock;
	class Relock;

	inline Bobject()
	{}

	template<typename T>
	inline T& cast()
	{
		return *static_cast<T*>(this);
	}

	// occupy this business object.
	inline TaskState occupy(
		IN std::shared_ptr<Btask> task,
		IN const uint32_t task_sour_state,
		IN const uint32_t task_dest_state,
		IN const uint32_t task_erase_state)
	{
		// if task is finished.
		if (m_state.has(task_dest_state))
		{
			return task_finished;
		}

		// if task is just not ready.
		if (task_sour_state != 0 && !m_state.has(task_sour_state))
		{
			return task_no_ready;
		}

		std_lock_guard lock(mutex());
		// if task is working by other task.
		if (m_working_task)
		{
			return (m_working_task.get() == task.get())
				? task_working_self : task_working_other;
		}
		m_working_task = task;
		return task_occupied;
	}

	// business object state.
	inline eco::atomic::State& state()
	{
		return m_state;
	}
	inline const eco::atomic::State& get_state() const
	{
		return m_state;
	}

	// release the occupied state.
	inline void release()
	{
		m_working_task.reset();
	}

	// finish the occupied state.
	inline void finish(
		IN const uint32_t task_dest_state,
		IN const uint32_t task_erase_state)
	{
		m_state.add(task_dest_state);
		m_state.del(task_erase_state);
		release();
	}

	// mutex of business object.
	inline std_mutex& mutex() const
	{
		return m_mutex;
	}

private:
	mutable std_mutex m_mutex;
	eco::atomic::State m_state;
	std::shared_ptr<Btask> m_working_task;
};


////////////////////////////////////////////////////////////////////////////////
class Bobject::Lock
{
	ECO_OBJECT(Lock);
public:
	inline Lock(IN Bobject* object = nullptr)
		: m_object(object)
		, m_data_occupy(task_no_ready)
	{}

	inline Lock(IN Lock&& rr)
		: m_object(rr.m_object)
		, m_data_occupy(rr.m_data_occupy)
	{
		rr.clear();
	}

	inline Lock& operator=(IN Lock&& rr)
	{
		m_object = rr.m_object;
		m_data_occupy = rr.m_data_occupy;
		rr.clear();
		return *this;
	}

	inline ~Lock()
	{
		if (m_data_occupy == task_occupied && m_object != nullptr)
		{
			m_object->release();
		}
	}

	// occupy this data.
	inline bool occupy(
		IN std::shared_ptr<Btask>& task,
		IN const uint32_t task_sour_state,
		IN const uint32_t task_dest_state,
		IN const uint32_t task_erase_state)
	{
		if (m_object != nullptr)
		{
			m_data_occupy = m_object->occupy(task,
				task_sour_state, task_dest_state, task_erase_state);
		}
		return m_data_occupy == task_occupied;
	}

	// whether occupied this data.
	inline bool occupied() const
	{
		return m_data_occupy == task_occupied;
	}

	// finish the task.
	inline void cancel_release()
	{
		clear();
	}

	// finish the task.
	inline void finish(
		IN const uint32_t task_dest_state,
		IN const uint32_t task_erase_state)
	{
		if (m_data_occupy == task_occupied && m_object != nullptr)
		{
			m_object->finish(task_dest_state, task_erase_state);
			clear();
		}
	}

protected:
	inline void clear()
	{
		m_object = nullptr;
		m_data_occupy = task_no_ready;
	}

	Bobject* m_object;
	uint32_t m_data_occupy;
};


////////////////////////////////////////////////////////////////////////////////
class Bobject::Relock
{
	ECO_OBJECT(Relock);
public:
	inline Relock(
		IN Bobject& bo,
		IN const uint32_t is_release = true)
		: m_release(is_release)
		, m_object(bo)
	{}

	inline ~Relock()
	{
		if (m_release)
		{
			m_object.release();
		}
	}

	inline void finish(uint32_t task_dest_state, uint32_t task_erase_state)
	{
		if (m_release)
		{
			m_object.finish(task_dest_state, task_erase_state);
			m_release = false;
		}
	}

private:
	uint32_t m_release;
	Bobject& m_object;
};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif