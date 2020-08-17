#ifndef ECO_THREAD_ERROR_H
#define ECO_THREAD_ERROR_H
/*******************************************************************************
@ name
thread and common thread function.

@ function


@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Type.h>
#include <eco/proto/Object.pb.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(this_thread);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Error
{
public:
	Error& id(int);
	void set_id(int);

	Error& path(const char*);
	void set_path(const char*);

	Error& key(int);
	Error& key(const char*);

	eco::proto::Error& data() const;
	inline operator eco::proto::Error&() const
	{
		return data();
	}

	operator bool() const;
	bool has_error() const;

	void clear();

	// path: add pro: "a/b" / "c" = "a/b/c";
	Error& operator / (const char* v);

	// param: add param "v1;v2" + "v3" = "v1;v2;v3".
	Error& operator<(const char v);
	Error& operator<(const char* v);
	Error& append(const char* v, uint32_t n);

	inline Error& operator<(uint32_t v)
	{
		return operator<(eco::Integer<uint32_t>(v).c_str());
	}
	inline Error& operator<(int32_t v)
	{
		return operator<(eco::Integer<int32_t>(v).c_str());
	}
	inline Error& operator<(uint64_t v)
	{
		return operator<(eco::Integer<uint64_t>(v).c_str());
	}
	inline Error& operator<(int64_t v)
	{
		return operator<(eco::Integer<int64_t>(v).c_str());
	}
	inline Error& operator<(double v)
	{
		return operator<(eco::Double(v).c_str());
	}
	inline Error& operator<(const std::string& v)
	{
		return append(v.c_str(), v.size());
	}

	template<typename T>
	inline Error& operator<=(const T v)
	{
		return operator<(v) < ';';
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(this_thread);
ECO_NS_END(eco);
#endif