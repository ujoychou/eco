#ifndef ECO_TOPIC_SERVER_H
#define ECO_TOPIC_SERVER_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Any.h>
#include <eco/MemoryPool.h>
#include <eco/thread/topic/Topic.h>
#include <eco/thread/TaskServer.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename Impl>
class TopicServerT
{
	ECO_OBJECT(TopicServerT);
public:
	inline TopicServerT()
	{}

	inline ~TopicServerT()
	{
		m_impl.stop();
	}

	inline void start()
	{
		m_impl.start();
	}

	inline void stop()
	{
		m_impl.stop();
	}

	inline void join()
	{
		m_impl.join();
	}

	// publish object to topic, create object if "make != nullptr". 
	template<typename object_t, typename topic_id_t>
	inline void publish(
		IN const topic_id_t& topic_id,
		IN const object_t& obj,
		IN eco::meta::Stamp stamp = eco::meta::stamp_clean)
	{
		Topic::ptr topic = get_topic(topic_id);
		if (topic != nullptr)
		{
			publish_to(topic, obj, stamp);
		}
	}

	// publish object to topic, and create topic.
	template<typename topic_t, typename object_t, typename topic_id_t>
	inline void publish(
		IN const topic_id_t& topic_id,
		IN const object_t& obj,
		IN eco::meta::Stamp stamp = eco::meta::stamp_clean)
	{
		Topic::ptr topic = get_topic(topic_id, topic_t::make);
		if (topic != nullptr)
		{
			publish_to(topic, obj, stamp);
		}
	}

	// publish object set to topic, and create topic.
	template<typename topic_t, typename object_set_t, typename topic_id_t>
	inline void publish_set(
		IN const topic_id_t& topic_id,
		IN const object_set_t& obj_set,
		IN eco::meta::Stamp stamp = eco::meta::stamp_clean)
	{
		for (auto it = obj_set.begin(); it != obj_set.end(); ++it)
		{
			publish<topic_t>(topic_id, *it, stamp);
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
			ContentData::ptr newc = set_topic->find(obj_id);
			if (newc)
			{
				newc->stamp() = eco::meta::stamp_remove;
				m_impl.publish_new(topic, newc);
			}
		}
	}

public:
	// publish object to topic.
	template<typename object_t>
	inline void publish_to(
		IN Topic::ptr& topic,
		IN const object_t& obj,
		IN eco::meta::Stamp stamp = eco::meta::stamp_clean)
	{
		ContentData::ptr newc(new ContentDataT<object_t, object_t>(obj, stamp));
		m_impl.publish_new(topic, newc);
	}

	// publish shared object to topic.
	template<typename object_t>
	inline void publish_to(
		IN Topic::ptr& topic,
		IN const std::shared_ptr<object_t>& obj,
		IN eco::meta::Stamp stamp = eco::meta::stamp_clean)
	{
		typedef std::shared_ptr<object_t> value_t;
		ContentData::ptr newc(new ContentDataT<object_t, value_t>(obj, stamp));
		m_impl.publish_new(topic, newc);
	}

public:
	// subscribe topic.
	template<typename topic_id_t>
	inline bool subscribe(
		IN const topic_id_t& topic_id,
		IN Subscriber* subscriber,
		IN TopicEvent::ptr& event = TopicEvent::ptr(),
		IN eco::MakeTopic make = nullptr)
	{
		Topic::ptr topic = get_topic(topic_id, make, event);
		if (topic != nullptr)
		{
			auto supscription = topic->reserve_subscribe(subscriber);
			if (!supscription.null())
			{
				m_impl.publish_snap(topic, supscription, event);
				return true;
			}
		}
		return false;
	}

	template<typename topic_t, typename topic_id_t>
	inline bool subscribe(
		IN const topic_id_t& topic_id,
		IN Subscriber* subscriber,
		IN TopicEvent::ptr& event = TopicEvent::ptr())
	{
		return subscribe(topic_id, subscriber, event, topic_t::make);
	}

	// unsubscribe topic, and remove topic when there is no subscriber.
	template<typename topic_id_t>
	inline int unsubscribe(
		IN const topic_id_t& topic_id,
		IN Subscriber* subscriber,
		IN TopicEvent::ptr& event = TopicEvent::ptr())
	{
		eco::Mutex::ScopeLock lock(m_topics_mutex);
		auto& topic_map = __get_topic_map(topic_id);
		auto it = topic_map.find(topic_id);
		if (it != topic_map.end() && it->second->unsubscribe(subscriber))
		{
			if (!it->second->has_subscriber())
			{
				if (event.get() != nullptr)
				{
					it->second->on_erase(event.get());
					topic_map.erase(it);
					return 2;
				}
			}
			return 1;
		}
		return 0;
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
	template<typename topic_t, typename topic_id_t>
	inline void create_topic(IN const topic_id_t& topic_id)
	{
		eco::Mutex::ScopeLock lock(m_topics_mutex);
		auto it = __get_topic_map(topic_id).find(topic_id);
		if (it == __get_topic_map(topic_id).end())
		{
			__get_topic_map(topic_id)[topic_id].reset(topic_t::make(topic_id));
		}
	}

	// get derived topic.
	template<typename topic_t, typename topic_id_t>
	inline std::shared_ptr<topic_t> cast_topic(
		IN const topic_id_t& topic_id,
		IN TopicEvent::ptr& event = TopicEvent::ptr())
	{
		Topic::ptr topic = get_topic(topic_id, topic_t::make, event);
		return std::dynamic_pointer_cast<topic_t>(topic);
	}

	template<typename topic_id_t>
	inline Topic::ptr get_topic(
		IN const topic_id_t& topic_id,
		IN eco::MakeTopic make = nullptr,
		IN TopicEvent::ptr& event = TopicEvent::ptr())
	{
		return __get_topic(topic_id, __get_topic_map(topic_id), make, event);
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
		Topic::ptr topic = pop_topic(topic_id);
		if (topic != nullptr)
		{
			m_impl.publish_erase(topic);
		}
	}

	// find derived topic.
	template<typename topic_t, typename topic_id_t>
	inline std::shared_ptr<topic_t> find_topic(IN const topic_id_t& topic_id) const
	{
		Topic::ptr topic = find_topic(topic_id);
		return std::dynamic_pointer_cast<topic_t>(topic);
	}

	// clear all topic
	inline void clear_topic()
	{
		__clear_topic(m_int_topics);
		__clear_topic(m_str_topics);
		__clear_topic(m_tid_topics);
	}


public:
	// get seq topic last content.
	template<typename seq_topic_t, typename object_t, typename topic_id_t>
	inline bool get_last_content(
		OUT object_t& obj,
		IN  const topic_id_t& tid) const
	{
		Topic::ptr topic = find_topic(tid);
		if (topic != nullptr)
		{
			auto c = static_cast<seq_topic_t*>(topic.get())->last();
			if (c != nullptr)
			{
				obj = *(object_t*)c->get_value();
				return true;
			}
		}
		return false;
	}

	// find content in repository topic.
	template<typename set_topic_t, typename topic_id_t,
		typename object_t, typename object_id_t>
		inline bool find_content(
			OUT object_t& obj,
			IN  const topic_id_t& topic_id,
			IN  const object_id_t& object_id) const
	{
		auto content = find_content<set_topic_t>(topic_id, object_id);
		if (content)
		{
			obj = *(object_t*)content->get_value();
			return true;
		}
		return false;
	}

	// find content in repository topic.
	template<typename set_topic_t, typename topic_id_t, typename object_id_t>
	inline eco::ContentData::ptr find_content(
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
			m_impl.publish_clear(topic);
		}
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
	inline std::unordered_map<TopicId, Topic::ptr, TopicId::Hash>&
		__get_topic_map(const TopicId&)
	{
		return m_tid_topics;
	}
	inline const std::unordered_map<TopicId, Topic::ptr, TopicId::Hash>&
		__get_topic_map(const TopicId&) const
	{
		return m_tid_topics;
	}

	// get and create topic.
	template<typename topic_id_t, typename topic_map_t>
	inline Topic::ptr __get_topic(
		IN const topic_id_t& topic_id,
		IN topic_map_t& topic_map,
		IN eco::MakeTopic make,
		IN TopicEvent::ptr& event = TopicEvent::ptr())
	{
		{
			eco::Mutex::ScopeLock lock(m_topics_mutex);
			auto it = topic_map.find(topic_id);
			if (it != topic_map.end())
			{
				return it->second;
			}
		}

		if (make != nullptr)
		{
			Topic::ptr topic(make(&topic_id));
			topic->on_init(event.get());
			eco::Mutex::ScopeLock lock(m_topics_mutex);
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
			m_impl.publish_erase(it->second);
		}
		topic_map.clear();
	}

private:
	// topic management.
	mutable eco::Mutex m_topics_mutex;
	Impl m_impl;
	std::unordered_map<uint64_t, Topic::ptr> m_int_topics;
	std::unordered_map<std::string, Topic::ptr> m_str_topics;
	std::unordered_map<TopicId, Topic::ptr, TopicId::Hash> m_tid_topics;
};


////////////////////////////////////////////////////////////////////////////////
class QueueImpl
{
public:
	inline void start()
	{}

	inline void stop()
	{}

	inline void join()
	{}

	inline void publish_new(
		IN eco::Topic::ptr& topic,
		IN eco::ContentData::ptr& newc)
	{
		topic->publish_new(newc);
	}

	inline void publish_snap(
		IN eco::Topic::ptr& topic,
		IN eco::AutoRefPtr<eco::Subscription>& sub,
		IN TopicEvent::ptr& event)
	{
		topic->publish_snap(*sub, event.get());
	}

	inline void publish_erase(
		IN eco::Topic::ptr& topic)
	{
		topic->publish_erase();
	}

	inline void publish_clear(
		IN eco::Topic::ptr& topic)
	{
		topic->publish_clear();
	}
};
typedef TopicServerT<QueueImpl> TopicQueue;


////////////////////////////////////////////////////////////////////////////////
class ServerImpl
{
public:
	inline void start()
	{
		m_publish_server.run();
	}

	inline void stop()
	{
		m_publish_server.stop();
	}

	inline void join()
	{
		m_publish_server.join();
	}

	inline void publish_new(
		IN eco::Topic::ptr& topic,
		IN eco::ContentData::ptr& newc)
	{
		m_publish_server.post(Publisher(topic, newc));
	}

	inline void publish_snap(
		IN eco::Topic::ptr& topic,
		IN eco::AutoRefPtr<eco::Subscription>& sub,
		IN TopicEvent::ptr& event)
	{
		m_publish_server.post(Publisher(topic, sub, event));
	}

	inline void publish_erase(
		IN eco::Topic::ptr& topic)
	{
		m_publish_server.post(Publisher(topic, Publisher::mode_publish_erase));
	}

	inline void publish_clear(
		IN eco::Topic::ptr& topic)
	{
		m_publish_server.post(Publisher(topic, Publisher::mode_publish_clear));
	}

private:
	// publish topic message thread.
	eco::TaskServer<Publisher> m_publish_server;
};
typedef TopicServerT<ServerImpl> TopicServer;


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif