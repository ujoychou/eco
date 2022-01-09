#ifndef ECO_STREAM_H
#define ECO_STREAM_H
/*******************************************************************************
@ name

@ function

@ exception


@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Cast.h>


////////////////////////////////////////////////////////////////////////////////
#define ECO_STREAM_OPERATOR_REF(stream_t, value_t, sep)\
inline stream_t& operator<(IN const value_t& v)\
{\
	return *this << v;\
}\
inline stream_t& operator<=(IN const value_t& v)\
{\
	return *this << sep << v;\
}
#define ECO_STREAM_OPERATOR_TPL(stream_t, value_t, sep1, sep2)\
template<typename T>\
inline stream_t& operator<<(IN const value_t& v)\
{\
	return (*this) << sep1 << v.value << sep2;\
}\
template<typename T>\
inline stream_t& operator<(IN const value_t& v)\
{\
	return (*this) << v;\
}\
template<typename T>\
inline stream_t& operator<=(IN const value_t& v)\
{\
	return (*this) <= sep1 < v.value < sep2;\
}

#define ECO_STREAM_OPERATOR(stream_t, member, sep)\
public:\
inline stream_t& append(double v, int prec, bool percent)\
{\
	eco::Double fmt(v, prec, percent);\
	member.append(fmt.c_str(), fmt.size());\
	return *this;\
}\
inline stream_t& operator<<(IN bool v)\
{\
	(*this) << eco::cast(v).c_str();\
	return *this;\
}\
inline stream_t& operator<<(IN char v)\
{\
	member.append(1, v);\
	return *this;\
}\
inline stream_t& operator<<(IN int8_t v)\
{\
	eco::Integer<int8_t> str(v);\
	member.append(str.c_str(), str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN uint8_t v)\
{\
	eco::Integer<uint8_t> str(v);\
	member.append(str.c_str(), str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN int16_t v)\
{\
	eco::Integer<int16_t> str(v);\
	member.append(str.c_str(), str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN uint16_t v)\
{\
	eco::Integer<uint16_t> str(v);\
	member.append(str.c_str(), str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN int32_t v)\
{\
	eco::Integer<int32_t> str(v);\
	member.append(str.c_str(), str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN uint32_t v)\
{\
	eco::Integer<uint32_t> str(v);\
	member.append(str.c_str(), str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN int64_t v)\
{\
	eco::Integer<int64_t> str(v);\
	member.append(str.c_str(), str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN uint64_t v)\
{\
	eco::Integer<uint64_t> str(v);\
	member.append(str.c_str(), str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN double v)\
{\
	eco::Double str(v);\
	member.append(str.c_str(), str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN const char* v)\
{\
	member.append(v, (uint32_t)strlen(v));\
	return *this;\
}\
template<typename T>\
inline stream_t& operator<(IN T v)\
{\
	return *this << v;\
}\
template<typename T>\
inline stream_t& operator<=(IN T v)\
{\
	return *this << sep << v;\
}\
inline stream_t& operator<<(IN const eco::Bytes& v)\
{\
	member.append(v.c_str(), v.size());\
	return *this;\
}\
ECO_STREAM_OPERATOR_REF(stream_t, eco::Bytes, sep)\
inline stream_t& operator<<(IN const eco::String& v)\
{\
	member.append(v.c_str(), v.size());\
	return *this;\
}\
ECO_STREAM_OPERATOR_REF(stream_t, eco::String, sep)\
inline stream_t& operator<<(IN const std::string& v)\
{\
	member.append(v.c_str(), (uint32_t)v.size());\
	return *this;\
}\
ECO_STREAM_OPERATOR_REF(stream_t, std::string, sep)\
ECO_STREAM_OPERATOR_TPL(stream_t, eco::GroupT<T>, '(', ')')\
ECO_STREAM_OPERATOR_TPL(stream_t, eco::SquareT<T>,'[', ']')\
ECO_STREAM_OPERATOR_TPL(stream_t, eco::BraceT<T>, '{', '}')


ECO_NS_BEGIN(eco);
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
ECO_NS_END(eco);
#endif