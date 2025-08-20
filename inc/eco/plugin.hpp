#pragma once
/*******************************************************************************
@ name
plugin

@ function


@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy created on 2025-08-19


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/rtti/api.hpp>
#include <eco/rtti/dll.hpp>
#include <eco/error.hpp>


eco_namespace(eco);
eco_namespace(rtti);
////////////////////////////////////////////////////////////////////////////////
struct plugin_type
{
public:
	typedef void* (*create_t)(const char* name);
	eco::string name;
	eco::string version;
	eco::string	dllpath;
	create_t	create;
};
class eco_api plugin_registry
{
public:
	static eco::rtti::plugin_type* set_plugin(
		const char* name,
		const char* version,
		eco::rtti::plugin_type::create_t create);

	static eco::rtti::plugin_type* get_plugin(
		const char* name,
		const char* version);

	static eco::rtti::plugin_type* get_plugin_compatible(
		const char* name,
		const char* version);
};
struct plugin_regisger
{
	inline plugin_regisger(
		const char* n, const char* v, eco::rtti::plugin_type::create_t c)
	{
		type = eco::rtti::plugin_registry::set_plugin(n, v, c);
	}
	eco::rtti::plugin_type* type = nullptr;
};


////////////////////////////////////////////////////////////////////////////////
class eco_api plugin
{
public:
	static void* get(
		const char* plugin_name,
		const char* plugin_version,
	    const char* object_name);

	template<typename object_t>
	inline static object_t* get(
		const char* plugin_name,
		const char* plugin_version,
	    const char* object_name)
	{
		return static_cast<object_t*>(get(
			plugin_name, plugin_version, object_name));
	}

	inline static eco::rtti::plugin_type* get_plugin(
		const char* name, const char* version)
	{
		return eco::rtti::plugin_registry::get_plugin(name, version);
	}

	static eco::rtti::plugin_type* get_plugin_compatible(
		const char* name, const char* version)
	{
		return eco::rtti::plugin_registry::get_plugin_compatible(name, version);
	}
};


////////////////////////////////////////////////////////////////////////////////
// plugin init when compile time.
template<typename type_t>
struct plugin_init { static eco::rtti::plugin_regisger reg; };
template<typename type_t>
eco::rtti::plugin_regisger plugin_init<type_t>::reg(
	type_t::name(), type_t::version(), &type_t::create);

#define eco_plugin__(type_t, nam, ver)\
public:\
inline static const char* name() { return nam; } \
inline static const char* version() { return ver; } \
inline static void* create(const char* name) { return new type_t(); } \
inline static eco::rtti::plugin_type* plugin()\
{\
	return eco::rtti::plugin_init<type_t>::reg.type;\
}
#define eco_plugin_2(type_t, version)\
eco_plugin__(type_t, eco_macro_str(type_t), version)
#define eco_plugin_3(type_t, type_name, version)\
eco_plugin__(type_t, type_name, version)
#define eco_plugin(...) eco_macro_overload(eco_plugin_,__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rtti);
eco_namespace_end(eco);
