#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note
1.ARX: AutoCAD Runtime Extension

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/rtti/dll.hpp>
#include <eco/rtti/type.hpp>


eco_namespace(eco);
eco_namespace(rtti);
////////////////////////////////////////////////////////////////////////////////
class app
{
protected:
	// notify erx when app init.
	virtual eco::result on_init()
	{
		return eco::result::ok;
	}

	// notify erx init command.
	virtual eco::result on_cmd()
	{
		return eco::result::ok;
	}

	// notify erx load data.
	virtual eco::result on_load()
	{
		return eco::result::ok;
	}

	// notify erx when app exit.
	virtual eco::result on_exit()
	{
		return eco::result::ok;
	}

public:
	// erx dll entry point.
	inline eco::result entry_point(const eco::rtti::message msg, void* app)
	{
		switch (msg)
		{
		case eco::rtti::message_init:
			return on_init();
		case eco::rtti::message_cmd:
			return on_cmd();
		case eco::rtti::message_load:
			return on_load();
		case eco::rtti::message_exit:
			return on_exit();
		}
		return eco::result::ok;
	}

private:
};


////////////////////////////////////////////////////////////////////////////////
#define eco_erx(erx_app_t, erx_app) \
inline erx_app_t& erx_app() \
{\
	return eco::singleton<erx_app_t>::get();\
}\
extern "C" __attribute__((weak, visibility("default"))) \
eco::result erx_entry_point(eco::rtti::message msg, void* ap) \
{ \
    return erx_app().entry_point(msg, ap); \
}

////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rtti);
eco_namespace_end(eco);