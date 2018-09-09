#include "PrecHeader.h"
#include <eco/Typex.ipp>
////////////////////////////////////////////////////////////////////////////////
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
StringAny::operator const float() const
{
	return eco::cast<float>(impl().m_value);
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
Parameter::operator const float() const
{
	return (float)(impl().m_value);
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
const StringAny ContextNode::at(IN const char* key) const
{
	auto* any = find(key);
	if (any == nullptr)
		EcoThrow << "get context node key value error: " << key;
	return *any;
}


////////////////////////////////////////////////////////////////////////////////
const StringAny* ContextNode::find(IN const char* key) const
{
	int node_end = eco::find_first(key, '/');
	if (node_end == -1)
	{
		// find the key value.
		auto it = impl().m_property_set.begin();
		for (; it != impl().m_property_set.end(); ++it)
		{
			if (strcmp(key, it->get_key()) == 0)
				return &it->get_value();
		}
	}
	else
	{
		// recursive find the key value.
		auto it = impl().m_children.begin();
		for (; it != impl().m_children.end(); ++it)
		{
			if (strncmp(it->get_name(), key, node_end) == 0)
			{
				return it->find(&key[node_end + 1]);
			}
		}
	}
	return nullptr;
}


////////////////////////////////////////////////////////////////////////////////
eco::ContextNode ContextNode::get_children(const char* key) const
{
	int node_end = eco::find_first(key, '/');
	if (node_end == -1)
	{
		// find the children.
		auto it = impl().m_children.begin();
		for (; it != impl().m_children.end(); ++it)
		{
			if (strcmp(key, it->get_name()) == 0)
				return *it;
		}
	}
	else
	{
		// recursive find the key value.
		auto it = impl().m_children.begin();
		for (; it != impl().m_children.end(); ++it)
		{
			if (strncmp(it->get_name(), key, node_end) == 0)
				return it->get_children(&key[node_end + 1]);
		}
	}
	EcoThrow << "get context node child error: " << key;
}


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(ContextNodeSet);
ECO_PROPERTY_SET_IMPL(ContextNodeSet, ContextNode);
////////////////////////////////////////////////////////////////////////////////
eco::ContextNodeSet ContextNodeSet::get_children(
	IN const char* parent_key) const
{
	int node_end = eco::find_first(parent_key, '/');
	for (auto it = impl().m_items.begin(); it != impl().m_items.end(); ++it)
	{
		// find the key node.
		if (node_end == -1 && strcmp(it->get_name(), parent_key) == 0)
		{
			return it->has_children() ? it->get_children() : eco::null;
		}

		// find the next level node.
		if (node_end != -1 &&
			strncmp(it->get_name(), parent_key, node_end) == 0)
		{
			return !it->has_children() ? eco::null
				: it->get_children().get_children(&parent_key[node_end + 1]);
		}
	}
	return eco::null;
}


 ////////////////////////////////////////////////////////////////////////////////
void ContextNodeSet::get_property_set(
	OUT eco::Context& result,
	IN const char* node_key) const
{
	int node_end = eco::find_first(node_key, '/');
	for (auto it = impl().m_items.begin(); it != impl().m_items.end(); ++it)
	{
		// find the key node.
		if (node_end == -1 && strcmp(it->get_name(), node_key) == 0)
		{
			result = it->get_property_set();
			return;
		}

		// find the next level node.
		if (node_end != -1 &&
			strncmp(it->get_name(), node_key, node_end) == 0)
		{
			if (it->has_children())
			{
				it->get_children().get_property_set(
					result, &node_key[node_end + 1]);
			}
			return;
		}
	}
}


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