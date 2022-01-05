#ifndef ECO_ERROR_H
#define ECO_ERROR_H
/*******************************************************************************
@ name
eco basic type.

@ function
1.string_any.
2.variant.

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
#include <eco/RxApi.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
// error mechanism: get this_thread error
class Error;
ECO_NS_BEGIN(this_thread);
// get current thread error.
ECO_API eco::Error& error();
ECO_API int  error_id();
ECO_API const char* error_path();
ECO_API const char* error_value();
ECO_API void error_key(int);
ECO_API void error_key(const char*);
ECO_API void error_add(const char*);
ECO_API void error_val(const char*);
ECO_API void error_clear();
ECO_API void error_append(IN uint32_t siz, IN char c);
ECO_API void error_append(IN const char* buf, IN uint32_t siz);
#define ECO_THROW(id_msg) throw eco::this_thread::error().key_throw(id_msg)
#define ECO_THIS_ERROR(id_path) eco::this_thread::error().key(id_path)
#define ECO_THIS_ERROR_ADD(id_path) eco::this_thread::error().add(id_path)
ECO_NS_END(this_thread);


////////////////////////////////////////////////////////////////////////////////
class Error : public std::exception
{
	ECO_STREAM_OPERATOR(Error, (*this), ';');
public:
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

	virtual const char* what() const override
	{
		return this_thread::error_value();
	}

	// whether has error.
	inline operator bool() const
	{
		return this_thread::error_id() != 0
			|| !eco::empty(this_thread::error_path());
	}

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
	inline Error& add(const char* v)
	{
		this_thread::error_add(v);
		return *this;
	}

	inline Error& key_throw(int id)
	{
		this_thread::error_key(id);
		return *this;
	}
	inline Error& key_throw(const char* v)
	{
		this_thread::error_val(v);
		return *this;
	}

	// path: add pro: "a/b" / "c" = "a/b/c";
	inline Error& operator / (const char* v)
	{
		append(1, '/');
		append(v, (uint32_t)strlen(v));
		return *this;
	}

	inline void clear()
	{
		this_thread::error_clear();
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