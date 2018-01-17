#ifndef ECO_DLL_H
#define ECO_DLL_H
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
#include <eco/ExportApi.h>
#include <eco/App.h>


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
typedef void  (*DllFunc)(void);		// function ptr.
ECO_API void* load_dll(IN const char* dll_name);
ECO_API void  free_dll(IN void*& dll_handle);
ECO_API DllFunc get_function(IN void* dll_handle, IN const char* func_name);


////////////////////////////////////////////////////////////////////////////////
class ECO_API DllObject
{
	ECO_OBJECT_API(DllObject);
public:
	void set_dll(
		IN const char* dll_name,
		IN const char* dll_path);

	const char* get_dll_path() const;

	void set_dll_name(IN const char* name);
	const char* get_dll_name() const;
	DllObject& dll_name(IN const char*);
	
	DllFunc get_function(IN const char* func_name);
	template<typename FunctionT>
	inline FunctionT cast_function(IN const char* func_name)
	{
		return reinterpret_cast<FunctionT>(get_function(func_name));
	}
};


////////////////////////////////////////////////////////////////////////////////
}
#endif