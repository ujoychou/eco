#include "Pch.h"
#include <eco/Config.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Error.h>
#include <eco/rx/RxImpl.h>
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
		if (strcmp(it->name(), key) == 0)
		{
			return &it->value();
		}
	}
	return nullptr;
}
const StringAny& Context::at(IN const char* key) const
{
	auto v = find(key);
	if (!v)
	{
		ECO_THROW("find parameter fail: key=") << key;
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
		ECO_THROW("get context node key value fail: ") << key;
	return *any;
}
const StringAny ContextNode::get(IN const char* key) const
{
	auto v = find(key);
	return v ? v->c_str() : "";
}


////////////////////////////////////////////////////////////////////////////////
void merge_impl(ContextNode::Impl& a, ContextNode::Impl& b)
{
	if (a.m_name != b.m_name)
	{
		assert(false);
		return;
	}
	
	// merge value
	a.m_value = b.m_value;

	// merge property set.
	auto itb = b.m_property_set.begin();
	for (; itb != b.m_property_set.end(); ++itb)
	{
		auto ita = a.m_property_set.begin();
		for (; ita != a.m_property_set.end(); ++ita)
		{
			if (eco::equal(ita->name(), itb->name()))
			{
				ita->set_value(itb->value());
				break;
			}
		}
		if (ita == a.m_property_set.end())
		{
			a.m_property_set.push_back(*itb);
		}
	}

	// merge children.
	if (!b.m_children.null())
	{
		auto icb = b.m_children.begin();
		for (; icb != b.m_children.end(); ++icb)
		{
			if (a.m_children.null())
			{
				a.m_children = eco::heap;
				a.m_children.push_back(*icb);
				continue;
			}

			auto ica = a.m_children.begin();
			for (; ica != a.m_children.end(); ++ica)
			{
				if (ica->impl().m_name == icb->impl().m_name)
				{
					merge_impl(ica->impl(), icb->impl());
					break;
				}
			}
			if (ica == a.m_children.end())
			{
				a.m_children.push_back(*icb);
			}
		}
	}
}
void ContextNode::merge(eco::ContextNode& node)
{
	merge_impl(impl(), node.impl());
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
			if (strcmp(key, it->name()) == 0)
				return &it->value();
		}
	}
	else
	{
		// recursive find the key value.
		auto it = impl().m_children.begin();
		for (; it != impl().m_children.end(); ++it)
		{
			if (strncmp(it->name(), key, node_end) == 0)
			{
				return it->find(&key[node_end + 1]);
			}
		}
	}
	return nullptr;
}


////////////////////////////////////////////////////////////////////////////////
eco::ContextNode ContextNode::get_child(const char* key) const
{
	int node_end = eco::find_first(key, '/');
	if (node_end == -1)
	{
		if (impl().m_children.null()) return eco::null;

		// find the children.
		auto it = impl().m_children.begin();
		for (; it != impl().m_children.end(); ++it)
		{
			if (strcmp(key, it->name()) == 0)
				return *it;
		}
	}
	else
	{
		// recursive find the key value.
		auto it = impl().m_children.begin();
		for (; it != impl().m_children.end(); ++it)
		{
			if (strncmp(it->name(), key, node_end) == 0)
				return it->get_child(&key[node_end + 1]);
		}
	}
	return eco::null;
}


////////////////////////////////////////////////////////////////////////////////
eco::ContextNodeSet ContextNode::get_children(IN const char* key) const
{
	int node_end = eco::find_first(key, '/');
	if (node_end == -1)
	{
		if (impl().m_children.null()) return eco::null;
		// find the children.
		eco::ContextNodeSet result;
		auto it = impl().m_children.begin();
		for (; it != impl().m_children.end(); ++it)
		{
			if (strcmp(key, it->name()) == 0)
				result.add(*it);
		}
		return !result.empty() ? result : eco::null;
	}
	else
	{
		// recursive find the key value.
		auto it = impl().m_children.begin();
		for (; it != impl().m_children.end(); ++it)
		{
			if (strncmp(it->name(), key, node_end) == 0)
				return it->get_children(&key[node_end + 1]);
		}
	}
	return eco::null;
}


////////////////////////////////////////////////////////////////////////////////
bool ContextNode::get_property_set(eco::Context& result, const char* key) const
{
	auto node = get_child(key);
	if (!node.null())
	{
		result = node.property_set();
	}
	return !node.null();
}

////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(ContextNodeSet);
ECO_PROPERTY_SET_IMPL(ContextNodeSet, ContextNode);
ContextNode* ContextNodeSet::find(IN const char* name)
{
	for (size_t i = 0; i < impl().m_items.size(); ++i)
	{
		ContextNode& node = impl().m_items[i];
		if (node.impl().m_name == name)
			return &node;
	}
	return nullptr;
}
const ContextNode* ContextNodeSet::find(IN const char* name) const
{
	ContextNodeSet* pthis = (ContextNodeSet*)this;
	return pthis->find(name);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);