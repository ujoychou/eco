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
eco_namespace(rx);
////////////////////////////////////////////////////////////////////////////////
class app
{
protected:
	// notify erx when app init.
	virtual eco::result on_init()
	{
		return eco::ok;
	}

	// notify erx init command.
	virtual eco::result on_cmd()
	{
		return eco::ok;
	}

	// notify erx load data.
	virtual eco::result on_load()
	{
		return eco::ok;
	}

	// notify erx when app exit.
	virtual eco::result on_exit()
	{
		return eco::ok;
	}

public:
	inline app()
	{
		m_app = nullptr;
	}

	// get app who load erx.
	inline eco::app& get_app()
	{
		return *m_app;
	}

	// erx dll entry point.
	inline eco::result entry_point(const eco::rx::message msg, void* app)
	{
		switch (msg)
		{
		case eco::rx::message_init:
			m_app = static_cast<eco::app*>(app);
			return on_init();
		case eco::rx::message_cmd:
			return on_cmd();
		case eco::rx::message_load:
			return on_load();
		case eco::rx::message_exit:
			return on_exit();
		}
		return eco::ok;
	}

private:
	eco::app* m_app;
};


////////////////////////////////////////////////////////////////////////////////
#define eco_erx(erx_app) \
extern "C" __attribute__((weak, visibility("default"))) \
eco::result erx_entry_point(eco::rx::message msg, void* ap) \
{ \
    eco::singleton<erx_app>::get().entry_point(msg, ap); \
	return eco::ok; \
}

////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rx);
eco_namespace_end(eco);