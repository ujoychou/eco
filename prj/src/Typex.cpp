#include "PrecHeader.h"
#include <eco/Typex.ipp>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/Cast.h>
#include <boost/algorithm/string.hpp>


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
ECO_VALUE_IMPL(StringAny);
StringAny::StringAny(IN const char* v)
{
	m_impl = new Impl(v);
}
StringAny& StringAny::operator=(IN const char* v)
{
	impl().m_value = v;
	return *this;
}
bool StringAny::operator==(IN const StringAny& obj) const
{
	return impl().m_value == obj.impl().m_value;
}
bool StringAny::empty() const
{
	return impl().m_value.empty();
}
const char* StringAny::c_str() const
{
	return impl().m_value.c_str();
}
StringAny::operator const char*() const
{
	return impl().m_value.c_str();
}
StringAny::operator const char() const
{
	return eco::cast<char>(impl().m_value);
}
StringAny::operator const unsigned char() const
{
	return eco::cast<unsigned char>(impl().m_value);
}
StringAny::operator const short() const
{
	return eco::cast<int16_t>(impl().m_value);
}
StringAny::operator const unsigned short() const
{
	return eco::cast<uint16_t>(impl().m_value);
}
StringAny::operator const int() const
{
	return eco::cast<int32_t>(impl().m_value);
}
StringAny::operator const unsigned int() const
{
	return eco::cast<uint32_t>(impl().m_value);
}
StringAny::operator const long() const
{
	return eco::cast<int32_t>(impl().m_value);
}
StringAny::operator const unsigned long() const
{
	return eco::cast<uint32_t>(impl().m_value);
}
StringAny::operator const int64_t() const
{
	return eco::cast<int64_t>(impl().m_value);
}
StringAny::operator const uint64_t() const
{
	return eco::cast<uint64_t>(impl().m_value);
}
StringAny::operator const double() const
{
	return eco::cast<double>(impl().m_value);
}
StringAny::operator const bool() const
{
	return boost::iequals(impl().m_value, "true");
}


////////////////////////////////////////////////////////////////////////////////
ECO_VALUE_IMPL(Parameter);
ECO_PROPERTY_STR_IMPL(Parameter, key);
ECO_PROPERTY_VAL_IMPL(Parameter, StringAny, value);
void Parameter::set_value(IN const char* val)
{
	impl().m_value = val;
}
Parameter& Parameter::value(IN const char* val)
{
	impl().m_value = val;
	return *this;
}
Parameter::operator const char*() const
{
	return impl().m_value.c_str();
}
Parameter::operator const char() const
{
	return (char)(impl().m_value);
}
Parameter::operator const unsigned char() const
{
	return (unsigned char)(impl().m_value);
}
Parameter::operator const short() const
{
	return (int16_t)(impl().m_value);
}
Parameter::operator const unsigned short() const
{
	return (uint16_t)(impl().m_value);
}
Parameter::operator const int() const
{
	return (int32_t)(impl().m_value);
}
Parameter::operator const unsigned int() const
{
	return (uint32_t)(impl().m_value);
}
Parameter::operator const long() const
{
	return (int32_t)(impl().m_value);
}
Parameter::operator const unsigned long() const
{
	return (uint32_t)(impl().m_value);
}
Parameter::operator const int64_t() const
{
	return (int64_t)(impl().m_value);
}
Parameter::operator const uint64_t() const
{
	return (uint64_t)(impl().m_value);
}
Parameter::operator const double() const
{
	return (double)(impl().m_value);
}
Parameter::operator const bool() const
{
	return boost::iequals(impl().m_value.c_str(), "true");
}


////////////////////////////////////////////////////////////////////////////////
ECO_VALUE_IMPL(Context);
ECO_PROPERTY_SET_IMPL(Context, Parameter);
bool Context::has(IN const char* key) const
{
	StringAny v;
	return find(v, key);
}


////////////////////////////////////////////////////////////////////////////////
bool Context::find(OUT StringAny& v, IN const char* key) const
{
	auto it = impl().m_items.begin();
	for (; it != impl().m_items.end(); ++it)
	{
		if (strcmp(it->get_key(), key) == 0)
		{
			v = it->get_value();
			return true;
		}
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
StringAny Context::at(IN const char* key) const
{
	StringAny v;
	if (!find(v, key))
	{
		EcoThrow << "find parameter fail: key=" << key;
	}
	return v;
}


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(ContextNode);
ECO_PROPERTY_STR_IMPL(ContextNode, name);
ECO_PROPERTY_STR_IMPL(ContextNode, value);
ECO_PROPERTY_VAL_IMPL(ContextNode, Context, property_set);
ECO_PROPERTY_OBJ_IMPL(ContextNode, ContextNodeSet, children);


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(ContextNodeSet);
ECO_PROPERTY_SET_IMPL(ContextNodeSet, ContextNode);



////////////////////////////////////////////////////////////////////////////////
ECO_VALUE_IMPL(StdString);
ECO_PROPERTY_STR_IMPL(StdString, value);
ECO_SHARED_IMPL(StdStringSet)
ECO_PROPERTY_SET_IMPL(StdStringSet, StdString);
StdString::StdString(const char* v)
{
	m_impl = new Impl();
	m_impl->init(*this);
	m_impl->m_value = v;
}
const char* StdString::c_str() const
{
	return impl().m_value.c_str();
}
StdString::operator const char*() const
{
	return impl().m_value.c_str();
}


}