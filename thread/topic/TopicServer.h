#ifndef ECO_TOPIC_SERVER_H
#define ECO_TOPIC_SERVER_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Any.h>
#include <eco/MemoryPool.h>
#include <eco/thread/topic/Topic.h>
#include <eco/thread/MessageServer.h>


ECO_NS_BEGIN(eco);
class TopicInner { public: inline TopicUid& id(Topic& t) { return t.m_id;}};
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

	inline void start(const char* name)
	{
		m_impl.start(name);
	}

	inline void stop()
	{
		m_impl.stop();
	}

	inline void join()
	{
		m_impl.join();
	}

	inline void set_capacity(uint32_t size)
	{
		m_impl.set_capacity(size);
	}

	inline uint32_t capacity() const
	{
		return m_impl.capacity();
	}

	inline uint32_t size() const
	{
		return m_impl.size();
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
		Topic::ptr topic = get_topic(topic_id, topic_t::create);
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
			assert(topic->get_type_name() == ECO_TYPE_NAME(SetTopic));
			if (topic->get_type_name() == ECO_TYPE_NAME(SetTopic))
				publish_to(topic, obj_id, eco::meta::stamp_delete);
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
	inline bool subscribe(
		IN Topic::ptr& topic,
		IN Subscriber* subscriber,
		IN TopicEvent::ptr event = nullptr)
	{
		auto supscription = topic->reserve_subscribe(subscriber);
		if (!supscription.null())
		{
			m_impl.publish_snap(topic, supscription, event);
			return true;
		}
		return false;
	}

	// subscribe topic.
	template<typename topic_id_t>
	inline bool subscribe(
		IN const topic_id_t& topic_id,
		IN Subscriber* subscriber,
		IN eco::CreateTopic create = nullptr,
		IN TopicEvent::ptr event = nullptr)
	{
		Topic::ptr topic = get_topic(topic_id, create, event);
		if (topic != nullptr)
		{
			return subscribe(topic, subscriber, event);
		}
		return false;
	}

	// subscribe topic id.
	template<typename topic_t, typename topic_id_t>
	inline bool subscribe(
		IN const topic_id_t& topic_id,
		IN Subscriber* subscriber,
		IN TopicEvent::ptr event = nullptr)
	{
		return subscribe(topic_id, subscriber, topic_t::create, event);
	}

	// unsubscribe topic, and remove topic when there is no subscriber.
	template<typename topic_id_t>
	inline int unsubscribe(
		IN const topic_id_t& topic_id,
		IN Subscriber* subscriber,
		IN TopicEvent::ptr event = nullptr)
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
				}
				return 2;
			}
			return 1;
		}
		return 0;
	}

	// whether the topic has subscriber.
	template<typename topic_id_t>
	inline bool has_subscriber(
		IN const topic_id_t& topic_id,
		IN Subscriber* subscriber) const
	{
		Topic::ptr topic = find_topic(topic_id);
		return (topic == nullptr) ? false : topic->has_subscriber(subscriber);
	}

	// whether the topic has subscriber.
	template<typename topic_id_t>
	inline bool has_subscriber(IN const topic_id_t& topic_id) const
	{
		Topic::ptr topic = find_topic(topic_id);
		return (topic == nullptr) ? false : topic->has_subscriber();
	}

public:
	// get and create derived topic.
	template<typename topic_t, typename topic_id_t>
	inline typename topic_t::ptr get_topic(
		IN const topic_id_t& topic_id,
		IN TopicEvent::ptr event = nullptr)
	{
		auto topic = get_topic(topic_id, topic_t::create, event);
		return std::dynamic_pointer_cast<topic_t>(topic);
	}

	// get and create topic.
	template<typename topic_id_t>
	inline Topic::ptr get_topic(
		IN const topic_id_t& topic_id,
		IN CreateTopic create = nullptr,
		IN TopicEvent::ptr event = nullptr)
	{
		eco::Mutex::ScopeLock lock(m_topics_mutex);
		auto& topic_map = __get_topic_map(topic_id);
		auto it = topic_map.find(topic_id);
		if (it != topic_map.end())
		{
			return it->second;
		}

		if (create != nullptr)
		{
			Topic::ptr topic = std::dynamic_pointer_cast<Topic>(create());
			TopicInner().id(*topic).set(topic_id, this);
			topic->on_init(event.get());
			return topic_map[topic_id] = topic;
		}
		return nullptr;
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
	// find derived topic.
	template<typename topic_t, typename topic_id_t>
	inline typename topic_t::ptr find_topic(IN const topic_id_t& topic_id) const
	{
		return std::dynamic_pointer_cast<topic_t>(find_topic(topic_id));
	}

	// pop topic.
	template<typename topic_id_t>
	inline Topic::ptr pop_topic(IN const topic_id_t& topic_id)
	{
		Topic::ptr topic;
		eco::Mutex::ScopeLock lock(m_topics_mutex);
		auto& topic_map = __get_topic_map(topic_id);
		auto it = topic_map.find(topic_id);
		if (it != topic_map.end())
		{
			topic = std::move(it->second);
			topic_map.erase(it);
		}
		return topic;
	}
	// pop derived topic.
	template<typename topic_t, typename topic_id_t>
	inline typename topic_t::ptr pop_topic(
		IN const topic_id_t& topic_id) const
	{
		return std::dynamic_pointer_cast<topic_t>(pop_topic(topic_id));
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
	inline void start(const char* name)
	{}

	inline void stop()
	{}

	inline void join()
	{}

	inline void set_capacity(uint32_t capacity)
	{}

	inline uint32_t capacity() const
	{
		return 0;
	}

	inline uint32_t size() const
	{
		return 0;
	}

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
	inline void start(const char* name)
	{
		m_publish_server.run(name, 1);
	}

	inline void stop()
	{
		m_publish_server.stop();
	}

	inline void join()
	{
		m_publish_server.join();
	}

	inline void set_capacity(uint32_t capacity)
	{
		m_publish_server.set_capacity(capacity);
	}

	inline uint32_t capacity() const
	{
		return m_publish_server.get_queue().capacity();
	}

	inline uint32_t size() const
	{
		return m_publish_server.get_queue().size();
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
	eco::MessageServer<Publisher, PublisherHandler> m_publish_server;
};
typedef TopicServerT<ServerImpl> TopicServer;


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif