#ifndef ECO_MODEL_H
#define ECO_MODEL_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/rx/RxApi.h>
#include <eco/persist/Persist.h>
#include <eco/proto/Proto.h>
#include <eco/thread/topic/TopicServer.h>


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
// mapping index defined.
enum
{
	map_control			= 1001,		// object control mapping for user view.
	map_persist			= 1002,		// object persist mapping for data source.
	map_defined			= 2001,		// user defined map.
};


////////////////////////////////////////////////////////////////////////////////
template<typename object_t, typename meta_t>
class Model
{
//////////////////////////////////////////////////////////////// [define object]
public:
	typedef object_t::value value;

	// init model topic.
	inline void init(IN eco::TopicServer& topic_server)
	{
		m_topic_server = &topic_server;
	}

	// get a object mapping of view or persist by map index.
	inline ObjectMapping& map(IN const uint32_t v = map_persist)
	{
		ObjectMapping::ptr& ptr = m_map[v];
		if (ptr == nullptr)
			ptr.reset(new ObjectMapping());
		return *ptr;
	}

//////////////////////////////////////////////////////////////// [access object]
public:
	// init model with persist data source.
	inline void init(IN Persist& persist, IN uint32_t map_v = map_persist)
	{
		std::vector<value> data_set;
		persist.master().select<meta_t>(data_set, map(map_v));
		for (auto it = data_set.begin(); it != data_set.end(); ++it)
		{
			m_topic_server->publish(*it);
		}
	}

	// insert a new object or update a exist object.
	inline void save(
		IN value& obj,
		IN Persist& persist, 
		IN uint32_t map_v = map_persist)
	{
		persist.master().save<meta_t>(obj, map(map_v), obj.timestamp());
	}
	inline void save(
		IN value& obj,
		IN Persist& persist,
		IN meta::Timestamp ts,
		IN uint32_t map_v = map_persist)
	{
		persist.master().save<meta_t>(obj, map(map_v), ts);
	}

	// insert a new object or update a exist object.
	template<typename meta_t>
	inline void save(
		IN value& obj,
		IN Persist& source,
		IN const uint32_t map = map_persist)
	{
		persist.master().save<meta_t>(obj, map(map_v), obj.timestamp());
	}
	template<typename meta_t>
	inline void save(
		IN value& obj,
		IN Persist& persist,
		IN meta::Timestamp ts,
		IN uint32_t map_v = map_persist)
	{
		persist.master().save<meta_t>(obj, map(map_v), ts);
	}

	// remove a object.
	inline void remove(IN const uint64_t id)
	{
	}

	// update property value of object.
	void update(IN const ::proto::Property& id);

private:
	eco::TopicServer* m_topic_server;
	std::unordered_map<uint32_t, ObjectMapping::ptr> m_map;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif