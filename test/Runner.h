#ifndef ECO_TEST_RUNNER_H
#define ECO_TEST_RUNNER_H
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
#include <eco/test/Object.h>
#include <eco/test/Check.h>



namespace eco{;
namespace test{;
////////////////////////////////////////////////////////////////////////////////
class Runner : public eco::test::Object
{
public:
	typedef std::list<std::shared_ptr<Runner>> RunnerChildren;

public:
	// update test result set.
	inline void CountResultSet()
	{
		// update result tests.
		RunnerChildren::const_iterator it = GetChildren().begin();
		for (; it!=GetChildren().end(); ++it)
		{
			// count result set: tests.
			GetResultSet().m_total_test += (**it).GetResultSet().m_total_test;
			GetResultSet().m_checked_test += (**it).GetResultSet().m_checked_test;
			GetResultSet().m_failed_test += (**it).GetResultSet().m_failed_test;

			// count result set: case.
			GetResultSet().m_failed_case += (**it).GetResultSet().m_failed_case;
			GetResultSet().m_checked_case += (**it).GetResultSet().m_checked_case;

			// count result set: child scene.
			GetResultSet().m_failed_scene += (**it).GetResultSet().m_failed_scene;
			GetResultSet().m_checked_scene += (**it).GetResultSet().m_checked_scene;
		}
	}

public:
	inline bool operator==(IN const std::string& name) const
	{
		return Object::operator==(name);
	}

	// get children runner list.
	inline RunnerChildren& GetChildren()
	{
		const Runner& runner = (*this);
		return (RunnerChildren&)(runner.GetChildren());
	}

	// whether has a child.
	inline bool HasChild()
	{
		return GetChildren().size() > 0;
	}

	// set owner scene.
	inline void SetOwner(IN Runner& owner)
	{
		m_owner = &owner;
	}

	// get owner scene.
	inline Runner* GetOwner()
	{
		return m_owner;
	}

	// get owner scene string.
	inline std::string GetFullName()
	{
		std::string name(GetName());
		Runner* owner = GetOwner();
		while (owner != nullptr && !owner->GetName().empty())
		{
			name = owner->GetName() + "." + name;
			owner = owner->GetOwner();
		}
		return name;
	}

	// run method.
	// this method will throw unexcepted exceptions.
	inline void Run()
	{
		try
		{
			OnBegin();

			// do test case work.
			(*this)();

			OnEnd();
		}
		catch (std::exception& err)
		{
			OnError();
			err.what();	// for warning.
		}
	}

////////////////////////////////////////////////////////////////////////////////
public:
	// count test case total number.
	virtual uint32_t CountCase() const
	{
		uint32_t count = 0;
		RunnerChildren::const_iterator it = GetChildren().begin();
		for (; it!=GetChildren().end(); ++it)
		{
			count += it->get()->CountCase();
		}
		return count;
	}

	// count test scene total number.
	virtual uint32_t CountScene() const
	{
		return 0;
	}

	// get children runner list.
	virtual const RunnerChildren& GetChildren() const
	{
		return eco::Singleton<RunnerChildren>::instance();
	}

	// do test case work.
	virtual void operator()(void)
	{
		RunnerChildren::iterator child = GetChildren().begin();
		for (; child!=GetChildren().end(); ++child)
		{
			(**child).SetContext(GetContext());
			(**child).Run();
		}
	}


////////////////////////////////////////////////////////////////////////// EVENT
public:
	virtual void OnBegin()
	{
		m_res_set.Reset();
		m_res_set.m_total_case = CountCase();
		m_res_set.m_total_scene = CountScene();
	}

	virtual void OnEnd()
	{
		CountResultSet();
	}

	virtual void OnError()
	{}

////////////////////////////////////////////////////////////////////////////////
protected:
	// constructor.
	inline Runner() : m_owner(nullptr)
	{}

	// constructor.
	inline Runner(IN const std::string& name) 
		: Object(name)
		, m_owner(nullptr)
	{}

	// destructor
	virtual ~Runner()
	{}

private:
	Runner* m_owner;
};

////////////////////////////////////////////////////////////////////////////////
// case running function.
typedef std::function<void (void)> RunnerFunc;

////////////////////////////////////////////////////////////////////////////////
}}
#endif