#ifndef ECO_TEST_CASE_H
#define ECO_TEST_CASE_H
/*******************************************************************************
@ name

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-06-18.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/test/Runner.h>
#include <eco/test/Object.h>
#include <eco/test/Trace.h>
#include <eco/log/Log.h>


namespace eco{;
namespace test{;
////////////////////////////////////////////////////////////////////////////////
class Case : public Runner
{
public:
	// constructor.
	inline Case()
	{}

	// constructor.
	inline Case(IN const std::string& name) : Runner(name)
	{}

	virtual ~Case()
	{}

protected:
	virtual void operator()(void)
	{}

	virtual uint32_t count_case() const override
	{
		return 1;
	}

	virtual void on_begin() override
	{
		Runner::on_begin();
		result_set().ResetTest();
		ECO_INFO << "[run       ] " << fullname() << ".";
	}

	virtual void on_end() override
	{
		Runner::on_end();

		// update case info.
		if (!result_set().ok())
		{
			result_set().m_failed_case = 1;
		}
		result_set().m_checked_case = 1;

		eco::String fmt;
		if (result_set().ok())
		{
			fmt << "[        ok] ";
		}
		else
		{
			fmt << "[       bad] ";
		}
		fmt << "test ";
		fmt << result_set().total_test() << "-";
		fmt << result_set().checked_test() << "-";
		fmt << result_set().failed_test() << "-";
		fmt << result_set().success_test() << ".";
		ECO_INFO << fmt.c_str();

		if (!result_set().ok())
		{
			fmt.clear();
			fmt << "[~~~~~~~~~~] " << fullname() << ".";
			ECO_INFO << fmt.c_str();
		}// end if.
	}
};


class CaseImpl : public Case
{
public:
	CaseImpl(
		IN const std::string& case_name,
		IN RunnerFunc case_run,
		IN std::shared_ptr<Object>& obj)
		: Case(case_name)
		, m_case_run(case_run)
		, m_object(obj)
	{}

	virtual void operator()(void) override
	{
		m_object->set_context(get_context());
		m_object->result_set() = result_set();
		m_object->set_name(name());

		m_case_run();	// run work.

		result_set() = m_object->result_set();
	}

private:
	RunnerFunc m_case_run;
	std::shared_ptr<Object> m_object;
};

////////////////////////////////////////////////////////////////////////////////
#define ECO_TEST_AUTO_REGISTER_CASE(xx)


////////////////////////////////////////////////////////////////////////////////
}}
#endif