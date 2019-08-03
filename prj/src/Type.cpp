#include "PrecHeader.h"
#include <eco/Type.h>
////////////////////////////////////////////////////////////////////////////////
#include <map>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
uint32_t __typeid(const char* type_info_name)
{
	static uint32_t type_id = 0;
	static std::map<std::string, uint32_t> map;
	auto it = map.find(type_info_name);
	if (it != map.end())
	{
		return it->second;
	}
	return (map[type_info_name] = ++type_id);
}


////////////////////////////////////////////////////////////////////////////////
ECO_VALUE_IMPL(Context);
ECO_PROPERTY_SET_IMPL(Context, Parameter);
////////////////////////////////////////////////////////////////////////////////
bool Context::has(IN const char* key) const
{
	return find(key) != nullptr;
}
const StringAny* Context::find(IN const char* key) const
{
	auto it = impl().m_items.begin();
	for (; it != impl().m_items.end(); ++it)
	{
		if (strcmp(it->get_name(), key) == 0)
		{
			return &it->get_value();
		}
	}
	return nullptr;
}
const StringAny& Context::at(IN const char* key) const
{
	auto v = find(key);
	if (!v)
	{
		ECO_THROW(0, "find parameter fail: key=") << key;
	}
	return *v;
}
const StringAny Context::get(IN const char* key) const
{
	auto v = find(key);
	return v ? *v : "";
}


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(ContextNode);
ECO_PROPERTY_STR_IMPL(ContextNode, name);
ECO_PROPERTY_STR_IMPL(ContextNode, value);
ECO_PROPERTY_VAL_IMPL(ContextNode, Context, property_set);
ECO_PROPERTY_OBJ_IMPL(ContextNode, ContextNodeSet, children);
////////////////////////////////////////////////////////////////////////////////
const StringAny& ContextNode::at(IN const char* key) const
{
	auto* any = find(key);
	if (any == nullptr)
		ECO_THROW(0, "get context node key value fail: ") << key;
	return *any;
}
const StringAny ContextNode::get(IN const char* key) const
{
	auto v = find(key);
	return v ? v->c_str() : "";
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
			if (strcmp(key, it->get_name()) == 0)
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
	ECO_THROW(0, "get context node child error: ") << key;
}


////////////////////////////////////////////////////////////////////////////////
bool ContextNode::get_property_set(eco::Context& result, const char* key) const
{
	auto nodes = impl().m_children;
	return !nodes.null() ? nodes.get_property_set(result, key) : false;
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
bool ContextNodeSet::get_property_set(
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
			return true;
		}

		// find the next level node.
		if (node_end != -1 &&
			strncmp(it->get_name(), node_key, node_end) == 0)
		{
			if (it->has_children())
			{
				return it->get_children().get_property_set(
					result, &node_key[node_end + 1]);
			}
		}
	}
	return false;
}
ECO_NS_END(eco);