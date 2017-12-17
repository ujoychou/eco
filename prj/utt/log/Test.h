#ifndef ECO_CMD_TEST_H
#define ECO_CMD_TEST_H
/*******************************************************************************
@ 名称

@ 功能

@ 异常

@ 备注

--------------------------------------------------------------------------------
@ 历史记录 @
@ ujoy modifyed on 2016-10-29

--------------------------------------------------------------------------------
* 版权所有(c) 2015 - 2017, ujoychou, 保留所有权利。

*******************************************************************************/
#include <eco/App.h>
#include <eco/thread/ThreadPool.h>


namespace eco{;
namespace log{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class MtCommand : public eco::cmd::Command
{
	ECO_COMMAND(MtCommand, "mt", "mt");
public:
	virtual void execute(
		IN const eco::cmd::Context& context) override;

private:
	void log_level(
		IN size_t loop_times);

	void log_domain(
		IN size_t loop_times,
		IN const char* domain);

private:
	eco::ThreadPool m_thread_pool;
};


////////////////////////////////////////////////////////////////////////////////
class FuncCommand : public eco::cmd::Command
{
	ECO_COMMAND(FuncCommand, "func", "fc");
public:
	virtual void execute(
		IN const eco::cmd::Context& context) override;

private:
	void log_test(
		IN const char* level,
		IN const char* domain);

private:
	eco::ThreadPool m_thread_pool;
};


}}}
#endif