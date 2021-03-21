#include "PrecHeader.h"
#include <eco/thread/Thread.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Cast.h>
#include <eco/log/Log.h>


// this thread data.
ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(this_thread);
////////////////////////////////////////////////////////////////////////////////
// define this_thread error data with: data struct/ proto error.
#ifndef ECO_NO_PROTOBUF
thread_local eco::proto::Error* t_error_data = 0;
inline eco::proto::Error& local_error_data() {
	if (t_error_data == 0)	t_error_data = new eco::proto::Error();
	return *t_error_data;
}
ECO_NS_BEGIN(proto);
eco::proto::Error& error() { return local_error_data(); }
ECO_NS_END(proto);
#endif


////////////////////////////////////////////////////////////////////////////////
int error_id()
{
	return local_error_data().id();
}
const char* error_path()
{
	return local_error_data().path().c_str();
}
const char* error_value()
{
	return local_error_data().value().c_str();
}


////////////////////////////////////////////////////////////////////////////////
void error_key(int id)
{
	local_error_data().set_id(id);
	local_error_data().mutable_path()->clear();
	local_error_data().mutable_value()->clear();
}
void error_key(const char* path)
{
	local_error_data().set_path(path);
	local_error_data().set_id(0);
	local_error_data().mutable_value()->clear();
}
void error_val(const char* val)
{
	local_error_data().set_value(val);
	local_error_data().mutable_path()->clear();
	local_error_data().set_id(0);
}
void error_add(const char* path)
{
	if (local_error_data().id() != 0)
	{
		eco::Integer<int32_t> fmt(local_error_data().id());
		local_error_data().set_id(0);
		local_error_data().set_path(fmt.c_str(), fmt.size());
	}
	if (!local_error_data().path().empty())
	{
		local_error_data().mutable_path()->append(1, ';');
	}
	local_error_data().mutable_path()->append(path);
}
void error_append(IN uint32_t siz, IN char c)
{
	local_error_data().mutable_value()->append(siz, c);
}
void error_append(IN const char* buf, IN uint32_t siz)
{
	local_error_data().mutable_value()->append(buf, siz);
}
void error_clear()
{
	local_error_data().set_id(0);
	local_error_data().mutable_path()->clear();
	local_error_data().mutable_value()->clear();
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(this_thread);
ECO_NS_END(eco);