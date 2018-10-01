#ifndef ECO_TOPIC_H
#define ECO_TOPIC_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/thread/topic/Role.h>
#include <map>
#include <deque>
#include <unordered_map>


ECO_NS_BEGIN(eco);
//##############################################################################
//##############################################################################
template<typename TopicId = eco::TopicId>
class OneTopic : public PopTopic<TopicId>
{
	ECO_TOPIC(OneTopic, PopTopic<TopicId>);
protected:
	virtual void do_snap(
		IN eco::Subscription& node,
		IN eco::TopicEvent* event) override
	{
		if (m_snap.get() != nullptr)
		{
			auto* suber = (Subscriber*)node.m_subscriber;
			ContentSnap type = content_snap | content_head | content_last;
			m_snap->stamp() = eco::meta::stamp_insert;
			suber->on_publish(*this, Content(m_snap, type, event));
		}
	}

	virtual bool do_move(OUT eco::ContentData::ptr& newc) override
	{
		m_snap = newc;		// update snap with new data.
		return true;
	}

	virtual void do_clear() override
	{
		m_snap.reset();
	}

	inline ContentData::ptr snap() const
	{
		eco::Mutex::ScopeLock lock(mutex());
		return m_snap;
	}

	// latest data.
	eco::ContentData::ptr m_snap;
};


//##############################################################################
//##############################################################################
template<typename TopicId = eco::TopicId>
class SeqTopic : public PopTopic<TopicId>
{
	ECO_TOPIC(SeqTopic, PopTopic<TopicId>);
public:
	/* exp"market kline", last content can be updated as follow:
	1) get seq topic last content, check it's id equal with new item?
	2) if they are equal, merge new item into last content.
	3) publish last content.
	*/
	inline ContentData::ptr last() const
	{
		eco::Mutex::ScopeLock lock(mutex());
		return !m_snap_set.empty() ? m_snap_set.back() : nullptr;
	}
	inline ContentData::ptr head() const
	{
		eco::Mutex::ScopeLock lock(mutex());
		return !m_snap_set.empty() ? m_snap_set.front() : nullptr;
	}

	inline const uint32_t size() const
	{
		eco::Mutex::ScopeLock lock(mutex());
		return m_snap_set.size();
	}
	inline const std::deque<eco::ContentData::ptr>& snap_set_raw() const
	{
		return m_snap_set;
	}

	// add "object set/object/shared_object" to topic by "load".
	template<typename object_set_t>
	inline void push_front_set(IN const object_set_t& set)
	{
		eco::Mutex::ScopeLock lock(mutex());
		for (auto it = set.rbegin(); it != set.rend(); ++it)
		{
			push_front_raw(*it);
		}
	}
	template<typename object_t>
	inline void push_front(IN const object_t& obj)
	{
		eco::Mutex::ScopeLock lock(mutex());
		push_front_raw(obj);
	}
	template<typename object_t>
	inline void push_front(IN const std::shared_ptr<object_t>& obj)
	{
		eco::Mutex::ScopeLock lock(mutex());
		push_front_raw(obj);
	}

	inline void pop_front()
	{
		eco::Mutex::ScopeLock lock(mutex());
		m_snap_set.pop_front();
	}

	template<typename object_t>
	inline void push_front_raw(IN const object_t& obj)
	{
		ContentData::ptr newc(new ContentDataT<
			object_t, object_t>(obj, eco::meta::stamp_insert));
		m_snap_set.push_front(newc);
	}
	template<typename object_t>
	inline void push_front_raw(IN const std::shared_ptr<object_t>& obj)
	{
		typedef std::shared_ptr<object_t> value_t;
		ContentData::ptr newc(new ContentT<
			object_t, value_t>(obj, eco::meta::stamp_insert));
		m_snap_set.push_front(newc);
	}

protected:
	virtual void do_snap(
		IN eco::Subscription& node,
		IN eco::TopicEvent* event) override
	{
		// get start seq.
		size_t seq = event ? event->get_snap_seq(*this) : 0;
		if (seq > m_snap_set.size())
		{
			seq = 0;
			EcoError << "seq topic hasn't enough content to publish snap "
				<< "seq > size()" << seq <= m_snap_set.size();
		}
		size_t seq_init = seq + 1;
		eco::ContentSnap type = eco::content_snap | eco::content_head;

		// *node subscriber may be destruct in "on_publish";
		// publish the snap, and set a content type.
		auto* suber = (eco::Subscriber*)node.m_subscriber;
		for (; seq < m_snap_set.size() && node.m_working; ++seq)
		{
			if (seq == seq_init)
				eco::del(type, eco::content_head);
			if (seq == m_snap_set.size() - 1)
				eco::add(type, eco::content_last);

			auto& content = m_snap_set[seq];
			content->stamp() = eco::meta::stamp_insert;
			suber->on_publish(*this, Content(content, type, event));
		}
	}

	virtual bool do_move(OUT eco::ContentData::ptr& newc) override
	{
		// update last content.
		if (eco::meta::is_update(newc->get_stamp()) && m_snap_set.size() > 0)
			m_snap_set.back() = newc;
		else
			m_snap_set.push_back(newc);
		return true;
	}

	virtual void do_clear() override
	{
		m_snap_set.clear();
	}

	// history data.
	std::deque<eco::ContentData::ptr> m_snap_set;
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
	typename ObjectMap = std::unordered_map<ObjectId, eco::ContentData::ptr> >
class SetTopic : public PopTopic<TopicId>
{
	ECO_TOPIC(SetTopic, PopTopic<TopicId>);
public:
	// find object by identity.
	template<typename value_t>
	inline bool find(OUT value_t& v, IN  const ObjectId& id) const
	{
		ContentData::ptr c = find(id);
		if (c != nullptr) v = *(value_t*)c->get_value();
		return c != nullptr;
	}

	// find content by identity.
	inline ContentData::ptr find(IN const ObjectId& id) const
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
	// replace old content with new content.
	// note that: old_c may be nullptr, when insert first content.
	virtual bool do_update(
		IN eco::ContentData::ptr& new_c,
		IN eco::ContentData::ptr& old_c)
	{
		old_c = new_c;
		return true;
	}

	virtual void do_snap(
		IN eco::Subscription& node,
		IN eco::TopicEvent* event) override
	{
		if (m_snap_set.empty()) return;

		// *node subscriber may be destruct in "on_publish";
		// publish the snap, and set a content type.
		uint32_t index = 0;
		auto* suber = (Subscriber*)node.m_subscriber;
		ContentSnap snap = content_snap | content_head;
		auto it = m_snap_set.begin();
		for (; it != m_snap_set.end() && node.m_working; ++it, ++index)
		{
			if (index == 1)
				eco::del(snap, content_head);
			if (index == m_snap_set.size() - 1)
				eco::add(snap, content_last);
			it->second->stamp() = eco::meta::stamp_insert;
			suber->on_publish(*this, Content(it->second, snap, event));
		}
	}

	virtual bool do_move(OUT eco::ContentData::ptr& newc) override
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
			auto& oldc = m_snap_set[obj_id];
			newc->stamp() = (oldc != nullptr)
				? eco::meta::stamp_update
				: eco::meta::stamp_insert;
			return do_update(newc, oldc);
		}
		return true;
	}

	virtual void do_clear() override
	{
		m_snap_set.clear();
	}

	ObjectMap m_snap_set;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif