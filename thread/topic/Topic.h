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

	inline eco::Mutex& content_mutex() const
	{
		return m_content_mutex;
	}

	virtual void append(IN eco::Content::ptr& content) override
	{
		eco::Mutex::ScopeLock lock(content_mutex());
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
			m_snap->timestamp().origin();
			suber.on_publish(m_id, m_snap);
		}
	}

	virtual bool do_move(OUT std::vector<eco::Content::ptr>& new_set) override
	{
		eco::Mutex::ScopeLock lock(content_mutex());
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
		eco::Mutex::ScopeLock lock(content_mutex());
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

	inline eco::Mutex& content_mutex() const
	{
		return m_content_mutex;
	}

	virtual void append(IN eco::Content::ptr& content) override
	{
		eco::Mutex::ScopeLock lock(content_mutex());
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
		eco::Mutex::ScopeLock lock(content_mutex());
		if (m_new_set.empty())
		{
			return false;
		}

		// get new data.
		new_set.reserve(m_new_set.size());
		// update snap.
		std::for_each(m_new_set.begin(), m_new_set.end(), 
			[&](eco::Content::ptr& item) {
			new_set.push_back(item);
			m_snap_set.push_back(item);
		});
		m_new_set.clear();
		return true;
	}

	virtual void do_clear() override
	{
		m_snap_set.clear();
		eco::Mutex::ScopeLock lock(content_mutex());
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
class SetTopicAdapter
{
public:
	template<typename Object, typename ObjectId, typename TopicId>
	inline void operator()(
		OUT ObjectId& id,
		IN  const Object& obj,
		IN  const TopicId& tid)
	{
		id = obj.get_id();
	}

	template<typename Object, typename ObjectId, typename TopicId>
	inline void operator()(
		OUT ObjectId& id,
		IN  const Object* obj,
		IN  const TopicId& tid)
	{
		id = obj->get_id();
	}

	template<typename Object, typename ObjectId, typename TopicId>
	inline void operator()(
		OUT ObjectId& id, 
		IN  const std::shared_ptr<Object>& obj,
		IN  const TopicId& tid)
	{
		id = obj->get_id();
	}
};

////////////////////////////////////////////////////////////////////////////////
template<
	typename Object, 
	typename ObjectId,
	typename ObjectIdAdapter = SetTopicAdapter,
	typename TopicId = eco::TopicId,
	typename ObjectMap = std::unordered_map<ObjectId, eco::Content::ptr>>
class SetTopic : public TopicT<TopicId>
{
	ECO_TOPIC(SetTopic);
public:
	typedef std::function<void(
		IN eco::Content::ptr& newc)> OnErrorFunc;
	typedef std::function<bool(
		IN eco::Content::ptr& newc)> OnInsertFunc;
	typedef std::function<bool(
		IN eco::Content::ptr& newc,
		IN eco::Content::ptr& oldc)> OnUpdateFunc;

	inline static void on_error(
		IN eco::Content::ptr& newc)
	{}

	inline static bool on_insert(
		IN eco::Content::ptr& newc)
	{
		return true;
	}

	inline static bool on_update(
		IN eco::Content::ptr& newc,
		IN eco::Content::ptr& oldc)
	{
		return true;
	}

public:
	// constructor.
	inline SetTopic()
	{
		m_on_error  = on_error;
		m_on_insert = on_insert;
		m_on_update = on_update;
	}

	// set callback function
	inline void set_function(
		IN OnErrorFunc e,
		IN OnInsertFunc i,
		IN OnInsertFunc u)
	{
		m_on_error  = e;
		m_on_insert = i;
		m_on_update = u;
	}

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
	template<typename Object>
	inline bool find(OUT Object& obj, IN  const ObjectId& id) const
	{
		eco::Mutex::ScopeLock lock(mutex());
		auto it = m_objects.find(id);
		if (it == m_objects.end())
		{
			return false;
		}
		obj = *(Object*)it->second->get_value();
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
		Object* obj = (Object*)newc->get_set_topic_object();
		ObjectIdAdapter()(obj_id, *obj, m_id);

		// fuzzy updated state, newc can be "inserted" or "updated".
		auto it = m_objects.find(obj_id);
		if (it != m_objects.end())
		{
			// remove item.
			if (newc->get_timestamp().is_removed())
			{
				it->second->timestamp().set_value(eco::meta::removed);
				m_new_set.push_back(it->second);
				m_objects.erase(it);
			}
			// update item.
			else
			{
				newc->timestamp().set_value(eco::meta::updated);
				if (m_on_update(newc, it->second))
				{
					m_new_set.push_back(newc);
					it->second = newc;
				}
			}
		}
		// insert item, ignore removed item.
		else if (!newc->get_timestamp().is_removed())
		{
			newc->timestamp().set_value(eco::meta::inserted);
			// set the new content with a identity if newc don't have that.
			if (m_on_insert(newc))
			{
				m_new_set.push_back(newc);
				m_objects[obj_id] = newc;
			}
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
			Object* obj = (Object*)(**it).get_set_topic_object();
			ObjectIdAdapter()(obj_id, *obj, m_id);

			// update item or insert item.
			if ((**it).get_timestamp().is_updated()  ||
				(**it).get_timestamp().is_inserted() ||
				(**it).get_timestamp().is_original())
			{
				new_set.push_back(*it);
				m_snap_set[obj_id] = *it;
			}
			// remove item.
			else if ((**it).get_timestamp().is_removed())
			{
				auto it_snap = m_snap_set.find(obj_id);
				if (it_snap != m_snap_set.end())
				{
					it_snap->second->timestamp().remove();
					new_set.push_back(it_snap->second);
					m_snap_set.erase(it_snap);
				}
			}
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

	// on function.
	OnErrorFunc		m_on_error;
	OnInsertFunc	m_on_insert;
	OnUpdateFunc	m_on_update;
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

	inline eco::Mutex& content_mutex() const
	{
		return m_content_mutex;
	}

	virtual void append(IN eco::Content::ptr& content) override
	{
		eco::Mutex::ScopeLock lock(content_mutex());
		m_new_set.push_back(content);
	}

protected:
	virtual void do_snap(IN Subscriber& suber) override
	{}

	virtual bool do_move(OUT std::vector<eco::Content::ptr>& new_set) override
	{
		eco::Mutex::ScopeLock lock(content_mutex());
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
		eco::Mutex::ScopeLock lock(content_mutex());
		m_new_set.clear();
	}

	// new data that never sended to subscriber.
	std::vector<eco::Content::ptr> m_new_set;
	mutable eco::Mutex m_content_mutex;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif




