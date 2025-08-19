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
#include <string>


eco_namespace(eco);
eco_namespace(rx);
////////////////////////////////////////////////////////////////////////////////
typedef void  	(*func_t)(void);
eco_api void* 	dll_load(const char* dll_name);
eco_api void  	dll_free(void*& dll_handle);
eco_api func_t 	dll_func(void* dll_handle, const char* func_name);


////////////////////////////////////////////////////////////////////////////////
class dll
{
public:
 	// rx object.
	inline dll() : m_handle(nullptr) {}
	inline ~dll() { if (m_handle) { dll_free(m_handle); } }

	// load dll file, you can dedicate the name of dll.
	inline void load(const char* dll_path, const char* dll_name = "")
	{
		m_path = dll_path;
		m_name = dll_name;
		m_handle = eco::dll_load(dll_path);
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
	inline func_t get_func(const char* func_name)
	{
		return eco::dll_func(m_handle, func_name);
	}

	// get right function type by cast.
	template<typename func_t>
	inline func_t cast_func(const char* func_name)
	{
		return reinterpret_cast<func_t>(get_func(func_name));
	}

	inline operator bool() const { return m_handle != nullptr; }

private:
	void* m_handle;
	std::string m_name;
	std::string m_path;
};


////////////////////////////////////////////////////////////////////////////////
class object : public eco::rx::dll
{
public:
	typedef eco::result (*erx_entry_point_t)(eco::rx::message msg, void* ap);

	inline object(
		const char* dll_path,
		const char* dll_name = "")
		: m_rx_msg(0)
	{
		eco::dll::load(dll_path, dll_name);
		m_entry_point = eco::rx::dll::cast_func<
			erx_entry_point_t>("erx_entry_point");
	}

	// notify erx when app init.
	inline eco::result on_init(eco::app& app)
	{
		m_rx_msg = message_init;
		return m_entry_point(message(m_rx_msg), &app);
	}

	// notify erx init command.
	inline eco::result on_cmd()
	{
		m_rx_msg = message_cmd;
		return m_entry_point(message(m_rx_msg), nullptr);
	}

	// notify erx load data.
	inline eco::result on_load()
	{
		m_rx_msg = message_load;
		return m_entry_point(message(m_rx_msg), nullptr);
	}

	// notify erx when app exit.
	inline eco::result on_exit()
	{
		m_rx_msg = message_exit;
		return m_entry_point(message(m_rx_msg), nullptr);
	}

	inline eco::rx::message message() const
	{
		return m_rx_msg;
	}

private:
	eco::rx::message  m_rx_msg;
	erx_entry_point_t m_entry_point;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rx);
eco_namespace_end(eco);