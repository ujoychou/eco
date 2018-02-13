#ifndef ECO_TEST_CONTEXT_H
#define ECO_TEST_CONTEXT_H
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
#include <eco/thread/Map.h>
#include <eco/Typex.h>




namespace eco{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class Context : public eco::HashMap<std::string, eco::StringAny>
{
public:
	typedef eco::HashMap<std::string, eco::StringAny> repository;

	inline const eco::StringAny GetValue(IN const std::string& key)
	{
		return repository::get(key);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif