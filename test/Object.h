#ifndef ECO_TEST_OBJECT_H
#define ECO_TEST_OBJECT_H
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
#include <eco/test/Result.h>
#include <eco/test/Context.h>
#include <eco/test/Check.h>




namespace eco{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class Object : public eco::Object<Object>
{
protected:
	// constructor.
	inline Object()
	{}

	// constructor.
	inline Object(IN const std::string& name) 
		: m_name(name)
	{}

	virtual ~Object()
	{}

public:
	// set name of this object.
	inline void SetName(
		IN const std::string& name)
	{
		m_name = name;
	}

	// get name of this object.
	inline const std::string& GetName() const
	{
		return m_name;
	}

	inline bool operator==(IN const std::string& name) const
	{
		return m_name == name;
	}

	// get result set.
	inline ResultSet& GetResultSet()
	{
		return m_res_set;
	}

	// set test runner context.
	inline void SetContext(IN std::shared_ptr<Context>& context)
	{
		if (m_context == nullptr)
		{
			m_context = context;
		}
	}

	// get test runner context.
	inline std::shared_ptr<Context>& GetContext()
	{
		if (m_context == nullptr)
		{
			m_context.reset(new Context());
		}
		return m_context;
	}

protected:
	// runner object name.
	std::string	m_name;

	// test context.
	std::shared_ptr<Context> m_context;

	// result set of checking.
	ResultSet m_res_set;
};



////////////////////////////////////////////////////////////////////////////////
}}
#endif