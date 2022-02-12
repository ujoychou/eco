#include "Pch.h"
#include <eco/Error.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Cast.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(this_thread);
////////////////////////////////////////////////////////////////////////////////
#ifdef ECO_NO_PROTOBUF
#else
typedef eco::proto::Error Error;
#endif


////////////////////////////////////////////////////////////////////////////////
thread_local eco::FormatX* t_format = nullptr;
inline eco::FormatX& impl_fmt()
{
	if (t_format == nullptr) { t_format = new eco::FormatX(); }
	return *t_format;
}
ECO_API eco::FormatX& format()
{
	return impl_fmt();
}
ECO_API eco::FormatX& format(const char* msg)
{
	impl_fmt().reset(msg);
	return impl_fmt();
}


////////////////////////////////////////////////////////////////////////////////
thread_local Error* t_error = nullptr;
inline eco::this_thread::Error& impl()
{
	if (t_error == nullptr)
	{
		// init error id, allocate memory in dll.
		t_error = new eco::proto::Error();
		t_error->set_id(0);
		t_error->mutable_path();
		t_error->mutable_value();
	}
	return *t_error;
}
ECO_API int error_id()
{
	return impl().id();
}
ECO_API const char* error_path()
{
	return impl().path().c_str();
}
ECO_API const char* error_value()
{
	return impl().value().c_str();
}
#ifndef ECO_NO_PROTOBUF
ECO_NS_BEGIN(proto);
ECO_API eco::proto::Error& error() { return impl(); }
ECO_NS_END(proto);
#endif


////////////////////////////////////////////////////////////////////////////////
ECO_API void error_key(int id)
{
	// key id mode, not using path.
	impl().set_id(id);
	impl().mutable_path()->clear();
	impl().mutable_value()->clear();
}
ECO_API void error_key(const char* path)
{
	// key path mode, not using path.
	impl().set_path(path);
	impl().set_id(0);
	impl().mutable_value()->clear();
}
ECO_API void error_value(const char* val)
{
	impl().set_value(val);
}
ECO_API void error_clear()
{
	impl().set_id(0);
	impl().mutable_path()->clear();
	impl().mutable_value()->clear();
}
ECO_API void error_append(IN uint32_t siz, IN char c)
{
	impl().mutable_value()->append(siz, c);
}
ECO_API void error_append(IN const char* buf, IN uint32_t siz)
{
	impl().mutable_value()->append(buf, siz);
}
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(this_thread);
ECO_NS_END(eco);