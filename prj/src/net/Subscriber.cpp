#include "Pch.h"
#include <eco/net/Subscriber.h>
////////////////////////////////////////////////////////////////////////////////



ECO_NS_BEGIN(eco);
namespace net{;
////////////////////////////////////////////////////////////////////////////////
// dispatcher mode thread safe.
std::map<TopicUid, Publisher::ptr> g_map;
std::map<uint32_t, Publisher::ptr> g_typeid_map;
void TopicUidMap::set(IN uint32_t id, IN Publisher::ptr& pub)
{
	g_typeid_map[id] = pub;
}
void TopicUidMap::set(IN const TopicUid& id, IN Publisher::ptr& pub)
{
	g_map[id] = pub;
}

////////////////////////////////////////////////////////////////////////////////
// subscriber mode thread unsafe.
eco::Map<TopicUid, Publisher::ptr> g_sub_map;
void TopicUidMap::sub(IN const TopicUid& id, IN Publisher::ptr& pub)
{
	g_sub_map.set(id, pub);
}
void TopicUidMap::erase(IN const TopicUid& id)
{
	g_sub_map.erase(id);
}


////////////////////////////////////////////////////////////////////////////////
Publisher::ptr TopicUidMap::get(IN const TopicUid& id, IN uint32_t type_id)
{
	// priority: g_sub_map > g_map > g_typeid_map.
	// #1.topic ids.
	Publisher::ptr sub_;
	if (g_sub_map.find(sub_, id))
	{
		return sub_;
	}

	// #2.topic uid map.
	auto itm = g_map.find(id);
	if (itm != g_map.end())
	{
		return itm->second;
	}

	// #3.type id map.
	auto itt = g_typeid_map.find(type_id);
	if (itt != g_typeid_map.end())
	{
		return itt->second;
	}
	return sub_;
}


////////////////////////////////////////////////////////////////////////////////
}}