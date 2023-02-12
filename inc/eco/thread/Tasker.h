#ifndef ECO_TASKER_H
#define ECO_TASKER_H
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
#include <eco/thread/Eco.h>
#include <eco/thread/Worker.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename task_t>
class TaskMessage
{
public:
	task_t m_task;
	int m_restart_sec_if_fail;

	inline TaskMessage() : m_restart_sec_if_fail(0) {}

	inline TaskMessage(task_t&& task, int restart_sec_if_fail)
		: m_task(std::move(task))
		, m_restart_sec_if_fail(restart_sec_if_fail) {}

	inline TaskMessage(const TaskMessage& r)
		: m_task(r.m_task)
		, m_restart_sec_if_fail(r.m_restart_sec_if_fail) {}

	inline TaskMessage(TaskMessage&& r)
		: m_task(std::move(r.m_task))
		, m_restart_sec_if_fail(r.m_restart_sec_if_fail)
	{
		r.m_restart_sec_if_fail = 0;
	}

	inline TaskMessage& operator=(TaskMessage&& r)
	{
		m_task = std::move(r.m_task);
		m_restart_sec_if_fail = r.m_restart_sec_if_fail;
		r.m_restart_sec_if_fail = 0;
		return *this;
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename task_t>
class TaskHandler
{
public:
	void* m_tasker;
	inline TaskHandler() : m_tasker(nullptr) {}
	inline void operator()(TaskMessage<task_t>& msg);
};
class Tasker : public Worker<TaskMessage<eco::Task>, TaskHandler<eco::Task>>
{
public:
	inline Tasker() { this->get_handler().m_tasker = this; }
	inline void post(IN eco::Task&& task, IN uint32_t restart_sec_if_fail)
	{
		typedef TaskMessage<eco::Task> Message;
		typedef Worker<TaskMessage<eco::Task>, TaskHandler<eco::Task>> THIS;
		THIS::post(Message(std::move(task), restart_sec_if_fail));
	}
};
template<typename task_t>
inline void TaskHandler<task_t>::operator()(TaskMessage<task_t>& msg)
{
	if (!eco::work_with_catch_exception([&]() { msg.m_task(); }))
	{
		// restart task, occur very little times.
		Eco::get().timing().run_after([=]() mutable {
			eco::Tasker* tasker = (eco::Tasker*)(m_tasker);
			tasker->post(std::move(msg.m_task), msg.m_restart_sec_if_fail);
		}, msg.m_restart_sec_if_fail * 1000, false);
	}
}
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif