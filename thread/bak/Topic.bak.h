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
class TopicId
{
public:
	uint64_t m_object_id;
	uint32_t m_aspect_id;
	uint32_t m_object_type;

	inline TopicId()
		: m_object_id(0)
		, m_aspect_id(0)
		, m_object_type(0)
	{}

	inline TopicId(
		IN const uint32_t object_type,
		IN const uint32_t aspect_id,
		IN const uint64_t object_id = 0)
		: m_object_id(object_id)
		, m_aspect_id(aspect_id)
		, m_object_type(object_type)
	{}

	inline void set(
		IN const uint32_t object_type,
		IN const uint32_t aspect_id,
		IN const uint64_t object_id = 0)
	{
		m_object_id = object_id;
		m_aspect_id = aspect_id;
		m_object_type = object_type;
	}

	inline uint64_t hash_value() const
	{
		uint64_t result = m_aspect_id;
		result += m_object_type * 100;
		result += m_object_id * 10000;
		return result;
	}

	inline bool operator==(IN const TopicId& topic_id) const
	{
		return m_object_type == topic_id.m_object_type
			&& m_object_id == topic_id.m_object_id
			&& m_aspect_id == topic_id.m_aspect_id;
	}

	inline bool equal(
		IN const uint32_t object_type,
		IN const uint32_t aspect_id,
		IN const uint64_t object_id = 0) const
	{
		return m_object_type == object_type
			&& m_object_id == object_id
			&& m_aspect_id == aspect_id;
	}
};


////////////////////////////////////////////////////////////////////////////////
class TopicIdHash
{
public:
	inline std::size_t operator()(IN const TopicId& v) const
	{
		return static_cast<std::size_t>(eco::hash_combine(v.hash_value()));
	}
};



//##############################################################################
//##############################################################################
// 单数据主题
template<typename TopicId>
class OneTopic : public Topic<TopicId>
{
	ECO_CLASS_TOPIC(OneTopic);
public:
	typedef eco::OneTopic<TopicId> ClassTopic;

	inline OneTopic()
	{}

	virtual void publish(IN eco::Content::ptr& content) override
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		if (content.get() != nullptr)
		{
			m_new = content;
		}
	}

protected:
	virtual void do_publish_snap(IN Subscriber& suber) override
	{
		if (m_snap.get() != nullptr)
		{
			suber.on_publish(m_id, m_snap);
		}
	}

	virtual bool do_move(OUT std::vector<eco::Content::ptr>& new_set) override
	{
		eco::Mutex::ScopeLock lock(m_mutex);
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
		eco::Mutex::ScopeLock lock(m_mutex);
		m_new.reset();
	}

protected:
	// latest data.
	eco::Content::ptr m_snap;

	// new data that never sended to subscriber.
	eco::Content::ptr m_new;
	mutable eco::Mutex m_mutex;
};


//##############################################################################
//##############################################################################
// 序列数据主题
template<typename TopicId>
class SerialTopic : public Topic<TopicId>
{
	ECO_CLASS_TOPIC(SerialTopic);
public:
	virtual void publish(IN eco::Content::ptr& content) override
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		m_new_set.push_back(content);
	}

protected:
	virtual void do_publish_snap(IN Subscriber& suber) override
	{
		for (auto it = m_snap_set.begin(); it != m_snap_set.end(); ++it)
		{
			suber.on_publish(m_id, *it);
		}
	}

	virtual bool do_move(OUT std::vector<eco::Content::ptr>& new_set) override
	{
		eco::Mutex::ScopeLock lock(m_mutex);
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
		eco::Mutex::ScopeLock lock(m_mutex);
		m_new_set.clear();
	}

	// history data.
	std::deque<eco::Content::ptr> m_snap_set;

	// new data that never sended to subscriber.
	std::vector<eco::Content::ptr> m_new_set;
	mutable eco::Mutex m_mutex;
};


//##############################################################################
//##############################################################################
// 多数据主题
// 对象管理主题
template<typename TopicId, typename ContentId, 
	typename ContentMap = std::unordered_map<ContentId, eco::Content::ptr >>
class SetTopic : public Topic<TopicId>
{
	ECO_CLASS_TOPIC(SetTopic);
public:
	typedef eco::SetContent<ContentId> SetContent;
	typedef eco::SetTopic<TopicId, ContentId, ContentMap> SuperTopic;

	template<typename Content>
	inline bool find_content(
		OUT std::shared_ptr<Content>& content,
		IN  const ContentId& content_id)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		auto it_snap = m_snap_set.find(content_id);
		if (it_snap == m_snap_set.end())
		{
			return false;
		}
		content = std::dynamic_pointer_cast<Content>(it_snap->second);
		return true;
	}

	virtual void publish(
		IN eco::Content::ptr& it,
		IN const bool remove) override
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		m_new_set.push_back(Element(it, remove));
	}

protected:
	virtual void do_publish_snap(IN Subscriber& suber) override
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		for (auto it = m_snap_set.begin(); it != m_snap_set.end(); ++it)
		{
			suber.on_publish(m_id, it->second);
		}
	}

	virtual bool do_move(OUT std::vector<eco::Content::ptr>& new_set) override
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		if (m_new_set.empty())
		{
			return false;
		}

		new_set.reserve(m_new_set.size());
		for (auto it = m_new_set.begin(); it != m_new_set.end(); ++it)
		{
			SetContent* newc = static_cast<SetContent*>(it->get());
			if (newc->get_timestamp().is_removed())
			{
				auto it_snap = m_snap_set.find(it->get_id());
				if (it_snap != m_snap_set.end())
				{
					new_set.push_back(*it);
					m_snap_set.erase(it_snap);
				}
			}
			else if (newc->get_timestamp().is_updated())
			{
				auto it_snap = m_snap_set.find(it->get_id());
				if (it_snap != m_snap_set.end())
				{
					new_set.push_back(*it);
					it_snap->second = it;
				}
			}
			else if (newc->get_timestamp().is_removed())
			{
			}
		}
		m_new_set.clear();
		return new_set.size() > 0;
	}

	virtual void do_clear() override
	{
		m_snap_set.clear();
		eco::Mutex::ScopeLock lock(m_mutex);
		m_new_set.clear();
	}

protected:
	// object set snap.
	ContentMap m_snap_set;
	ContentMap m_repository;

	// new data that never sended to subscriber.
	struct Element
	{
		uint32_t m_remove;
		eco::Content::ptr m_content;
		Element(IN eco::Content::ptr& c, IN bool remove)
			: m_content(c), m_remove(remove)
		{}
	};
	std::vector<Element> m_new_set;
	mutable eco::Mutex m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif




