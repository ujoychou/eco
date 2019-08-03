#include "PrecHeader.h"
#include <eco/net/Subscriber.h>
////////////////////////////////////////////////////////////////////////////////



namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
eco::Map<TopicUid, Publisher::ptr> g_sub_map;
std::map<TopicUid, Publisher::ptr> g_map;
std::map<uint32_t, Publisher::ptr> g_typeid_map;
////////////////////////////////////////////////////////////////////////////////
void TopicUidMap::sub(IN const TopicUid& id, IN Publisher::ptr& pub)
{
	g_sub_map.set(id, pub);
}
void TopicUidMap::erase(IN const TopicUid& id)
{
	g_sub_map.erase(id);
}
Publisher::ptr TopicUidMap::get(IN const TopicUid& id)
{
	Publisher::ptr sub;
	g_sub_map.find(sub, id);
	return sub;
}


////////////////////////////////////////////////////////////////////////////////
void TopicUidMap::set(IN uint32_t id, IN Publisher::ptr& pub)
{
	g_typeid_map[id] = pub;
}
void TopicUidMap::set(IN const TopicUid& id, IN Publisher::ptr& pub)
{
	g_map[id] = pub;
}
Publisher::ptr TopicUidMap::get(IN const TopicUid& id, IN uint32_t type_id)
{
	// type id map.
	auto itt = g_typeid_map.find(type_id);
	if (itt != g_typeid_map.end())
	{
		return itt->second;
	}

	// topic uid map.
	auto itm = g_map.find(id);
	if (itm != g_map.end())
	{
		return itm->second;
	}

	return nullptr;
}


////////////////////////////////////////////////////////////////////////////////
}}