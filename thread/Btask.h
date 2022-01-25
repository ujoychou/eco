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
#include <eco/Rtype.h>
#include <eco/log/Log.h>
#include <eco/thread/Eco.h>
#include <eco/thread/Work.h>
#include <eco/thread/Bobject.h>


////////////////////////////////////////////////////////////////////////////////
#define ECO_TASK_2(type_t, parent_t) \
ECO_OBJECT(type_t) \
ECO_TYPE_2(type_t, parent_t) \
public: inline static type_t::ptr create() { return type_t::ptr(new type_t());}

#define ECO_TASK_3(task_t, parent_t, prepare_t) \
ECO_TASK_2(task_t, parent_t) \
public: virtual void prepare() override { prepare_t::create()->start(); }
#define ECO_TASK(...) ECO_MACRO(ECO_TASK_,__VA_ARGS__)


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
// only support single business object.
class Btask : public std::enable_shared_from_this<Btask>
{
	ECO_TYPE(Btask);
	ECO_OBJECT(Btask);
public:
	enum { restart_secs_default = 5 };

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
	virtual void operator()(void)
	{
		eco::Bobject::Relock relock(bobject());

		// if finished this task, set state and notify wait task to restart.
		bool succ = eco::work_with_catch_exception(get_type_name(), [&](){
			execute();
			relock.finish(m_add_state, m_erase_state);
			Eco::get().move_wait();
		});
		
		// restart task.
		if (!succ && m_restart_secs > 0)
		{
			Btask::ptr this_ptr = shared_from_this();
			Eco::get().timer().run_after([=]() mutable {
				Eco::get().post_task(std::move(this_ptr));
			}, restart_secs() * 1000, false);
		}// end if.
	}

	// task start.
	virtual void start()
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
			Eco::get().post_task(shared_from_this());
		}
	}

public:
	// state btask.
	inline Btask(
		IN uint32_t sour_state,
		IN uint32_t add_state,
		IN uint32_t erase_state = 0,
		IN uint32_t restart_secs = restart_secs_default)
		: m_sour_state(sour_state)
		, m_add_state(add_state)
		, m_erase_state(erase_state)
		, m_restart_secs(restart_secs)
	{}

	// state btask with no sour_state.
	inline Btask(IN uint32_t dest_state)
		: m_sour_state(0)
		, m_add_state(dest_state)
		, m_erase_state(0)
		, m_restart_secs(restart_secs_default)
	{}

	// set task restart secs.
	// restart interval second when task fail(operator() will throw exception).
	inline void set_restart_secs(IN uint32_t secs = restart_secs_default)
	{
		m_restart_secs = (secs > 0 ? secs : restart_secs_default);
	}
	inline uint32_t restart_secs() const
	{
		return m_restart_secs;
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
	uint32_t m_restart_secs;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif