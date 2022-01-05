#include "Pch.h"
#include <eco/DllObject.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/RxApp.h>

ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class DllObject::Impl
{
public:
public:
	std::string m_name;
	std::string m_path;
	void* m_dll_handle;

public:
	void init(IN DllObject&)
	{
		m_dll_handle = nullptr;
	}

	~Impl()
	{
		eco::free_dll(m_dll_handle);
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_OBJECT_IMPL(DllObject);
ECO_PROPERTY_STR_IMPL(DllObject, name);
const char* DllObject::path() const
{
	return impl().m_path.c_str();
}
void DllObject::load(IN const char* dll_path, IN const char* dll_name)
{
	impl().m_name = dll_name;
	impl().m_path = dll_path;
	impl().m_dll_handle = eco::load_dll(dll_path);
}
DllFunc DllObject::get_function(IN const char* func_name)
{
	return eco::get_function(impl().m_dll_handle, func_name);
}


////////////////////////////////////////////////////////////////////////////////
}