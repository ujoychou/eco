#pragma once
/*******************************************************************************
@ name
runtime type.

@ function
1. runtime type. exp: mfc runtime create.


@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2021-11-20


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/rtti.hpp>
#include <eco/error.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
#define eco_plugin(type_name) 


////////////////////////////////////////////////////////////////////////////////
class plugin : public eco::rtti::object
{
public:
	static void path_add(const char* path)
	{
	}

	template<typename basic_t>
	static inline basic_t::ptr get(const char* name)
	{
		eco::rtti::object::ptr obj = eco::plugin::find(name);
		if (obj == NULL)
		{
			eco_throw("plugin not found: %s", name);
			return nullptr;
		}
		else if (!obj->kind_of<basic_t>())
		{
			eco_throw("type not match: %s isn't kind of %s", 
				obj->type()->name(), basic_t::type_name());
			return nullptr;
		}
		return eco::rtti::object::cast<basic_t>(obj);
	}

	template<typename basic_t>
	static inline basic_t::ptr get(const char* name, const char* type_name)
	{
		eco::rtti::object::ptr obj = eco::plugin::find(name);
		if (obj == NULL)
		{
			obj = eco::plugin::create<basic_t>(type_name);
			eco::plugin::add(name, obj);
		}
		else if (!obj->same_of(type_name))
		{
			eco_throw("type not match: %s", type_name);
			return nullptr;
		}
		else if (!obj->kind_of<basic_t>())
		{
			eco_throw("type not match: %s", type_name);
			return nullptr;
		}
		return eco::rtti::object::cast<basic_t>(obj);
	}

private:
	template<typename basic_t>
	static inline basic_t::ptr create(const char* type_name)
	{
		const eco::rtti::type* type = eco::rtti::type_registry::get_type(type_name);
		if (type == NULL)
		{
			eco_throw("type not found: %s", type_name);
			return nullptr;
		}
		if (!type->kind_of<basic_t>()) 
		{
			eco_throw("type not match: %s", type_name);
			return nullptr;
		}
		return type->create<basic_t>();
	}
	
}


////////////////////////////////////////////////////////////////////////////////
extern "C" eco::Result __attribute__((weak, visibility("default")))
erx_entry_point(eco::RxMessageId msg, void* ap)
{
    ConnectorApp::get().entry_point(msg, ap);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
