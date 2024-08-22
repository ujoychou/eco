#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-08-22] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <stdint.h>
#include <string>


namespace eco {
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class GroupT	// ()
{
public:
	inline GroupT(IN const T& v) : value(v) {}
	const T& value;
};
template<typename T>
inline GroupT<T> group(IN const T& v)
{
	return GroupT<T>(v);
}

template<typename T>
class SquareT	// []
{
public:
	inline SquareT() {}
	inline SquareT(IN const T& v) : value(v) {}
	const T& value;
};
template<typename T>
inline SquareT<T> square(IN const T& v)
{
	return SquareT<T>(v);
}

template<typename T>
class BraceT	// {}
{
public:
	inline BraceT() {}
	inline BraceT(IN const T& v) : value(v) {}
	const T& value;
};
template<typename T>
inline BraceT<T> brace(IN const T& v)
{
	return BraceT<T>(v);
}


////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
class stream
{
public:
	inline type_t& append(double v, int prec, bool percent)
	{
		eco::Double fmt(v, prec, percent);
		return rthis().append(fmt.c_str(), fmt.size());
	}

	template<typename T>
	inline type_t& operator<(T v)
	{
		return *this << v;
	}
	template<typename T>
	inline type_t& operator<=(T v)
	{
		return *this << sep << v;
	}
	inline type_t& operator<<(const eco::Bytes& v)
	{
		return rthis().append(v.c_str(), v.size());
		return (type_t&)(*this);
	}
	ECO_STREAM_OPERATOR_REF(type_t, eco::Bytes, sep)
	inline type_t& operator<<(const eco::String& v)
	{
		return rthis().append(v.c_str(), v.size());
		return (type_t&)(*this);
	}
	ECO_STREAM_OPERATOR_REF(type_t, eco::String, sep)
	inline type_t& operator<<(const std::string& v)
	{
		return rthis().append(v.c_str(), (uint32_t)v.size());
		return (type_t&)(*this);
	}

public:
	inline type_t& rthis()
	{
		return (type_t&)(*this);
	}
	inline type_t& operator<<(const char* v)
	{
		return rthis().append(v, (uint32_t)strlen(v));
	}
	inline type_t& operator<<(bool v)
	{
		(*this) << eco::cast(v).c_str();
		return (type_t&)(*this);
	}
	inline type_t& operator<<(char v)
	{
		return rthis().append(1, v);
	}
	inline type_t& operator<<(int8_t v)
	{
		eco::Integer<int8_t> str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(uint8_t v)
	{
		eco::Integer<uint8_t> str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(int16_t v)
	{
		eco::Integer<int16_t> str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(uint16_t v)
	{
		eco::Integer<uint16_t> str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(int32_t v)
	{
		eco::Integer<int32_t> str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(uint32_t v)
	{
		eco::Integer<uint32_t> str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(int64_t v)
	{
		eco::Integer<int64_t> str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(uint64_t v)
	{
		eco::Integer<uint64_t> str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(double v)
	{
		eco::Double str(v);
		return rthis().append(str.c_str(), str.size());
	}
};


#define ECO_APP(app_class)
////////////////////////////////////////////////////////////////////////////////
} // namespace eco