#ifndef ECO_THREAD_BTASK_H
#define ECO_THREAD_BTASK_H
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
#include <eco/log/Log.h>
#include <eco/thread/Task.h>
#include <eco/thread/Bobject.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
// only support single business object.
class Btask : public Task
{
	ECO_TASK_I(Btask, Task);
public:
	// get business object.
	virtual eco::Bobject& bobject() { return bob(); }

	// occupy this business object.
	virtual TaskState occupy()
	{
		return bobject().occupy(shared_from_this(),
			m_sour_state, m_add_state, m_erase_state);
	}

	// execute pre_task operation.
	virtual void prepare() {}

	// do execute task operation.
	virtual void execute() = 0;

	// task operator()
	virtual void operator()(void) override
	{
		eco::Bobject::Relock relock(bobject());
		// if finished this task, set state and notify wait task to restart.
		try
		{
			execute();
			relock.finish(m_add_state, m_erase_state);
			Eco::get().move_wait();
		}
		catch (std::exception& e)
		{
			ECO_LOG(error, "btask") < e.what();
			if (restart_secs() > 0)
			{
				Btask::ptr this_ptr = shared_from_this();
				Eco::get().timer().run_after([=]() mutable {
					Eco::get().post_btask(this_ptr);
				}, restart_secs() * 1000, false);
			}
			return;
		}
	}

	// task start.
	virtual void start() override
	{
		// post btask to queue.
		TaskState task_state = occupy();
		if (task_state == task_no_ready || task_state == task_working_other)
		{
			prepare();
			Eco::get().post_wait(shared_from_this());
		}
		else if (task_state == task_occupied)
		{
			Eco::get().post_btask(shared_from_this());
		}
	}

public:
	// state btask.
	inline Btask(
		IN uint32_t sour_state,
		IN uint32_t add_state,
		IN uint32_t erase_state = 0,
		IN uint32_t restart_secs = restart_secs_default)
		: Task(restart_secs > 0 ? restart_secs : restart_secs_default)
		, m_sour_state(sour_state)
		, m_add_state(add_state)
		, m_erase_state(erase_state)
	{}

	// state btask with no sour_state.
	inline Btask(IN uint32_t dest_state)
		: m_sour_state(0)
		, m_add_state(dest_state)
		, m_erase_state(0)
	{}

	inline Btask::ptr shared_from_this()
	{
		return std::dynamic_pointer_cast<Btask>(Task::shared_from_this());
	}

	// get instance of eco::Bobject.
	static inline eco::Bobject& bob()
	{
		static eco::Bobject s_bobject;
		return s_bobject;
	}

	// init task source and destination state.
	inline void set_state(
		IN uint32_t sour_state,
		IN uint32_t add_state,
		IN uint32_t erase_state = 0)
	{
		m_sour_state = sour_state;
		m_add_state = add_state;
		m_erase_state = erase_state;
	}
	// task dest state.
	inline const uint32_t get_add_state() const
	{
		return m_add_state;
	}
	inline const uint32_t get_sour_state() const
	{
		return m_sour_state;
	}

private:
	uint32_t m_sour_state;
	uint32_t m_add_state;
	uint32_t m_erase_state;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif