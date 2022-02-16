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


ECO_NS_BEGIN(eco);
namespace test{;
////////////////////////////////////////////////////////////////////////////////
class Testing : public eco::Object<Testing>
{
////////////////////////////////////////////////////////////////////////////////
protected:
	// init test context.
	virtual void init_context(OUT eco::Context& context) = 0;

	// init test scene.
	virtual void init_scene() = 0;

public:
	// init test framework.
	inline void init()
	{
		// construct test context.
		std::shared_ptr<Context> context(new Context());
		init_context(*context);
		Suite::get().set_context(context);

		// construct test scene and test case.
		init_scene();
	}

	// run test case.
	virtual void run()
	{
		Suite::get().run();
	}
};


////////////////////////////////////////////////////////////////////////////////
}// ns::test
ECO_NS_END(eco);
#endif