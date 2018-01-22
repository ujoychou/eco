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
#include <eco/Bobject.h>
#include <eco/log/Log.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
#define ECO_TASK(task_t) \
public:\
	ECO_TYPE(task_t)\
	virtual std::auto_ptr<Btask> copy() const override\
	{\
		return std::auto_ptr<Btask>(new task_t(*this));\
	}

////////////////////////////////////////////////////////////////////////////////
// only support single business object.
class ECO_API Btask
{
public:
	// task type.
	virtual const char* get_type() const = 0;

	// task copy constructor.
	virtual std::auto_ptr<Btask> copy() const = 0;

	// get business object.
	virtual eco::Bobject& bobject() = 0;

	// occupy business object.
	// call: "bobject().occupy(m_sour_state, m_dest_state, m_erase_state);"
	virtual const eco::TaskState occupy();

	// execute pre_task operation.
	virtual void prepare();

	// do execute task operation.
	virtual bool execute() = 0;

public:
	enum { v_restart_secs = 5 };

	inline Btask(
		IN const uint32_t sour_state,
		IN const uint32_t dest_state,
		IN const uint32_t erase_state = 0,
		IN const uint32_t restart_secs = v_restart_secs)
		: m_sour_state(sour_state)
		, m_dest_state(dest_state)
		, m_erase_state(erase_state)
		, m_restart_secs(restart_secs > 0 ? restart_secs : v_restart_secs)
	{}

	inline explicit Btask(
		IN const uint32_t restart_secs = v_restart_secs)
		: m_sour_state(0)
		, m_dest_state(0)
		, m_erase_state(0)
		, m_restart_secs(restart_secs > 0 
		? restart_secs : v_restart_secs)
	{}

	// async execute this task.
	inline void start()
	{
		TaskState task_state = occupy();
		if (task_state == task_no_ready || 
			task_state == task_working_other)
		{
			prepare();
			post_wait(*this);
		}
		else if (task_state == task_occupied)
		{
			post_task(*this);
		}
	}

	// task operator()
	inline void operator()(void)
	{
		eco::Bobject::Relock relock(bobject());

		// execute this task.
		bool result = true;
		try
		{
			result = execute();
		}
		catch (eco::Error& e)
		{
			result = false;
			EcoError << get_type() << ": " << e;
		}
		catch (std::exception& e)
		{
			result = false;
			EcoError << get_type() << ": " << e.what();
		}

		// if finished this task, set state and notify wait task to restart.
		// else restart this task intervally.
		if (result)
		{
			relock.finish(m_dest_state, m_erase_state);
			move_wait();
		}
		else
		{
			set_timer(m_restart_secs * 1000, *this);
		}
	}

	// task dest state.
	inline const uint32_t get_dest_state() const
	{
		return m_dest_state;
	}
	inline const uint32_t get_sour_state() const
	{
		return m_sour_state;
	}

private:
	uint32_t m_sour_state;
	uint32_t m_dest_state;
	uint32_t m_erase_state;
	uint32_t m_restart_secs;

private:
	void post_task(IN const Btask& task);
	void post_wait(IN const Btask& task);
	void move_wait();
	void set_timer(IN const uint32_t restart_millsecs, IN const Btask& task);
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif