#ifndef AFW_TEST_TESTING_H
#define AFW_TEST_TESTING_H
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
#include <eco/test/Suite.h>




namespace eco{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class Testing : public eco::Object<Testing>
{
////////////////////////////////////////////////////////////////////////////////
protected:
	// init test context.
	virtual void InitContext(
		OUT eco::test::Context& context) = 0;

	// init test scene.
	virtual void InitScene() = 0;

public:
	// init test framework.
	inline void Init()
	{
		// construct test context.
		std::shared_ptr<Context> context(new Context());
		InitContext(*context);
		GetSuite().SetContext(context);

		// construct test scene and test case.
		InitScene();
	}

	// run test case.
	virtual void Run()
	{
		GetSuite().Run();
	}
};


////////////////////////////////////////////////////////////////////////////////
}// ns::test
}// ns::eco
#endif