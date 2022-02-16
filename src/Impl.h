#ifndef ECO_APP_INNER_H
#define ECO_APP_INNER_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2019-08-19.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2017 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/App.h>



ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class AppInner
{
public:
	inline static bool on_console(App& app, int event)
	{
		return app.on_console(event);
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif