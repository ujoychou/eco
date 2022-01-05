#ifndef ECO_THREAD_TASK_H
#define ECO_THREAD_TASK_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy created on 2019-05-09.
1.init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Object.h>
#include <eco/Eco.h>
#include <functional>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
// task interface.
#define ECO_TASK_I(type_t, parent_t) ECO_TYPE_2(type_t, parent_t)\
ECO_OBJECT(type_t)

// task & btask.
#define ECO_TASK_2(type_t, parent_t) ECO_TASK_I(type_t, parent_t) \
public: inline static type_t::ptr create() { return type_t::ptr(new type_t());}
#define ECO_TASK_3(task_t, parent_t, prepare_t) ECO_TASK_2(task_t, parent_t)\
public: virtual void prepare() override { prepare_t::create()->start(); }
#define ECO_TASK(...) ECO_MACRO(ECO_TASK_,__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
class Task : public std::enable_shared_from_this<Task>
{
	ECO_TYPE(Task);
	ECO_OBJECT(Task);
public:
	enum { restart_secs_default = 5 };

	// init task.
	inline Task(IN uint32_t secs = restart_secs_default)
	{
		set_restart_secs(secs);
	}
	virtual ~Task()	{}

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

	// start task.
	virtual void start()
	{
		Task::ptr task = shared_from_this();
		Eco::get().post_task(task);
	}

	// if task throw exception mean fail, and it will be restart again.
	virtual void operator()(void) = 0;

	// start task.
	static inline void start(Closure&& task)
	{
		Eco::get().post_task(std::move(task));
	}

private:
	uint32_t m_restart_secs;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif