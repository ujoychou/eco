#ifndef ECO_TOPIC_H
#define ECO_TOPIC_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/thread/topic/Role.h>
#include <unordered_map>
#include <deque>
#include <map>


ECO_NS_BEGIN(eco);
//##############################################################################
//##############################################################################
template<typename TopicId = eco::TopicId>
class OneTopic : public PopTopic<TopicId>
{
	ECO_TOPIC_TOPIC(OneTopic);
protected:
	virtual void do_snap(IN Subscriber& suber) override
	{
		if (m_snap.get() != nullptr)
		{
			suber.on_publish(m_id, m_snap, content_snap_end);
		}
	}

	virtual void do_move(OUT Content::ptr& newc) override
	{
		m_snap = newc;		// update snap with new data.
	}

	virtual void do_clear() override
	{
		m_snap.reset();
	}

	// latest data.
	eco::Content::ptr m_snap;
};


//##############################################################################
//##############################################################################
template<typename TopicId = eco::TopicId>
class SeqTopic : public PopTopic<TopicId>
{
	ECO_TOPIC_TOPIC(SeqTopic);
public:
	/* exp"market kline", last content can be updated as follow:
	1) get seq topic last content, check it's id equal with new item?
	2) if they are equal, merge new item into last content.
	3) publish last content.
	*/
	inline Content::ptr last() const
	{
		eco::Mutex::ScopeLock lock(mutex());
		return !m_snap_set.empty() ? m_snap_set.back() : nullptr;
	}

protected:
	virtual void do_snap(IN Subscriber& suber) override
	{
		if (!m_snap_set.empty())
		{
			auto it_end = m_snap_set.end(); --it_end;
			for (auto it = m_snap_set.begin(); it != m_snap_set.end(); ++it)
			{
				auto snap = (it != it_end) ? content_snap : content_snap_end;
				suber.on_publish(m_id, *it, snap);
			}
		}
	}

	virtual void do_move(OUT Content::ptr& newc) override
	{
		// update last content.
		if (eco::meta::is_update(newc->get_stamp()))
			m_snap_set.back() = newc;
		else
			m_snap_set.push_back(newc);
	}

	virtual void do_clear() override
	{
		m_snap_set.clear();
	}

	// history data.
	std::deque<eco::Content::ptr> m_snap_set;
};


//##############################################################################
//##############################################################################
class IdAdapter
{
public:
	template<typename ObjectId, typename Object, typename TopicId>
	inline void get_id(ObjectId& id, const Object& obj, const TopicId& tid)
	{
		id = obj.id();
	}
};
class GetIdAdapter
{
public:
	template<typename ObjectId, typename Object, typename TopicId>
	inline void get_id(ObjectId& id, const Object& obj, const TopicId& tid)
	{
		id = obj.get_id();
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename ObjectId, typename Object, 
	typename ObjectIdAdapter = GetIdAdapter,
	typename TopicId = eco::TopicId,
	typename ObjectMap = std::unordered_map<ObjectId, eco::Content::ptr> >
class SetTopic : public PopTopic<TopicId>
{
	ECO_TOPIC_TOPIC(SetTopic);
public:
	// find object by identity.
	template<typename value_t>
	inline bool find(OUT value_t& v, IN  const ObjectId& id) const
	{
		Content::ptr c = find(id);
		if (c != nullptr) v = *(value_t*)c->get_value();
		return c != nullptr;
	}

	// find content by identity.
	inline Content::ptr find(IN const ObjectId& id) const
	{
		eco::Mutex::ScopeLock lock(mutex());
		auto it = m_snap_set.find(id);
		return (it != m_snap_set.end()) ? it->second : nullptr;
	}

	// get objects map.
	inline const ObjectMap& get_object_map() const
	{
		return m_snap_set;
	}

protected:
	virtual void do_snap(IN Subscriber& suber) override
	{
		if (!m_snap_set.empty())
		{
			auto it_end = m_snap_set.end(); --it_end;
			for (auto it = m_snap_set.begin(); it != m_snap_set.end(); ++it)
			{
				auto snap = (it != it_end) ? content_snap : content_snap_end;
				suber.on_publish(m_id, it->second, snap);
			}
		}
	}

	virtual void do_move(OUT Content::ptr& newc) override
	{
		// get object id.
		ObjectId obj_id;
		ObjectIdAdapter adapt;
		adapt.get_id(obj_id, *(Object*)newc->get_set_topic_object(), m_id);
		if (eco::meta::is_remove(newc->get_stamp()))
		{
			newc->stamp() = eco::meta::stamp_remove;
			m_snap_set.erase(obj_id);
		}
		else
		{
			newc->stamp() = eco::meta::stamp_clean;
			m_snap_set[obj_id] = newc;
		}
	}

	virtual void do_clear() override
	{
		m_snap_set.clear();
	}

protected:
	ObjectMap m_snap_set;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif