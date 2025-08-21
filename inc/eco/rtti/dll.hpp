#pragma once
/*******************************************************************************
@ name
dll entry

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/rtti/api.hpp>
#include <eco/rtti/type.hpp>
#include <eco/os/os.hpp>
#include <string>


eco_namespace(eco);
eco_namespace(rtti);
////////////////////////////////////////////////////////////////////////////////
class dll
{
public:
 	// rx object.
	inline dll() {}
	inline ~dll() { if (m_handle) { eco::os::dll_free(m_handle); } }

	// load dll file, you can dedicate the name of dll.
	inline void load(const char* dll_path, const char* dll_name = "")
	{
		m_path = dll_path;
		m_name = dll_name;
		m_handle = eco::os::dll_load(dll_path);
	}

	// get dll file path.
	inline const char* path() const
	{
		return m_path.c_str();
	}

	// dll name when dedicated, or path leaf name if not.
	inline void set_name(const char* name) { m_name = name; }
	inline const char* name() const  { return m_name.c_str(); }
	inline dll& name(const char* name) { m_name = name; return *this; }
	
	// get dll function.
	inline eco::func_t get_func(const char* func_name)
	{
		return eco::os::dll_func(m_handle, func_name);
	}

	// get right function type by cast.
	template<typename func_t>
	inline func_t cast_func(const char* func_name)
	{
		return reinterpret_cast<func_t>(get_func(func_name));
	}

	inline operator bool() const { return m_handle != nullptr; }

private:
	void* m_handle = nullptr;
	std::string m_name;
	std::string m_path;
};


////////////////////////////////////////////////////////////////////////////////
class rxobject : public eco::rtti::dll
{
public:
	typedef eco::result (*erx_entry_point_t)(eco::rtti::message msg, void* app);

	inline rxobject(const char* dll_path, const char* dll_name = "")
	{
		eco::rtti::dll::load(dll_path, dll_name);	
		m_entry_point = eco::rtti::dll::cast_func<
			erx_entry_point_t>("erx_entry_point");
	}

	// notify erx when app init.
	inline eco::result on_init()
	{
		return m_entry_point(eco::rtti::message_init, nullptr);
	}

	// notify erx init command.
	inline eco::result on_cmd()
	{
		return m_entry_point(eco::rtti::message_cmd, nullptr);
	}

	// notify erx load data.
	inline eco::result on_load()
	{
		return m_entry_point(eco::rtti::message_load, nullptr);
	}

	// notify erx when app exit.
	inline eco::result on_exit()
	{
		return m_entry_point(eco::rtti::message_exit, nullptr);
	}

private:
	erx_entry_point_t m_entry_point;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rtti);
eco_namespace_end(eco);