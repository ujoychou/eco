#ifndef ECO_RX_OBJECT_H
#define ECO_RX_OBJECT_H
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
#include <eco/rx/RxApi.h>
#include <eco/rx/RxExport.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
typedef void  	(*RxFunc)(void);
ECO_API void* 	dll_load(IN const char* dll_name);
ECO_API void  	dll_free(IN void*& dll_handle);
ECO_API RxFunc 	dll_func(IN void* dll_handle, IN const char* func_name);


////////////////////////////////////////////////////////////////////////////////
class RxObject
{
public:
 	// rx object.
	inline RxObject() : m_handle(nullptr) {}
	inline ~RxObject() { if (m_handle) { dll_free(m_handle); } }

	// load dll file, you can dedicate the name of dll.
	inline void load(IN const char* dll_path, IN const char* dll_name = "")
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
	inline void set_name(IN const char* name) {	m_name = name; }
	inline const char* name() const  { return m_name.c_str(); }
	inline RxObject& name(IN const char* name) { m_name = name; return *this; }
	
	// get dll function.
	inline RxFunc get_func(IN const char* func_name)
	{
		return eco::dll_func(m_handle, func_name);
	}

	// get right function type by cast.
	template<typename func_t>
	inline func_t cast_func(IN const char* func_name)
	{
		return reinterpret_cast<func_t>(get_func(func_name));
	}

private:
	void* m_handle;
	std::string m_name;
	std::string m_path;
};


////////////////////////////////////////////////////////////////////////////////
}
#endif