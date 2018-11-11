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
typedef eco::Topic* (*MakeTopic)(const void* id);



////////////////////////////////////////////////////////////////////////////////
// topic content snap type.
enum
{
	snap_head = 1,
	snap_last = 2,
	snap_none = 4,
};
typedef uint8_t Snap;


// whether it is a snap.
inline bool is_newc(IN const Snap v)
{
	return eco::has(v, snap_none);
}
inline bool is_snap(IN const Snap v)
{
	return !is_newc(v);
}
inline bool is_snap_last(IN const Snap v)
{
	return eco::has(v, snap_last);
}
inline bool is_snap_head(IN const Snap v)
{
	return eco::has(v, snap_head);
}


////////////////////////////////////////////////////////////////////////////////
class TopicClass
{
public:
	inline TopicClass(
		IN const uint32_t type_id,
		IN const char* class_name,
		IN const MakeTopic make_topic,
		IN const TopicClass* parent)
	{
		m_type_id = type_id;
		m_make_topic = make_topic;
		m_class_name = class_name;
		m_parent = parent;
	}

public:
	uint32_t m_type_id;
	MakeTopic m_make_topic;
	const char* m_class_name;
	const TopicClass* m_parent;
};


////////////////////////////////////////////////////////////////////////////////
// define customer topic.
#define ECO_TOPIC(topic_t, parent_t) \
ECO_OBJECT(topic_t) \
public:\
	inline topic_t() {} \
	static const eco::TopicClass* topic_class()\
	{\
		static eco::TopicClass s_clss(eco::TypeId<topic_t>::value, \
			#topic_t, &topic_t::make, parent_t::topic_class());\
		return &s_clss; \
	}\
	static const eco::TopicClass* parent()\
	{\
		return topic_class()->m_parent;\
	}\
	virtual const eco::TopicClass* get_topic_class() const\
	{\
		return topic_class();\
	}\
	inline static eco::Topic* make(const void* id)\
	{\
		topic_t* topic = new topic_t();\
		topic->m_id = *(const TopicId*)id;\
		return topic; \
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
class ContentData
{
	ECO_OBJECT(ContentData);
public:
	inline ContentData(IN const eco::meta::Stamp v)	: m_stamp(v)
	{}

	// destructor.
	virtual ~ContentData() = 0 
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
class ContentDataT : public eco::ContentData
{
public:
	inline ContentDataT(IN const Value& v, IN eco::meta::Stamp ts)
		: m_value((Value&)v), eco::ContentData(ts)
	{}

	virtual ~ContentDataT()
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
class Content
{
public:
	inline Content(
		IN eco::ContentData::ptr& content,
		IN eco::Snap snap,
		IN eco::TopicEvent* event = nullptr)
		: m_event(event), m_snap(snap), m_content(&content)
	{}

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

	// content type.
	inline const uint32_t type_id() const
	{
		return (**m_content).get_type_id();
	}

	// content data.
	inline eco::ContentData& data()
	{
		return **m_content;
	}
	inline const eco::ContentData::ptr& data_ptr() const
	{
		return *m_content;
	}

	// content snap.
	inline eco::Snap snap() const
	{
		return m_snap;
	}

	// get snap type.
	inline bool is_snap() const
	{
		return !eco::is_newc(m_snap);
	}
	inline bool is_newc() const
	{
		return eco::is_newc(m_snap);
	}
	inline bool is_snap_head() const
	{
		return eco::is_snap_head(m_snap);
	}
	inline bool is_snap_last() const
	{
		return eco::is_snap_last(m_snap);
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
	eco::Snap m_snap;
	eco::ContentData::ptr* m_content;
};


////////////////////////////////////////////////////////////////////////////////
class Subscriber : public eco::detail::Subscriber
{
	ECO_OBJECT(Subscriber);
public:
	inline Subscriber() {};
	virtual ~Subscriber() {};

	// publish content: new/remove content.
	virtual void on_publish(IN eco::Topic& t, IN eco::Content& c)
	{}

	// clear topic content
	virtual void on_clear(IN eco::Topic& t)
	{}

	// erase topic and clear content.
	virtual void on_erase(IN eco::Topic& t)
	{}
};


////////////////////////////////////////////////////////////////////////////////
class Topic : public detail::Topic
{
	ECO_OBJECT(Topic);
public:
	// topic class.
	static const TopicClass* topic_class()
	{
		static eco::TopicClass s_clss(0, 0, 0, 0);
		return &s_clss;
	}
	// get topic class.
	virtual const TopicClass* get_topic_class() const
	{
		return topic_class();
	}

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
		OUT eco::ContentData::ptr& new_c) 
	{
		return true;
	}

	// clear all content
	virtual void do_clear() {}

public:
	inline Topic() {}
	virtual ~Topic() {}

	// check the topic is a type of Topic.
	template<typename Topic>
	inline bool is_a() const
	{
		auto* tc = get_topic_class();
		while (tc && tc != Topic::topic_class())
		{
			tc = tc->m_parent;
		}
		return tc != 0;
	}

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
	virtual void publish_new(IN eco::ContentData::ptr& new_c)
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
				suber->on_publish(*this, Content(new_c, snap_none));
			}
			node = next;
		}
		eco::meta::clean(new_c->stamp());
	}

	// publish remove topic event.
	virtual void publish_erase()
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
	virtual void publish_clear()
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
		ContentData::ptr newc(new ContentDataT<
			object_t, object_t>(obj, eco::meta::stamp_insert));
		do_move(newc);
	}
	template<typename object_t>
	inline void push_back_raw(IN const std::shared_ptr<object_t>& obj)
	{
		typedef std::shared_ptr<object_t> value_t;
		ContentData::ptr newc(new ContentDataT<
			object_t, value_t>(obj, eco::meta::stamp_insert));
		do_move(newc);
	}
};


////////////////////////////////////////////////////////////////////////////////
// note that PopTopic is base topic of "One/Seq/Set" topic.
template<typename TopicId = eco::TopicId>
class PopTopic : public Topic
{
	ECO_TOPIC(PopTopic, Topic);
protected:
	typedef TopicId TopicId;
	TopicId m_id;

public:
	// topic identity.
	inline const TopicId& get_id() const
	{
		return m_id;
	}
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
		mode_publish_erase		= 3,	// when erase a topic.
		mode_publish_clear		= 4,	// when clear all content of topic.
	};
	typedef uint32_t Mode;

	inline Publisher() : m_mode(eco::value_none)
	{}

	// publish new content.
	inline Publisher(
		IN Topic::ptr& topic,
		IN ContentData::ptr& new_content)
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
		case mode_publish_erase:
			m_topic->publish_erase();
			break;
		case mode_publish_clear:
			m_topic->publish_clear();
			break;
		}
	}

private:
	uint32_t m_mode;
	Topic::ptr m_topic;
	TopicEvent::ptr m_event;
	ContentData::ptr m_new_content;
	AutoRefPtr<Subscription> m_node;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif