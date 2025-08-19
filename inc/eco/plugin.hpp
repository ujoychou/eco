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
class plugin_type;
////////////////////////////////////////////////////////////////////////////////
class eco_api plugin_registry
{
public:
	static void set(eco::plugin_type* plugin);

	static eco::plugin_type* get(
		const char* name,
		const char* version);

	static eco::plugin_type* get_compatible(
		const char* name,
		const char* version);
};


////////////////////////////////////////////////////////////////////////////////
class plugin_type
{
public:
	inline plugin_type(
		const char* name,
		const char* version,
		create_func_t create,
		compatible_func_t compatible)
		: m_name(name)
		, m_version(version)
		, m_dllpath(eco::os::dllpath())
		, m_create(create)
		, m_compatible(compatible)
	{
		eco::plugin_registry::set(this);
	}

private:
	typedef void* (*create_func_t)(const char* name);
	typedef eco::bool_t (*compatible_func_t)(const char* version);

	const char* 		m_name;
	const char* 		m_version;
	const char* 		m_dllpath;
	create_func_t 		m_create;
	compatible_func_t 	m_compatible;
	eco::rtti::dll 		m_dll;
};


////////////////////////////////////////////////////////////////////////////////
class eco_api plugin
{
public:
	static void* get_object(
		const char* plugin_name,
		const char* plugin_version,
	    const char* object_name);

	template<typename object_t>
	inline static object_t* get_object(
		const char* plugin_name,
		const char* plugin_version,
	    const char* object_name)
	{
		return static_cast<object_t*>(get_object(
			plugin_name, plugin_version, object_name));
	}

	inline static eco::plugin_type* get_plugin(
		const char* name, const char* version)
	{
		return eco::plugin_registry::get(name, version);
	}

	static eco::plugin_type* get_plugin_compatible(
		const char* name, const char* version)
	{
		return eco::plugin_registry::get_compatible(name, version);
	}
};


////////////////////////////////////////////////////////////////////////////////
// plugin init when compile time.
template<typename type_t>
struct plugin_init { static eco::plugin_type type; };
template<typename type_t>
eco::plugin_type plugin_init<type_t>::type(
	type_t::name(), type_t::version(),
    &type_t::create, &type_t::compatible);

#define eco_plugin__(type_t, name, version)\
inline static const char* name() { return name; } \
inline static const char* version() { return version; } \
inline static void* create() { return new type_t(); } \
inline static eco::plugin_type* plugin() { return &plugin_init<type_t>::type;}
#define eco_plugin_2(type_t, version)\
eco_plugin__(type_t, eco_macro_str(type_t), version)
#define eco_plugin_3(type_t, type_name, version)\
eco_plugin__(type_t, type_name, version)
#define eco_plugin(...) eco_macro_overload(eco_plugin_,__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rtti);
eco_namespace_end(eco);
