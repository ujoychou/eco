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
	inline Case(IN const std::string& name) 
		: Runner(name)
	{}

	virtual ~Case()
	{}

protected:
	virtual void operator()(void)
	{}

	virtual uint32_t CountCase() const
	{
		return 1;
	}

	virtual void OnBegin() override
	{
		Runner::OnBegin();
		GetResultSet().ResetTest();

		EcoTrace << "[run       ] " << GetFullName() << ".";
	}

	virtual void OnEnd() override
	{
		Runner::OnEnd();

		// update case info.
		if (!GetResultSet().IsOk())
		{
			GetResultSet().m_failed_case = 1;
		}
		GetResultSet().m_checked_case = 1;

		eco::FixStream fmt;
		if (GetResultSet().IsOk())
		{
			fmt << "[        ok] ";
		}
		else
		{
			fmt << "[       bad] ";
		}
		fmt << "test ";
		fmt << GetResultSet().GetTotalTest() << "-";
		fmt << GetResultSet().GetCheckedTest() << "-";
		fmt << GetResultSet().GetFailedTest() << "-";
		fmt << GetResultSet().GetSuccessTest() << ".";
		EcoTrace << fmt.c_str();

		if (!GetResultSet().IsOk())
		{
			fmt.buffer().clear();
			fmt << "[~~~~~~~~~~] " << GetFullName() << ".";
			EcoTrace << fmt.c_str();
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
		m_object->SetContext(GetContext());
		m_object->GetResultSet() = GetResultSet();
		m_object->SetName(GetName());

		m_case_run();	// run work.

		GetResultSet() = m_object->GetResultSet();
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