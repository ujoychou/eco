#ifndef ECO_TOPIC_H
#define ECO_TOPIC_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/thread/topic/Role.h>
#include <unordered_map>
#include <deque>
#include <map>


ECO_NS_BEGIN(eco);


//##############################################################################
//##############################################################################
template<typename TopicId = eco::TopicId>
class OneTopic : public TopicT<TopicId>
{
	ECO_TOPIC(OneTopic);
public:
	inline OneTopic()
	{}

	inline eco::Mutex& mutex() const
	{
		return m_content_mutex;
	}

	virtual void append(IN eco::Content::ptr& content) override
	{
		eco::Mutex::ScopeLock lock(mutex());
		if (content.get() != nullptr)
		{
			m_new = content;
		}
	}

protected:
	virtual void do_snap(IN Subscriber& suber) override
	{
		if (m_snap.get() != nullptr)
		{
			eco::meta::clear(m_snap->timestamp());
			suber.on_publish(m_id, m_snap);
		}
	}

	virtual bool do_move(OUT std::vector<eco::Content::ptr>& new_set) override
	{
		eco::Mutex::ScopeLock lock(mutex());
		if (m_new == nullptr)
		{
			return false;
		}

		// update snap with new data.
		m_snap = m_new;
		new_set.push_back(std::move(m_new));
		return true;
	}

	virtual void do_clear() override
	{
		m_snap.reset();
		eco::Mutex::ScopeLock lock(mutex());
		m_new.reset();
	}

protected:
	// latest data.
	eco::Content::ptr m_snap;

	// new data that never sended to subscriber.
	eco::Content::ptr m_new;
	mutable eco::Mutex m_content_mutex;
};


//##############################################################################
//##############################################################################
template<typename TopicId = eco::TopicId>
class SeqTopic : public TopicT<TopicId>
{
	ECO_TOPIC(SeqTopic);
public:
	inline SeqTopic()
	{}

	inline eco::Mutex& mutex() const
	{
		return m_content_mutex;
	}

	virtual void append(IN eco::Content::ptr& content) override
	{
		eco::Mutex::ScopeLock lock(mutex());
		m_new_set.push_back(content);
	}

protected:
	virtual void do_snap(IN Subscriber& suber) override
	{
		for (auto it = m_snap_set.begin(); it != m_snap_set.end(); ++it)
		{
			suber.on_publish(m_id, *it);
		}
	}

	virtual bool do_move(OUT std::vector<eco::Content::ptr>& new_set) override
	{
		eco::Mutex::ScopeLock lock(mutex());
		if (m_new_set.empty())
		{
			return false;
		}

		// get new data.
		new_set.reserve(m_new_set.size());
		// update snap.
		for (auto it = m_new_set.begin(); it != m_new_set.end(); ++it)
		{
			new_set.push_back(*it);
			m_snap_set.push_back(*it);
		}
		m_new_set.clear();
		return true;
	}

	virtual void do_clear() override
	{
		m_snap_set.clear();
		eco::Mutex::ScopeLock lock(mutex());
		m_new_set.clear();
	}

	// history data.
	std::deque<eco::Content::ptr> m_snap_set;

	// new data that never sended to subscriber.
	std::vector<eco::Content::ptr> m_new_set;
	mutable eco::Mutex m_content_mutex;
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
class SetTopic : public TopicT<TopicId>
{
	ECO_TOPIC(SetTopic);
public:
	inline SetTopic(){};

	// content mutex.
	inline eco::Mutex& mutex() const
	{
		return m_content_mutex;
	}
	// get objects map.
	inline const ObjectMap& get_object_map() const
	{
		return m_objects;
	}

	// find object by identity.
	template<typename value_t>
	inline bool find(OUT value_t& v, IN  const ObjectId& id) const
	{
		eco::Mutex::ScopeLock lock(mutex());
		auto it = m_objects.find(id);
		if (it == m_objects.end())
		{
			return false;
		}
		v = *(value_t*)it->second->get_value();
		return true;
	}

	// find content by identity.
	inline eco::Content::ptr find(IN const ObjectId& id) const
	{
		eco::Mutex::ScopeLock lock(mutex());
		auto it = m_objects.find(id);
		return (it != m_objects.end()) ? it->second : nullptr;
	}

	virtual void append(IN eco::Content::ptr& newc) override
	{
		eco::Mutex::ScopeLock lock(mutex());
		ObjectId obj_id;
		ObjectIdAdapter adapt;
		adapt.get_id(obj_id, *(Object*)newc->get_set_topic_object(), m_id);

		// fuzzy updated state, newc can be "inserted" or "updated".
		auto it = m_objects.find(obj_id);
		if (it != m_objects.end())
		{
			// remove item.
			if (eco::meta::removed(newc->get_timestamp()))
			{
				it->second->timestamp() = eco::meta::v_remove;
				m_new_set.push_back(it->second);
				m_objects.erase(it);
			}
			// update item.
			else
			{
				newc->timestamp() = eco::meta::v_update;
				m_new_set.push_back(newc);
				it->second = newc;
			}
		}
		// insert item, ignore removed item.
		else if (!eco::meta::removed(newc->get_timestamp()))
		{
			newc->timestamp() = eco::meta::v_insert;
			m_new_set.push_back(newc);
			m_objects[obj_id] = newc;
		}
	}

protected:
	virtual void do_snap(IN Subscriber& suber) override
	{
		for (auto it = m_snap_set.begin(); it != m_snap_set.end(); ++it)
		{
			suber.on_publish(m_id, it->second);
		}
	}

	virtual bool do_move(OUT std::vector<eco::Content::ptr>& new_set) override
	{
		eco::Mutex::ScopeLock lock(mutex());
		if (m_new_set.empty())
		{
			return false;
		}

		new_set.reserve(m_new_set.size());
		for (auto it = m_new_set.begin(); it != m_new_set.end(); ++it)
		{
			ObjectId obj_id;
			ObjectIdAdapter adapt;
			adapt.get_id(obj_id, *(Object*)(**it).get_set_topic_object(), m_id);
			// sync snap data.
			if (eco::meta::removed((**it).get_timestamp()))
				m_snap_set.erase(obj_id);
			else
				m_snap_set[obj_id] = *it;

			new_set.push_back(*it);
		}
		m_new_set.clear();
		return new_set.size() > 0;
	}

	virtual void do_clear() override
	{
		m_snap_set.clear();
		eco::Mutex::ScopeLock lock(mutex());
		m_new_set.clear();
		m_objects.clear();
	}

protected:
	// object set snap.
	ObjectMap m_snap_set;
	ObjectMap m_objects;

	// new data that never sended to subscriber.
	std::vector<eco::Content::ptr> m_new_set;
	mutable eco::Mutex m_content_mutex;
};


//##############################################################################
//##############################################################################
template<typename TopicId = eco::TopicId>
class PopTopic : public TopicT<TopicId>
{
	ECO_TOPIC(PopTopic);
public:
	inline PopTopic()
	{}

	inline eco::Mutex& mutex() const
	{
		return m_content_mutex;
	}

	virtual void append(IN eco::Content::ptr& content) override
	{
		eco::Mutex::ScopeLock lock(mutex());
		m_new_set.push_back(content);
	}

protected:
	virtual void do_snap(IN Subscriber& suber) override
	{}

	virtual bool do_move(OUT std::vector<eco::Content::ptr>& new_set) override
	{
		eco::Mutex::ScopeLock lock(mutex());
		if (m_new_set.empty())
			return false;

		// get new data.
		new_set.reserve(m_new_set.size());
		std::for_each(m_new_set.begin(), m_new_set.end(),
			[&](eco::Content::ptr& item) {
			new_set.push_back(item);
		});
		m_new_set.clear();
		return true;
	}

	virtual void do_clear() override
	{
		eco::Mutex::ScopeLock lock(mutex());
		m_new_set.clear();
	}

	// new data that never sended to subscriber.
	std::vector<eco::Content::ptr> m_new_set;
	mutable eco::Mutex m_content_mutex;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif




