#ifndef ECO_TOPIC_ROLE_H
#define ECO_TOPIC_ROLE_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Cast.h>
#include <eco/meta/Stamp.h>
#include <eco/thread/topic/Subscription.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
// define customer topic.
#define ECO_TOPIC(topic_t) \
ECO_OBJECT(topic_t) \
ECO_TYPE(topic_t) \
public:\
	inline topic_t() {} \
	inline static eco::Topic* make(IN const TopicId& id)\
	{\
		eco::PopTopic<TopicId>* topic = new topic_t();\
		topic->set_id(id);\
		return topic;\
	}

// define basic topic supported by topic framework.
#define ECO_TOPIC_TOPIC(topic_t) \
ECO_TOPIC(topic_t) \
public:\
	inline static const char* topic_type()\
	{\
		return #topic_t;\
	}\
	virtual const char* get_topic_type() const\
	{\
		return topic_type();\
	}

//##############################################################################
//##############################################################################
class TopicId
{
private:
	uint32_t m_type;
	uint64_t m_value;

public:
	inline TopicId(
		IN const uint16_t type  = 0,
		IN const uint16_t prop	= 0,
		IN const uint64_t value = 0)
		: m_type((type << 16) + prop)
		, m_value(value)
	{}

	inline void set(
		IN const uint16_t type  = 0,
		IN const uint16_t prop  = 0,
		IN const uint64_t value = 0)
	{
		m_type = (type << 16) + prop;
		m_value = value;
	}

public:
	inline uint16_t get_type() const
	{
		return uint16_t(m_type >> 16);
	}

	inline uint16_t get_prop() const
	{
		return uint16_t(m_type & 0xFFFF);
	}

	inline uint64_t get_value() const
	{
		return m_value;
	}

	inline TopicId& type(IN const uint16_t v)
	{
		m_type = (v << 16) + (m_type & 0xFFFF);
		return *this;
	}

	inline TopicId& prop(IN const uint16_t v)
	{
		m_type = (m_type & 0xFFFF0000) + v;
		return *this;
	}

	inline TopicId& value(IN const uint64_t v)
	{
		m_value = v;
		return *this;
	}

public:
	inline std::size_t hash_value() const
	{
		uint64_t seed = m_type;
		eco::hash_combine<uint64_t>(seed, m_value << 16);
		return static_cast<std::size_t>(seed);
	}

	inline bool operator==(IN const TopicId& tid) const
	{
		return m_value == tid.m_value && m_type == tid.m_type;
	}

	inline bool equal(
		IN const uint32_t type,
		IN const uint32_t prop,
		IN const uint64_t value = 0) const
	{
		return *this == TopicId(type, prop, value);
	}
};
////////////////////////////////////////////////////////////////////////////////
class TopicIdHash
{
public:
	inline std::size_t operator()(IN const TopicId& v) const
	{
		return v.hash_value();
	}
};


////////////////////////////////////////////////////////////////////////////////
class Content
{
	ECO_OBJECT(Content);
public:
	inline Content(IN const eco::meta::Stamp v)	: m_stamp(v)
	{}

	// destructor.
	virtual ~Content() = 0 
	{}

	// set topic content object_t.
	virtual void* get_set_topic_object() = 0;

	// set topic content value_t. 
	virtual void* get_value() = 0;

	// content type.
	virtual const uint32_t get_type_id() const = 0;

	// get content object.
	template<typename value_t>
	inline value_t& cast()
	{
		return *static_cast<value_t*>(get_value());
	}
	template<typename value_t>
	inline value_t* cast_ptr()
	{
		return get_type_id() == eco::TypeId<value_t>::value
			? static_cast<value_t*>(get_value()) : nullptr;
	}

	// content stamp.
	inline eco::meta::Stamp& stamp()
	{
		return m_stamp;
	}
	inline const eco::meta::Stamp get_stamp() const
	{
		return m_stamp;
	}

private:
	eco::meta::Stamp m_stamp;
};

////////////////////////////////////////////////////////////////////////////////
template<typename Object, typename Value>
class ContentT : public eco::Content
{
public:
	inline ContentT(IN const Value& v, IN eco::meta::Stamp ts)
		: m_value((Value&)v), eco::Content(ts)
	{}

	virtual ~ContentT() override
	{}

	virtual const uint32_t get_type_id() const override
	{
		return eco::TypeId<Value>::value;
	}

	virtual void* get_value() override
	{
		return &m_value;
	}

	virtual void* get_set_topic_object() override
	{
		return get_object(m_value);
	}

private:
	inline Object* get_object(IN Object* obj)
	{
		return obj;
	}
	inline const Object* get_object(IN const Object* obj)
	{
		return obj;
	}
	inline Object* get_object(IN Object& obj)
	{
		return &obj;
	}
	inline const Object* get_object(IN const Object& obj) const
	{
		return &obj;
	}
	inline Object* get_object(IN std::shared_ptr<Object>& obj)
	{
		return obj.get();
	}
	inline const Object* get_object(IN const std::shared_ptr<Object>& obj) const
	{
		return obj.get();
	}
	Value m_value;
};


////////////////////////////////////////////////////////////////////////////////
class Subscriber : public eco::detail::Subscriber
{
	ECO_OBJECT(Subscriber);
public:
	inline Subscriber() {};
	virtual ~Subscriber() {};

public:
	// publish content: new/remove content.
	virtual void on_publish(
		IN const eco::TopicId& topic_id,
		IN eco::Content::ptr& content,
		IN const eco::ContentType type)
	{}

	// clear topic content
	virtual void on_clear(
		IN const eco::TopicId& topic_id)
	{}

	// erase topic and clear content.
	virtual void on_erase(
		IN const eco::TopicId& topic_id)
	{}

public:
	virtual void on_publish(
		IN const std::string& topic_id,
		IN eco::Content::ptr& content,
		IN const eco::ContentType type)
	{}

	// clear topic content
	virtual void on_clear(IN const std::string& topic_id)
	{}

	// erase topic and clear content.
	virtual void on_erase(IN const std::string& topic_id)
	{}

public:
	virtual void on_publish(
		IN const uint64_t topic_id,
		IN eco::Content::ptr& content,
		IN const eco::ContentType type)
	{}

	// clear topic content
	virtual void on_clear(IN const uint64_t topic_id)
	{}

	// erase topic and clear content.
	virtual void on_erase(IN const uint64_t topic_id)
	{}
};


////////////////////////////////////////////////////////////////////////////////
class Topic : public detail::Topic
{
	ECO_OBJECT(Topic);
public:
	inline Topic() {}
	virtual ~Topic() {}

	// init topic event.
	virtual void on_init(IN void* context, IN void* data) {};

	// erase topic event.
	virtual void on_erase(IN void* context, IN void* data) {};

public:
	// topic type.
	virtual const char* get_type() const = 0;
	virtual const char* get_topic_type() const = 0;

	// topic server: publish snap after subsriber reserve topic.
	virtual void publish_snap(IN Subscription& subscription) = 0;

	// topic server: publish new content after append_new()(recv) new content.
	virtual void publish_new(IN Content::ptr& new_c) = 0;

	// publish erase topic event.
	virtual void publish_erase_topic() = 0;

	// publish clear topic content.
	virtual void publish_clear_content() = 0;
};


////////////////////////////////////////////////////////////////////////////////
// note that PopTopic is base topic of "One/Seq/Set" topic.
template<typename TopicId = eco::TopicId>
class PopTopic : public Topic
{
	ECO_TOPIC_TOPIC(PopTopic);
protected:
	// publish snap to subscriber.
	virtual void do_snap(IN Subscription& subscription) {}

	// move new content to snap content.
	virtual void do_move(OUT Content::ptr& new_c) {}

	// clear all content
	virtual void do_clear() {}

public:
	// topic identity.
	typedef TopicId TopicId;
	inline void set_id(IN const TopicId& id)
	{
		m_id = id;
	}
	inline const TopicId& get_id() const
	{
		return m_id;
	}

	// add "object set/object/shared_object" to topic by "load".
	template<typename object_set_t>
	inline void append_set(IN const object_set_t& set)
	{
		for (auto it = set.begin(); it != set.end(); ++it)
		{
			append(*it);
		}
	}
	template<typename object_t>
	inline void append(IN const object_t& obj)
	{
		Content::ptr newc(new ContentT<
			object_t, object_t>(obj, eco::meta::stamp_insert));
		do_move(newc);
	}
	template<typename object_t>
	inline void append(IN const std::shared_ptr<object_t>& obj)
	{
		typedef std::shared_ptr<object_t> value_t;
		Content::ptr newc(new ContentT<
			object_t, value_t>(obj, eco::meta::stamp_insert));
		do_move(newc);
	}

public:
	// topic server: publish snap after subsriber reserve topic.
	virtual void publish_snap(IN Subscription& node) override
	{
		eco::Mutex::ScopeLock lock(mutex());
		if (node.m_subscriber != nullptr)
		{
			node.subscribe_submit();
			do_snap(node);
		}
	}

	// topic server: publish new content after append_new()(recv) new content.
	virtual void publish_new(IN Content::ptr& new_c) override
	{
		// publish new content to all subscriber.
		do_move(new_c);
		eco::Mutex::ScopeLock lock(mutex());
		Subscription* node = subscriber_head();
		while (!subscriber_end(node))
		{
			Subscription* next = node->m_topic_subscriber_next;
			if (node->m_working)
			{
				auto* suber = (Subscriber*)node->m_subscriber;
				suber->on_publish(m_id, new_c, content_new);
			}
			node = next;
		}
		eco::meta::clean(new_c->stamp());
	}

	// publish remove topic event.
	virtual void publish_erase_topic() override
	{
		eco::Mutex::ScopeLock lock(mutex());
		Subscription* node = subscriber_head();
		while (!subscriber_end(node))
		{
			Subscription* next = node->m_topic_subscriber_next;
			if (node->m_working)
			{
				Subscriber* suber = (Subscriber*)(node->m_subscriber);
				suber->on_erase(m_id);
			}
			node = next;
		}

		detail::Topic::clear();
	}

	// publish clear all content in topic.
	virtual void publish_clear_content() override
	{
		do_clear();
		eco::Mutex::ScopeLock lock(mutex());
		Subscription* node = subscriber_head();
		while (!subscriber_end(node))
		{
			// avoid getting next suber fail when node erased by "on_clear";
			Subscription* next = node->m_topic_subscriber_next;
			if (node->m_working)
			{
				Subscriber* suber = (Subscriber*)(node->m_subscriber);
				suber->on_clear(m_id);
			}
			node = next;
		}// end while.
	}

protected:
	TopicId m_id;
};


////////////////////////////////////////////////////////////////////////////////
class Publisher
{
	ECO_NONCOPYABLE(Publisher);
public:
	// publish mode.
	enum
	{
		mode_publish_snap,				// when subscribe a topic.
		mode_publish_new,				// when publish a content.
		mode_erase_topic,				// when erase a topic.
		mode_clear_content,				// when clear all content of topic.
	};
	typedef uint32_t Mode;

	inline Publisher() : m_publish_mode(-1)
	{}

	// publish new content.
	inline Publisher(
		IN Topic::ptr& topic,
		IN Content::ptr& new_content)
		: m_topic(std::move(topic))
		, m_new_content(std::move(new_content))
		, m_publish_mode(mode_publish_new)
	{}

	// publish some mode: erase_topic & clear content.
	inline Publisher(
		IN Topic::ptr& topic, 
		IN const Mode publish_mode)
		: m_topic(std::move(topic))
		, m_publish_mode(publish_mode)
	{}

	// subscriber subscribe topic.
	inline Publisher(
		IN Topic::ptr& topic,
		IN AutoRefPtr<Subscription>& subscription)
		: m_topic(std::move(topic))
		, m_subscription(std::move(subscription))
		, m_publish_mode(mode_publish_snap)
	{}

	inline Publisher(Publisher&& pub)
		: m_topic(std::move(pub.m_topic))
		, m_new_content(std::move(pub.m_new_content))
		, m_subscription(std::move(pub.m_subscription))
		, m_publish_mode(pub.m_publish_mode)
	{}

	inline Publisher& operator=(Publisher&& pub)
	{
		m_topic = std::move(pub.m_topic);
		m_new_content = std::move(pub.m_new_content);
		m_subscription = std::move(pub.m_subscription);
		m_publish_mode = pub.m_publish_mode;
		return *this;
	}

	inline void operator()(void)
	{
		switch (m_publish_mode)
		{
		case mode_publish_new:
			m_topic->publish_new(m_new_content);
			break;
		case mode_publish_snap:
			m_topic->publish_snap(*m_subscription);
			break;
		case mode_erase_topic:
			m_topic->publish_erase_topic();
			break;
		case mode_clear_content:
			m_topic->publish_clear_content();
			break;
		}
	}

private:
	Topic::ptr m_topic;
	Content::ptr m_new_content;
	AutoRefPtr<Subscription> m_subscription;
	uint32_t m_publish_mode;
};


////////////////////////////////////////////////////////////////////////////////
}// ns::front
#endif