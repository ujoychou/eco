#ifndef ECO_TOPIC_ROLE_H
#define ECO_TOPIC_ROLE_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Any.h>
#include <eco/Object.h>
#include <eco/log/Log.h>
#include <eco/meta/Stamp.h>
#include <eco/thread/topic/Subscription.h>


ECO_NS_BEGIN(eco);
class Topic;
class TopicServer;
////////////////////////////////////////////////////////////////////////////////
// define customer topic.
#define ECO_TOPIC(topic_t) \
ECO_OBJECT(topic_t) \
ECO_TYPE(topic_t) \
public:\
	inline topic_t() {} \
	inline static eco::Topic* make(const TopicId& id)\
	{\
		eco::PopTopic<TopicId>* topic = new topic_t();\
		topic->set_id(id);\
		return topic;\
	}

// define basic topic supported by topic framework.
#define ECO_TOPIC_TOPIC(topic_t) \
ECO_TOPIC(topic_t) \
public:\
	typedef topic_t super;\
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
	inline explicit TopicId(
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

public:
	// for std::unorder_map.
	struct Hash
	{
		inline std::size_t operator()(IN const TopicId& v) const
		{
			return v.hash_value();
		}
	};

	// for std::map.
	inline bool operator<(IN const TopicId& tid) const
	{
		return (m_type != tid.m_type) 
			? m_type < tid.m_type : m_value < tid.m_value;
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
class TopicEvent
{
public:
	typedef std::shared_ptr<TopicEvent> ptr;

	// get snap seq size to publish snap.
	virtual uint32_t get_snap_seq(IN eco::Topic& topic)
	{
		return 0;	// publish all snap.
	}
};


////////////////////////////////////////////////////////////////////////////////
class ContentWrap
{
public:
	inline ContentWrap(
		IN eco::Content::ptr& content,
		IN eco::ContentType type,
		IN eco::TopicEvent* event = nullptr)
		: m_event(event), m_type(type), m_content(&content)
	{}

	// content type.
	inline const uint32_t get_type_id() const
	{
		return (**m_content).get_type_id();
	}

	// data context.
	inline bool has_event() const
	{
		return m_event != nullptr;
	}
	inline eco::TopicEvent* event()
	{
		return m_event;
	}

	// cast content object.
	template<typename value_t>
	inline value_t& cast()
	{
		return *static_cast<value_t*>((**m_content).get_value());
	}
	template<typename value_t>
	inline value_t* cast_ptr()
	{
		return type_id() == eco::TypeId<value_t>::value
			? static_cast<value_t*>((**m_content).get_value()) : nullptr;
	}

	// content data.
	inline eco::Content::ptr& data()
	{
		return *m_content;
	}
	inline const eco::ContentType snap() const
	{
		return m_type;
	}

	// content stamp.
	inline eco::meta::Stamp& stamp()
	{
		return (**m_content).stamp();
	}
	inline const eco::meta::Stamp get_stamp() const
	{
		return (**m_content).get_stamp();
	}

private:
	eco::TopicEvent* m_event;
	eco::ContentType m_type;
	eco::Content::ptr* m_content;
};


////////////////////////////////////////////////////////////////////////////////
class Subscriber : public eco::detail::Subscriber
{
	ECO_OBJECT(Subscriber);
public:
	inline Subscriber() {};
	virtual ~Subscriber() {};

	// publish content: new/remove content.
	virtual void on_publish(IN eco::Topic& topic, IN eco::ContentWrap& wrap)
	{}

	// clear topic content
	virtual void on_clear(IN eco::Topic& opic)
	{}

	// erase topic and clear content.
	virtual void on_erase(IN eco::Topic& topic)
	{}
};


////////////////////////////////////////////////////////////////////////////////
class Topic : public detail::Topic
{
	ECO_OBJECT(Topic);
public:
	// topic type.
	virtual const char* get_topic_type() const = 0;

	// topic type name.
	virtual const char* get_type() const = 0;

	// object type id.
	virtual const uint32_t get_type_id() const = 0;

	// get topic id.
	virtual bool get_id(OUT const uint64_t*& id) const	{ return false; }
	virtual bool get_id(OUT const std::string*& id) const { return false; }
	virtual bool get_id(OUT const eco::TopicId*& id) const { return false; }

	// init topic event.
	virtual void on_init(IN eco::TopicEvent* event) {}

	// erase topic event.
	virtual void on_erase(IN eco::TopicEvent* event) {}

protected:
	// publish snap to subscriber.
	virtual void do_snap(
		IN eco::Subscription& node,
		IN eco::TopicEvent* evt) {}

	// move new content to snap content.
	virtual bool do_move(
		OUT eco::Content::ptr& new_c) 
	{
		return true;
	}

	// clear all content
	virtual void do_clear() {}

public:
	inline Topic() {}
	virtual ~Topic() {}

	// add "object set/object/shared_object" to topic by "load".
	template<typename object_set_t>
	inline void push_back_set(IN const object_set_t& set)
	{
		eco::Mutex::ScopeLock lock(mutex());
		for (auto it = set.begin(); it != set.end(); ++it)
		{
			push_back_raw(*it);
		}
	}
	template<typename object_t>
	inline void push_back(IN const object_t& obj)
	{
		eco::Mutex::ScopeLock lock(mutex());
		push_back_raw(obj);
	}
	template<typename object_t>
	inline void push_back(IN const std::shared_ptr<object_t>& obj)
	{
		eco::Mutex::ScopeLock lock(mutex());
		push_back_raw(obj);
	}

	// topic server: publish snap after subsriber reserve topic.
	virtual void publish_snap(IN Subscription& node, IN eco::TopicEvent* evt)
	{
		eco::Mutex::ScopeLock lock(mutex());
		if (node.m_subscriber != nullptr)
		{
			node.subscribe_submit();
			do_snap(node, evt);
		}
	}

	// topic server: publish new content after append_new()(recv) new content.
	virtual void publish_new(IN eco::Content::ptr& new_c)
	{
		// publish new content to all subscriber.
		eco::Mutex::ScopeLock lock(mutex());
		if (!do_move(new_c)) return;
		Subscription* node = subscriber_head();
		while (!subscriber_end(node))
		{
			Subscription* next = node->m_topic_subscriber_next;
			if (node->m_working)
			{
				auto* suber = (Subscriber*)node->m_subscriber;
				suber->on_publish(*this, ContentWrap(new_c, content_new));
			}
			node = next;
		}
		eco::meta::clean(new_c->stamp());
	}

	// publish remove topic event.
	virtual void publish_erase_topic()
	{
		eco::Mutex::ScopeLock lock(mutex());
		Subscription* node = subscriber_head();
		while (!subscriber_end(node))
		{
			Subscription* next = node->m_topic_subscriber_next;
			if (node->m_working)
			{
				Subscriber* suber = (Subscriber*)(node->m_subscriber);
				suber->on_erase(*this);
			}
			node = next;
		}

		detail::Topic::clear();
	}

	// publish clear all content in topic.
	virtual void publish_clear_content()
	{
		eco::Mutex::ScopeLock lock(mutex());
		do_clear();
		Subscription* node = subscriber_head();
		while (!subscriber_end(node))
		{
			// avoid getting next suber fail when node erased by "on_clear";
			Subscription* next = node->m_topic_subscriber_next;
			if (node->m_working)
			{
				Subscriber* suber = (Subscriber*)(node->m_subscriber);
				suber->on_clear(*this);
			}
			node = next;
		}// end while.
	}

protected:
	template<typename object_t>
	inline void push_back_raw(IN const object_t& obj)
	{
		Content::ptr newc(new ContentT<
			object_t, object_t>(obj, eco::meta::stamp_insert));
		do_move(newc);
	}
	template<typename object_t>
	inline void push_back_raw(IN const std::shared_ptr<object_t>& obj)
	{
		typedef std::shared_ptr<object_t> value_t;
		Content::ptr newc(new ContentT<
			object_t, value_t>(obj, eco::meta::stamp_insert));
		do_move(newc);
	}
};


////////////////////////////////////////////////////////////////////////////////
// note that PopTopic is base topic of "One/Seq/Set" topic.
template<typename TopicId = eco::TopicId>
class PopTopic : public Topic
{
	ECO_TOPIC_TOPIC(PopTopic);
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
	virtual bool get_id(OUT const TopicId*& id) const override
	{
		id = &m_id;
		return true;
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
		mode_publish_snap		= 1,	// when subscribe a topic.
		mode_publish_new		= 2,	// when publish a content.
		mode_erase_topic		= 3,	// when erase a topic.
		mode_clear_content		= 4,	// when clear all content of topic.
	};
	typedef uint32_t Mode;

	inline Publisher() : m_mode(eco::value_none)
	{}

	// publish new content.
	inline Publisher(
		IN Topic::ptr& topic,
		IN Content::ptr& new_content)
		: m_topic(std::move(topic))
		, m_new_content(std::move(new_content))
		, m_mode(mode_publish_new)
	{}

	// publish some mode: erase_topic & clear content.
	inline Publisher(
		IN Topic::ptr& topic, 
		IN const Mode publish_mode)
		: m_topic(std::move(topic))
		, m_mode(publish_mode)
	{}

	// subscriber subscribe topic.
	inline Publisher(
		IN Topic::ptr& topic,
		IN AutoRefPtr<Subscription>& node,
		IN eco::TopicEvent::ptr& event)
		: m_topic(std::move(topic))
		, m_node(std::move(node))
		, m_mode(mode_publish_snap)
		, m_event(event)
	{}

	inline Publisher(Publisher&& pub)
		: m_topic(std::move(pub.m_topic))
		, m_new_content(std::move(pub.m_new_content))
		, m_node(std::move(pub.m_node))
		, m_event(pub.m_event)
		, m_mode(pub.m_mode)
	{}

	inline Publisher& operator=(Publisher&& pub)
	{
		m_event = pub.m_event;
		m_mode = pub.m_mode;
		m_topic = std::move(pub.m_topic);
		m_new_content = std::move(pub.m_new_content);
		m_node = std::move(pub.m_node);
		return *this;
	}

	inline void operator()(void)
	{
		switch (m_mode)
		{
		case mode_publish_new:
			m_topic->publish_new(m_new_content);
			break;
		case mode_publish_snap:
			m_topic->publish_snap(*m_node, m_event.get());
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
	uint32_t m_mode;
	Topic::ptr m_topic;
	TopicEvent::ptr m_event;
	Content::ptr m_new_content;
	AutoRefPtr<Subscription> m_node;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif