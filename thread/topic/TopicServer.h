#ifndef ECO_TOPIC_SERVER_H
#define ECO_TOPIC_SERVER_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/MemoryPool.h>
#include <eco/thread/topic/Topic.h>
#include <eco/thread/TaskServer.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class TopicServer
{
	ECO_OBJECT(TopicServer);
public:
	inline TopicServer()
	{}

	inline ~TopicServer()
	{
		stop();
	}

	// start topic server.
	inline void start()
	{
		m_publish_server.run();
	}

	// stop topic server.
	inline void stop()
	{
		m_publish_server.stop();
	}

	// join topic server.
	inline void join()
	{
		m_publish_server.join();
	}

public:
	// publish object to topic, create object if "make != nullptr". 
	template<typename object_t, typename topic_id_t>
	inline void publish(
		IN const topic_id_t& topic_id,
		IN const object_t& obj,
		IN Topic* (*make)(IN const topic_id_t&) = nullptr,
		IN bool remove_obj = false)
	{
		Topic::ptr topic = get_topic(topic_id, make);
		if (topic != nullptr)
		{
			publish_new(topic, obj, remove_obj);
		}
	}

	// publish object to topic, and create topic.
	template<typename topic_t, typename object_t, typename topic_id_t>
	inline void publish(
		IN const topic_id_t& topic_id,
		IN const object_t& obj,
		IN bool remove_obj = false)
	{
		Topic::ptr topic = get_topic(topic_id, topic_t::make);
		if (topic != nullptr)
		{
			publish_new(topic, obj, remove_obj);
		}
	}

	// publish object set to topic, and create topic.
	template<typename topic_t, typename object_set_t, typename topic_id_t>
	inline void publish_set(
		IN const topic_id_t& topic_id,
		IN const object_set_t& obj_set)
	{
		for (auto it = obj_set.begin(); it != obj_set.end(); ++it)
		{
			publish<topic_t>(topic_id, *it);
		}
	}

	// publish "object to be removed."
	template<typename set_topic_t, typename object_id_t, typename topic_id_t>
	inline void remove(
		IN const topic_id_t& topic_id,
		IN const object_id_t& obj_id)
	{
		Topic::ptr topic = find_topic(topic_id);
		if (topic != nullptr)
		{
			auto* set_topic = static_cast<set_topic_t*>(topic.get());
			Content::ptr content = set_topic->find(obj_id);
			content->timestamp() = eco::meta::v_remove;
			topic->append(content);
			m_publish_server.post(
				Publisher(topic, Publisher::mode_publish_new));
		}
	}

	// publish object to topic.
	template<typename object_t>
	inline void publish_new(
		IN Topic::ptr& topic,
		IN const object_t& obj,
		IN bool remove_obj = false)
	{
		auto ts = remove_obj ? eco::meta::v_remove : eco::meta::v_insert;
		Content::ptr content(new ContentT<object_t, object_t>(obj, ts));
		topic->append(content);
		m_publish_server.post(Publisher(topic, Publisher::mode_publish_new));
	}

	// publish shared object to topic.
	template<typename object_t>
	inline void publish_new(
		IN Topic::ptr& topic,
		IN const std::shared_ptr<object_t>& obj,
		IN bool remove_obj = false)
	{
		typedef std::shared_ptr<object_t> value_t;
		auto ts = remove_obj ? eco::meta::v_remove : eco::meta::v_insert;
		Content::ptr content(new ContentT<object_t, value_t>(obj, ts));
		topic->append(content);
		m_publish_server.post(Publisher(topic, Publisher::mode_publish_new));
	}

public:
	// subscribe topic.
	template<typename topic_id_t>
	inline bool subscribe(
		IN const topic_id_t& topic_id,
		IN Subscriber* subscriber,
		IN Topic* (*f)(IN const topic_id_t&) = nullptr)
	{
		Topic::ptr topic = get_topic(topic_id, f);
		if (topic != nullptr)
		{
			auto supscription = topic->reserve_subscribe(subscriber);
			if (!supscription.null())
			{
				m_publish_server.post(Publisher(topic, supscription));
				return true;
			}
		}
		return false;
	}

	template<typename topic_t, typename topic_id_t>
	inline bool subscribe(
		IN const topic_id_t& topic_id,
		IN Subscriber* subscriber)
	{
		return subscribe(topic_id, subscriber, topic_t::make);
	}

	// unsubscribe topic, and remove topic when there is no subscriber.
	template<typename topic_id_t>
	inline bool unsubscribe(
		IN const topic_id_t& topic_id,
		IN Subscriber* subscriber)
	{
		eco::Mutex::ScopeLock lock(m_topics_mutex);
		auto it = __get_topic_map(topic_id).find(topic_id);
		if (it != __get_topic_map(topic_id).end() &&
			it->second->unsubscribe(subscriber))
		{
			if (it->second->get_type() == OneTopic<topic_id_t>::type() &&
				!it->second->has_subscriber())
			{
				__get_topic_map(topic_id).erase(it);
			}
			return true;
		}
		return false;
	}

	template<typename topic_id_t>
	inline bool has_subscriber(
		IN const topic_id_t& topic_id,
		IN Subscriber* subscriber) const
	{
		Topic::ptr topic = find_topic(topic_id);
		return (topic == nullptr) ? false : topic->has_subscriber(subscriber);
	}

public:
	// create topic.
	template<typename topic_id_t>
	inline void create_topic(
		IN const topic_id_t& topic_id, 
		IN Topic* (*f)(IN const topic_id_t&))
	{
		eco::Mutex::ScopeLock lock(m_topics_mutex);
		auto it = __get_topic_map(topic_id).find(topic_id);
		if (it == __get_topic_map(topic_id).end())
		{
			__get_topic_map(topic_id)[topic_id].reset(f(topic_id));
		}
	}
	template<typename topic_t, typename topic_id_t>
	inline void create_topic(IN const topic_id_t& topic_id)
	{
		create_topic(topic_id, topic_t::make);
	}

	// get derived topic.
	template<typename topic_t, typename topic_id_t>
	inline std::shared_ptr<topic_t> cast_topic(IN const topic_id_t& topic_id)
	{
		Topic::ptr topic = get_topic(topic_id, topic_t::make);
		return std::dynamic_pointer_cast<topic_t>(topic);
	}

	template<typename topic_id_t>
	inline Topic::ptr get_topic(
		IN const topic_id_t& topic_id,
		IN Topic* (*f)(IN const topic_id_t&) = nullptr)
	{
		return __get_topic(topic_id, __get_topic_map(topic_id), f);
	}

	// find topic.
	template<typename topic_id_t>
	inline Topic::ptr find_topic(IN const topic_id_t& topic_id) const
	{
		eco::Mutex::ScopeLock lock(m_topics_mutex);
		auto it = __get_topic_map(topic_id).find(topic_id);
		return (it != __get_topic_map(topic_id).end()) 
			? it->second : Topic::ptr();
	}

	// find topic.
	template<typename topic_id_t>
	inline Topic::ptr pop_topic(IN const topic_id_t& topic_id)
	{
		Topic::ptr topic;
		eco::Mutex::ScopeLock lock(m_topics_mutex);
		auto it = __get_topic_map(topic_id).find(topic_id);
		if (it != __get_topic_map(topic_id).end())
		{
			topic = std::move(it->second);
			__get_topic_map(topic_id).erase(it);
		}
		return topic;
	}

	// remove topic.
	template<typename topic_id_t>
	inline void erase_topic(IN const topic_id_t& topic_id)
	{
		Topic::ptr t = pop_topic(topic_id);
		if (t != nullptr)
		{
			m_publish_server.post(Publisher(t, Publisher::mode_erase_topic));
		}
	}

	// find derived topic.
	template<typename topic_t, typename topic_id_t>
	inline std::shared_ptr<topic_t> find_topic(IN const topic_id_t& topic_id) const
	{
		Topic::ptr topic = find_topic(topic_id);
		return std::dynamic_pointer_cast<topic_t>(topic);
	}

	// find content in repository topic.
	template<typename set_topic_t, typename topic_id_t,
		typename object_t, typename object_id_t>
	inline bool find_content(
		OUT object_t& obj,
		IN  const topic_id_t& topic_id,
		IN  const object_id_t& object_id) const
	{
		Topic::ptr topic = find_topic(topic_id);
		if (topic != nullptr)
		{
			auto* set_topic = static_cast<set_topic_t*>(topic.get());
			return set_topic->find(obj, object_id);
		}
		return false;
	}

	// find content in repository topic.
	template<typename set_topic_t, typename topic_id_t, typename object_id_t>
	inline eco::Content::ptr find_content(
			IN  const topic_id_t& topic_id,
			IN  const object_id_t& object_id) const
	{
		Topic::ptr topic = find_topic(topic_id);
		if (topic != nullptr)
		{
			auto* set_topic = static_cast<set_topic_t*>(topic.get());
			return set_topic->find(object_id);
		}
		return nullptr;
	}

	// clear topic's content.
	template<typename topic_id_t>
	inline void clear_content(IN const topic_id_t& topic_id)
	{
		Topic::ptr topic = find_topic(topic_id);
		if (topic != nullptr)
		{
			m_publish_server.post(
				Publisher(topic, Publisher::mode_clear_content));
		}
	}

	// clear all topic
	inline void clear_topic()
	{
		__clear_topic(m_int_topics);
		__clear_topic(m_str_topics);
		__clear_topic(m_tid_topics);
	}

private:
	inline std::unordered_map<uint64_t, Topic::ptr>&
		__get_topic_map(const uint64_t)
	{
		return m_int_topics;
	}
	inline const std::unordered_map<uint64_t, Topic::ptr>&
		__get_topic_map(const uint64_t) const
	{
		return m_int_topics;
	}
	inline std::unordered_map<std::string, Topic::ptr>&
		__get_topic_map(const std::string&)
	{
		return m_str_topics;
	}
	inline const std::unordered_map<std::string, Topic::ptr>&
		__get_topic_map(const std::string&) const
	{
		return m_str_topics;
	}
	inline std::unordered_map<TopicId, Topic::ptr, TopicIdHash>&
		__get_topic_map(const TopicId&)
	{
		return m_tid_topics;
	}
	inline const std::unordered_map<TopicId, Topic::ptr, TopicIdHash>&
		__get_topic_map(const TopicId&) const
	{
		return m_tid_topics;
	}

	// get and create topic.
	template<typename topic_id_t, typename topic_map_t>
	inline Topic::ptr __get_topic(
		IN const topic_id_t& topic_id,
		IN topic_map_t& topic_map,
		IN Topic* (*f)(IN const topic_id_t&))
	{
		eco::Mutex::ScopeLock lock(m_topics_mutex);
		auto it = topic_map.find(topic_id);
		if (it != topic_map.end())
		{
			return it->second;
		}
		if (f != nullptr)
		{
			Topic::ptr topic(f(topic_id));
			return topic_map[topic_id] = topic;
		}
		return nullptr;
	}

	// clear all topic
	template<typename TopicMap>
	inline void __clear_topic(IN TopicMap& topic_map)
	{
		eco::Mutex::ScopeLock lock(m_topics_mutex);
		for (auto it = topic_map.begin(); it != topic_map.end(); ++it)
		{
			Publisher publish_task(
				std::move(it->second), Publisher::mode_erase_topic);
			m_publish_server.post(publish_task);
		}
		topic_map.clear();
	}

	// topic management.
	mutable eco::Mutex m_topics_mutex;
	std::unordered_map<uint64_t, Topic::ptr> m_int_topics;
	std::unordered_map<std::string, Topic::ptr> m_str_topics;
	std::unordered_map<TopicId, Topic::ptr, TopicIdHash> m_tid_topics;
	
	// publish topic message thread.
	eco::TaskServer<Publisher> m_publish_server;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif




