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


ECO_NS_BEGIN(eco);
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
	void load(
		IN const char* dll_path,
		IN const char* dll_name = "");

	const char* path() const;

	void set_name(IN const char* name);
	const char* name() const;
	DllObject& name(IN const char*);
	
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