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



ECO_NS_BEGIN(eco);
namespace test{;
////////////////////////////////////////////////////////////////////////////////
class Runner : public eco::test::Object
{
public:
	typedef std::list<std::shared_ptr<Runner>> RunnerChildren;

public:
	// update test result set.
	inline void count_result_set()
	{
		// update result tests.
		if (!has_child()) return;
		for (auto it = children()->begin(); it != children()->end(); ++it)
		{
			// count result set: tests.
			result_set().m_total_test += (**it).result_set().m_total_test;
			result_set().m_checked_test += (**it).result_set().m_checked_test;
			result_set().m_failed_test += (**it).result_set().m_failed_test;

			// count result set: case.
			result_set().m_failed_case += (**it).result_set().m_failed_case;
			result_set().m_checked_case += (**it).result_set().m_checked_case;

			// count result set: child scene.
			result_set().m_failed_scene += (**it).result_set().m_failed_scene;
			result_set().m_checked_scene += (**it).result_set().m_checked_scene;
		}
	}

public:
	inline bool operator==(IN const std::string& name) const
	{
		return Object::operator==(name);
	}

	// get children runner list.
	inline RunnerChildren& get_children()
	{
		const Runner& runner = (*this);
		return (RunnerChildren&)(*runner.children());
	}

	// whether has a child.
	inline bool has_child() const
	{
		auto* c = children();
		return c && !c->empty();
	}

	// set owner scene.
	inline void set_owner(IN Runner& _owner)
	{
		m_owner = &_owner;
	}

	// get owner scene.
	inline Runner* get_owner()
	{
		return m_owner;
	}
	inline const Runner* owner() const
	{
		return m_owner;
	}

	// get owner scene string.
	inline std::string fullname() const
	{
		std::string name(name());
		const Runner* obj = owner();
		while (obj != nullptr && !obj->name().empty())
		{
			name = obj->name() + "." + name;
			obj = obj->owner();
		}
		return name;
	}

	// run method.
	// this method will throw unexcepted exceptions.
	inline void run()
	{
		try
		{
			on_begin();

			// do test case work.
			(*this)();

			on_end();
		}
		catch (std::exception& err)
		{
			on_error();
			err.what();	// for warning.
		}
	}

////////////////////////////////////////////////////////////////////////////////
public:
	// count test case total number.
	virtual uint32_t count_case() const
	{
		uint32_t count = 0;
		if (!has_child()) return count;
		for (auto it = children()->begin(); it != children()->end(); ++it)
		{
			count += it->get()->count_case();
		}
		return count;
	}

	// count test scene total number.
	virtual uint32_t count_scene() const
	{
		return 0;
	}

	// get children runner list.
	virtual const RunnerChildren* children() const
	{
		return nullptr;
	}

	// do test case work.
	virtual void operator()(void)
	{
		if (!has_child()) return;
		for (auto it = get_children().begin(); it != get_children().end(); ++it)
		{
			(**it).set_context(get_context());
			(**it).run();
		}
	}


////////////////////////////////////////////////////////////////////////// EVENT
public:
	virtual void on_begin()
	{
		m_res_set.reset();
		m_res_set.m_total_case = count_case();
		m_res_set.m_total_scene = count_scene();
	}

	virtual void on_end()
	{
		count_result_set();
	}

	virtual void on_error()
	{}

////////////////////////////////////////////////////////////////////////////////
protected:
	// constructor.
	inline Runner() : m_owner(nullptr)
	{}

	// constructor.
	inline Runner(IN const std::string& name) 
		: Object(name), m_owner(nullptr)
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