#ifndef ECO_TYPE_INL
#define ECO_TYPE_INL
/*******************************************************************************
@ name

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-01-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Type.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
StringAny::StringAny() : m_vtype(0) {}
StringAny::StringAny(IN StringAny&& v)
	: m_value(std::move(v.m_value)), m_vtype(v.m_vtype) { v.m_vtype = 0; }
StringAny::StringAny(IN const StringAny& v)
	: m_value(v.m_value), m_vtype(v.m_vtype) {}
StringAny::StringAny(IN const char* v) : m_value(v), m_vtype(type_string) {}
StringAny::StringAny(IN bool v)
	: m_value(eco::cast<std::string>(v).c_str()), m_vtype(type_bool) {}
StringAny::StringAny(IN char v)
	: m_value(eco::Integer<int8_t>(v).c_str()), m_vtype(type_int32) {}
StringAny::StringAny(IN unsigned char v)
	: m_value(eco::Integer<uint8_t>(v).c_str()), m_vtype(type_int32) {}
StringAny::StringAny(IN short v)
	: m_value(eco::Integer<int16_t>(v).c_str()), m_vtype(type_int32) {}
StringAny::StringAny(IN unsigned short v)
	: m_value(eco::Integer<uint16_t>(v).c_str()), m_vtype(type_int32) {}
StringAny::StringAny(IN int v)
	: m_value(eco::Integer<int32_t>(v).c_str()), m_vtype(type_int32) {}
StringAny::StringAny(IN unsigned int v)
	: m_value(eco::Integer<uint32_t>(v).c_str()), m_vtype(type_int32) {}
StringAny::StringAny(IN long v)
	: m_value(eco::Integer<int32_t>(v).c_str()), m_vtype(type_int32) {}
StringAny::StringAny(IN unsigned long v)
	: m_value(eco::Integer<uint32_t>(v).c_str()), m_vtype(type_int32) {}
StringAny::StringAny(IN int64_t v)
	: m_value(eco::Integer<int64_t>(v).c_str()), m_vtype(type_int64) {}
StringAny::StringAny(IN uint64_t v)
	: m_value(eco::Integer<uint64_t>(v).c_str()), m_vtype(type_int64) {}
StringAny::StringAny(IN double v, IN int precision)
	: m_value(eco::Double(v, precision).c_str()), m_vtype(type_double) {}


////////////////////////////////////////////////////////////////////////////////
StringAny& StringAny::operator=(IN StringAny&& v)
{
	m_value = std::move(v.m_value);
	m_vtype = v.m_vtype;
	v.m_vtype = 0;
	return *this;
}
StringAny& StringAny::operator=(IN const StringAny& v)
{
	m_value = v.m_value;
	m_vtype = v.m_vtype;
	return *this;
}
StringAny& StringAny::operator=(IN const char* v)
{
	m_vtype = type_string;
	m_value = v;
	return *this;
}


////////////////////////////////////////////////////////////////////////////////
bool StringAny::operator==(IN const StringAny& obj) const
{
	return m_value == obj.m_value && m_vtype == obj.m_vtype;
}
bool StringAny::empty() const
{
	return m_value.empty();
}
const char* StringAny::c_str() const
{
	return m_value.c_str();
}
std::string& StringAny::str()
{
	return m_value;
}
StringAny::operator const char*() const
{
	return m_value.c_str();
}
StringAny::operator const char() const
{
	return eco::cast<char>(m_value);
}
StringAny::operator const unsigned char() const
{
	return eco::cast<unsigned char>(m_value);
}
StringAny::operator const short() const
{
	return eco::cast<int16_t>(m_value);
}
StringAny::operator const unsigned short() const
{
	return eco::cast<uint16_t>(m_value);
}
StringAny::operator const int() const
{
	return eco::cast<int32_t>(m_value);
}
StringAny::operator const unsigned int() const
{
	return eco::cast<uint32_t>(m_value);
}
StringAny::operator const long() const
{
	return eco::cast<int32_t>(m_value);
}
StringAny::operator const unsigned long() const
{
	return eco::cast<uint32_t>(m_value);
}
StringAny::operator const int64_t() const
{
	return eco::cast<int64_t>(m_value);
}
StringAny::operator const uint64_t() const
{
	return eco::cast<uint64_t>(m_value);
}
StringAny::operator const float() const
{
	return eco::cast<float>(m_value);
}
StringAny::operator const double() const
{
	return eco::cast<double>(m_value);
}
StringAny::operator const bool() const
{
	return eco::cast<bool>(m_value);
}
ValueType StringAny::type() const
{
	return m_vtype;
}
bool StringAny::operator==(IN const char* v) const
{
	return m_value == v;
}
inline double StringAny::to_double(int precision) const
{
	return eco::cast<double>(m_value, precision);
}

////////////////////////////////////////////////////////////////////////////////
void Parameter::set_name(IN const char* val)
{
	impl().m_name = val;
}
Parameter& Parameter::name(IN const char* val)
{
	impl().m_name = val;
	return *this;
}
const char* Parameter::get_name() const
{
	return impl().m_name.c_str();
}


////////////////////////////////////////////////////////////////////////////////
void Parameter::set_value(IN const StringAny& val)
{
	impl().m_value = val;
}
Parameter& Parameter::value(IN const StringAny& val)
{
	impl().m_value = val;
	return *this;
}
StringAny& Parameter::value()
{
	return impl().m_value;
}
const StringAny& Parameter::get_value() const
{
	return impl().m_value;
}


////////////////////////////////////////////////////////////////////////////////
void Parameter::set_value(IN const char* val)
{
	m_value = val;
}
Parameter& Parameter::value(IN const char* val)
{
	m_value = val;
	return *this;
}
Parameter::operator const char*() const
{
	return m_value.c_str();
}
Parameter::operator const char() const
{
	return (char)(m_value);
}
Parameter::operator const unsigned char() const
{
	return (unsigned char)(m_value);
}
Parameter::operator const short() const
{
	return (int16_t)(m_value);
}
Parameter::operator const unsigned short() const
{
	return (uint16_t)(m_value);
}
Parameter::operator const int() const
{
	return (int32_t)(m_value);
}
Parameter::operator const unsigned int() const
{
	return (uint32_t)(m_value);
}
Parameter::operator const long() const
{
	return (int32_t)(m_value);
}
Parameter::operator const unsigned long() const
{
	return (uint32_t)(m_value);
}
Parameter::operator const int64_t() const
{
	return (int64_t)(m_value);
}
Parameter::operator const uint64_t() const
{
	return (uint64_t)(m_value);
}
Parameter::operator const float() const
{
	return (float)(m_value);
}
Parameter::operator const double() const
{
	return (double)(m_value);
}
Parameter::operator const bool() const
{
	return m_value;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif