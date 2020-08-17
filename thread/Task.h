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
// use for cast closure function to task.
class Task;
class TaskUnit
{
public:
	typedef std::shared_ptr<Task> TaskPtr;

	inline TaskUnit()
	{}

	inline TaskUnit(TaskPtr& task) : m_task(task)
	{}

	inline TaskUnit(Closure& func) : m_func(func)
	{}

	inline TaskUnit(TaskPtr&& task) : m_task(std::forward<TaskPtr>(task))
	{}

	inline TaskUnit(Closure&& func) : m_func(std::forward<Closure>(func))
	{}

	inline TaskUnit(const TaskUnit& unit)
		: m_func(unit.m_func)
		, m_task(unit.m_task)
	{}

	inline TaskUnit(TaskUnit&& unit)
		: m_func(std::move(unit.m_func))
		, m_task(std::move(unit.m_task))
	{}

	inline void operator()(void);

	inline TaskUnit& operator=(TaskUnit&& unit)
	{
		m_func = std::move(unit.m_func);
		m_task = std::move(unit.m_task);
		return *this;
	}

	inline TaskUnit& operator=(const TaskUnit& unit)
	{
		m_func = unit.m_func;
		m_task = unit.m_task;
		return *this;
	}

	inline const char* get_type_name() const;

	inline uint32_t restart_secs() const;

	template<typename task_t>
	inline task_t* cast()
	{
		return (task_t*)m_task.get();
	}

private:
	Closure m_func;
	TaskPtr m_task;
};


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
	inline uint32_t get_restart_secs() const
	{
		return m_restart_secs;
	}

	// start task.
	virtual void start()
	{
		Eco::get().post_task(TaskUnit(
			std::dynamic_pointer_cast<Task>(shared_from_this())));
	}

	// if task throw exception mean fail, and it will be restart again.
	virtual void operator()(void) = 0;

	// start task.
	static inline void start(Closure&& task)
	{
		Eco::get().post_task(TaskUnit(std::forward<Closure>(task)));
	}

private:
	uint32_t m_restart_secs;
};


////////////////////////////////////////////////////////////////////////////////
inline void TaskUnit::operator()(void)
{
	return m_task ? (*m_task)() : m_func();
}
inline const char* TaskUnit::get_type_name() const
{
	return m_task ? m_task->get_type_name() : "closure";
}
inline uint32_t TaskUnit::restart_secs() const
{
	return m_task ? m_task->restart_secs() : Task::restart_secs_default;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif