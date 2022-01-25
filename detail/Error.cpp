#include "Pch.h"
#include <eco/Error.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Cast.h>
#ifndef ECO_NO_PROTOBUF
#	include <eco/detail/proto/Eco.pb.h>
#endif

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
inline eco::this_thread::Error& impl_err()
{
	if (t_error == nullptr)
	{
		// init error id, allocate memory in dll.
		t_error = new eco::this_thread::Error();
		t_error->set_id(0);
		t_error->mutable_path();
		t_error->mutable_value();
	}
	return *t_error;
}
ECO_API int error_id()
{
	return impl_err().id();
}
ECO_API const char* error_path()
{
	return impl_err().path().c_str();
}
ECO_API const char* error_value()
{
	return impl_err().value().c_str();
}
ECO_API void error_key(int id)
{
	// key id mode, not using path.
	impl_err().set_id(id);
	impl_err().mutable_path()->clear();
	impl_err().mutable_value()->clear();
}
ECO_API void error_key(const char* path)
{
	// key path mode, not using path.
	impl_err().set_path(path);
	impl_err().set_id(0);
	impl_err().mutable_value()->clear();
}
ECO_API void error_value(const char* val)
{
	impl_err().set_value(val);
}
ECO_API void error_append(IN uint32_t siz, IN char c)
{
	impl_err().mutable_value()->append(siz, c);
}
ECO_API void error_append(IN const char* buf, IN uint32_t siz)
{
	impl_err().mutable_value()->append(buf, siz);
}
ECO_API void error_clear()
{
	impl_err().set_id(0);
	impl_err().mutable_path()->clear();
	impl_err().mutable_value()->clear();
}
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(this_thread);
ECO_NS_END(eco);