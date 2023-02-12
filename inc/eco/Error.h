#ifndef ECO_ERROR_H
#define ECO_ERROR_H
/*******************************************************************************
@ name
error type.

@ function
1.eco error data.
2.eco error throw.

@ exception


@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/String.h>
#include <eco/rx/RxApi.h>
#ifndef ECO_NO_PROTOBUF
#include <eco/detail/proto/Eco.pb.h>
#endif


ECO_NS_BEGIN(eco);
// eco error mechanism base on thread_local error.
////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(this_thread);
// current thread error c api.
ECO_API int  error_id();
ECO_API const char* error_path();
ECO_API const char* error_value();
ECO_API void error_key(int);
ECO_API void error_key(const char*);
ECO_API void error_value(const char*);
ECO_API void error_clear();
ECO_API void error_append(IN uint32_t siz, IN char c);
ECO_API void error_append(IN const char* buf, IN uint32_t siz);
#define ECO_THIS_ERROR(id_path) eco::Error().key(id_path)
#define ECO_THIS_ERROR_ADD(id_path) eco::Error().add(id_path)
#define ECO_THROW(id_msg) throw eco::Error(id_msg)

// get protobuf error object.
#ifndef ECO_NO_PROTOBUF
ECO_NS_BEGIN(proto);
ECO_API eco::proto::Error& error();
ECO_NS_END(proto);
#endif

// get this thread format.
ECO_API eco::FormatX& format();
ECO_API eco::FormatX& format(const char* msg);
ECO_NS_END(this_thread);


////////////////////////////////////////////////////////////////////////////////
class Error : public std::exception
{
	ECO_STREAM_OPERATOR(Error, (*this), ';');
public:
	inline Error()
	{}
	// for ECO_THROW.
	inline Error(int id)
	{
		this_thread::error_key(id);
	}
	inline Error(const char* msg)
	{
		this_thread::error_key(msg);
	}

	// get error key: id/path;value
	inline int id() const
	{
		return this_thread::error_id();
	}
	inline const char* path() const
	{
		return this_thread::error_path();
	}
	inline const char* value() const
	{
		return this_thread::error_value();
	}
	virtual const char* what() const noexcept override
	{
		return this_thread::error_value();
	}
	// whether has error.
	inline operator bool() const
	{
		return id() != 0 || !eco::empty(path());
	}

	// set error key: id/path;value
	inline Error& key(int id)
	{
		this_thread::error_key(id);
		return *this;
	}
	inline Error& key(const char* v)
	{
		this_thread::error_key(v);
		return *this;
	}
	inline void clear()
	{
		this_thread::error_clear();
	}
	// path: add pro: "a/b" / "c" = "a/b/c";
	inline Error& operator / (const char* v)
	{
		append(1, '/');
		append(v, (uint32_t)strlen(v));
		return *this;
	}

private:
	// append char.
	inline void append(IN uint32_t siz, IN char c)
	{
		this_thread::error_append(siz, c);
	}
	// append string.
	inline void append(IN const char* buf, IN uint32_t siz)
	{
		this_thread::error_append(buf, siz);
	}
};
template<typename StreamT>
StreamT& operator<<(OUT StreamT& stream, IN const eco::Error& e)
{
	const char* p = e.path();
	stream <= e.value();
	if (e.id() != 0) stream <= '#' < e.id();
	if (!eco::empty(p)) stream <= '#' < p;
	return stream;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif