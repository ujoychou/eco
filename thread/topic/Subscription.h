#ifndef ECO_DETAIL_SUBSCRIPTION_H
#define ECO_DETAIL_SUBSCRIPTION_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/thread/Mutex.h>
#include <eco/thread/AutoRef.h>
#include <forward_list>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(detail);
class Topic;
class Subscriber;
ECO_NS_END(detail);
ECO_NS_END(eco);


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class Subscription;
class SubscriptionList
{
public:
	Subscription* m_head;
	Subscription* m_tail;

	inline SubscriptionList() 
		: m_head(nullptr)
		, m_tail(nullptr)
	{}

	inline void clear()
	{
		m_head = nullptr;
		m_tail = nullptr;
	}
};


////////////////////////////////////////////////////////////////////////////////
class Subscription
{
	ECO_OBJECT_AUTOREF(uint32_t);
public:
	/* this subscription will be added to topic's subscriber list, and record
	the position of topic's subscriber list, when release subscriber and it's
	also will clear the subscriber nodes referenced by topic.
	*/
	detail::Topic* m_topic;
	Subscription*  m_topic_subscriber_next;
	Subscription** m_topic_subscriber_prev;

	/* this subscription will be added to subscriber's topic list, and record
	the position of subscriber's topic list, when release topic it also will
	clear the topic nodes referenced by subscriber.
	*/
	detail::Subscriber* m_subscriber;
	Subscription*  m_subscriber_topic_next;
	Subscription** m_subscriber_topic_prev;

	// when unsubsribe: identify the node is erasing.
	std::forward_list<Subscription**> m_erasing_nodes;

	// subscription working state.
	uint32_t m_working;

	// construct.
	inline Subscription(IN detail::Topic* topic, IN detail::Subscriber* sub)
		: m_topic(topic), m_subscriber(sub)
		, m_topic_subscriber_next(nullptr)
		, m_topic_subscriber_prev(nullptr)
		, m_subscriber_topic_next(nullptr)
		, m_subscriber_topic_prev(nullptr)
		, m_working(false)
	{}

	// when delete node: it need get a erasing node to notify "thead stack node"
	// that this node has been erased.
	inline Subscription**& new_erasing_node()
	{
		m_erasing_nodes.push_front(nullptr);
		return m_erasing_nodes.front();
	}

	// reserve subscriber in topic, and subscriber will really work until
	// subscribe() setting working state to "true".
	inline void reserve_subscribe(
		IN SubscriptionList& topic_subscriber_head,
		IN Subscription*& subscriber_topic_head)
	{
		add_topic_subscriber(topic_subscriber_head);
		add_subscriber_topic(subscriber_topic_head);
		add_ref();
	}

	// set subscription working state.
	inline void confirm_subscribe()
	{
		m_working = true;
	}

	// erase this subscription node from both of subscriber and topic.
	inline void unsubscribe()
	{
		// 将多个线程中正在删除该节点的“操作节点”置空，以便通知他们该节点已被删除。
		// 从而避免出现多个线程同时删除一个节点导致的野指针。
		auto it = m_erasing_nodes.begin();
		for (; it != m_erasing_nodes.end(); ++it)
		{
			assert(*it != nullptr);
			(**it) = nullptr;
		}
		erase_subscriber_topic();
		erase_topic_subscriber();
		del_ref();
	}

private:
	// add subscriber in topic.
	inline void add_topic_subscriber(IN SubscriptionList& head)
	{
		// when there is a empty topic list.
		// 技巧：除了第一个节点的Prev指向“头部”，其他节点Prev均指向上一个节点的Next。
		// 而此恰恰能在删除节点时，“头部”指针自动移向下一个节点。
		if (head.m_head == nullptr)
		{
			head.m_head = this;
			m_topic_subscriber_prev = &head.m_head;
		}
		else
		{
			head.m_tail->m_topic_subscriber_next = this;
			m_topic_subscriber_prev = &head.m_tail->m_topic_subscriber_next;
		}
		head.m_tail = this;
	}

	// add topic in subscriber.
	inline void add_subscriber_topic(IN Subscription*& head)
	{
		// add this node in front of subscriber's topic list.
		if (head != nullptr)
		{
			head->m_subscriber_topic_prev = &m_subscriber_topic_next;
		}
		m_subscriber_topic_next = head;
		m_subscriber_topic_prev = &head;
		head = this;
	}

	// erase topic's subscriber item.
	inline void erase_topic_subscriber()
	{
		*m_topic_subscriber_prev = m_topic_subscriber_next;
		if (m_topic_subscriber_next)
		{
			m_topic_subscriber_next->m_topic_subscriber_prev
				= m_topic_subscriber_prev;
		}
		m_subscriber = nullptr;
	}

	// erase subscriber's topic item.
	inline void erase_subscriber_topic()
	{
		*m_subscriber_topic_prev = m_subscriber_topic_next;
		if (m_subscriber_topic_next)
		{
			m_subscriber_topic_next->m_subscriber_topic_prev
				= m_subscriber_topic_prev;
		}
		m_topic = nullptr;
	}
};
ECO_NS_END(eco);



ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(detail);
////////////////////////////////////////////////////////////////////////////////
class Subscriber
{
public:
	inline Subscriber();
	inline ~Subscriber();

	// where subscriber has subscribe topic.
	inline bool has_topic(IN const Topic* topic) const;

	// erase topic from topic list.
	inline bool unsubscribe(IN Topic* topic);

	// clear topic list.
	inline void unsubscribe_all();

private:
	// topic list.
	friend class Topic;
	mutable eco::Mutex m_mutex;
	Subscription* m_subscriber_topic_head;
};


////////////////////////////////////////////////////////////////////////////////
class Topic
{
public:
	inline ~Topic();

	// add subscriber to this topic.
	inline AutoRefPtr<Subscription> reserve_subscribe(
		IN Subscriber* subscriber);

	// erase subscriber from this topic.
	inline bool unsubscribe(IN Subscriber* subscriber);

	// find subscriber in this topic.
	inline bool has_subscriber(IN Subscriber* subscriber) const;

	// is there subscriber in this topic.
	inline bool has_subscriber() const;

	// clear all subscriber in this topic.
	inline void clear();

	// thread safe mutex.
	inline eco::Mutex& subscribe_mutex() const
	{
		return m_mutex;
	}

	// thread safe mutex.
	inline Subscription* subscriber_head() const
	{
		return m_topic_subscriber_head.m_head;
	}
	
private:
	// manage subscription's life cycle.
	friend class Subscriber;
	mutable eco::Mutex m_mutex;
	SubscriptionList m_topic_subscriber_head;
};



//##############################################################################
//##############################################################################
inline Subscriber::Subscriber() : m_subscriber_topic_head(nullptr)
{}
inline Subscriber::~Subscriber()
{
	unsubscribe_all();
}


////////////////////////////////////////////////////////////////////////////////
inline void Subscriber::unsubscribe_all()
{
	// 从头部节点开始逐个清理。
	eco::Mutex::ScopeLock lock(m_mutex);
	while (m_subscriber_topic_head != nullptr)
	{
		// 原理可参考Topic.clear。
		eco::Mutex::OrderRelock rlock(m_mutex,
			m_subscriber_topic_head->m_topic->m_mutex);
		if (m_subscriber_topic_head != nullptr)
		{
			m_subscriber_topic_head->unsubscribe();	// 自动移向下一个节点。
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
inline bool Subscriber::unsubscribe(IN Topic* topic)
{
	eco::Mutex::ScopeLock lock(m_mutex);
	Subscription* node = m_subscriber_topic_head;
	while (node != nullptr)
	{
		if (node->m_topic != topic)
		{
			node = node->m_subscriber_topic_next;
			continue;
		}
		
		// 如果在OrderRelock加锁时有其他线程删除了该节点，则该节点将为空。
		// 由于存在多个线程同时调用unsubscribe删除该节点，所以使用erasing_node_list
		// 从而避免多线程相互干扰。
		node->new_erasing_node() = &node;
		eco::Mutex::OrderRelock rlock(m_mutex, node->m_topic->m_mutex);
		if (node != nullptr)		// node为空，表示该节点已被删除。
		{
			node->unsubscribe();
		}
		return true;
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
inline bool Subscriber::has_topic(IN const Topic* topic) const
{
	eco::Mutex::ScopeLock lock(m_mutex);
	Subscription* node = m_subscriber_topic_head;
	while (node != nullptr)
	{
		if (node->m_topic == topic)
		{
			return true;
		}
		node = node->m_subscriber_topic_next;
	}
	return false;
}


//##############################################################################
//##############################################################################
inline Topic::~Topic()
{
	clear();
}


////////////////////////////////////////////////////////////////////////////////
inline AutoRefPtr<Subscription> Topic::reserve_subscribe(
	IN Subscriber* subscriber)
{
	AutoRefPtr<Subscription> aref;
	if (has_subscriber(subscriber))
	{
		return aref;
	}

	Subscription* node(new Subscription(this, subscriber));
	eco::Mutex::OrderLock lock(m_mutex, subscriber->m_mutex);
	node->reserve_subscribe(m_topic_subscriber_head,
		subscriber->m_subscriber_topic_head);
	aref.reset(node);
	return aref;
}


////////////////////////////////////////////////////////////////////////////////
inline bool Topic::unsubscribe(IN Subscriber* subscriber)
{
	return subscriber->unsubscribe(this);
}


////////////////////////////////////////////////////////////////////////////////
inline void Topic::clear()
{
	eco::Mutex::ScopeLock lock(m_mutex);
	if (m_topic_subscriber_head.m_head == nullptr)
	{
		return;
	}
	Subscription* node = m_topic_subscriber_head.m_head;
	node->m_topic_subscriber_prev = &node;
	while (node != nullptr)
	{
		/* 当前节点可能在OrderRelock期间（可能会释放锁）被其他线程删除掉。但没关系由于
		设置了“head->m_topic_subscriber_prev = &node”，node将自动移向下一个节点。

		#1.技巧：除了第一个节点的Prev指向“头部”，其他节点Prev均指向上一个节点的Next。
		而此恰恰能在【从头开始删除节点】时，“头部”指针自动移向下一个节点。
		#2.注意：【非从头删除节点】需要使用m_erasing_node。
		*/
		eco::Mutex::OrderRelock rlock(m_mutex, node->m_subscriber->m_mutex);
		if (node != nullptr)
		{
			node->unsubscribe();		// 将自动移向下一个节点。
		}
	}
	m_topic_subscriber_head.clear();
}


////////////////////////////////////////////////////////////////////////////////
inline bool Topic::has_subscriber(IN Subscriber* subscriber) const
{
	return subscriber->has_topic(this);
}


////////////////////////////////////////////////////////////////////////////////
inline bool Topic::has_subscriber() const
{
	eco::Mutex::ScopeLock lock(m_mutex);
	return m_topic_subscriber_head.m_head != nullptr;
}



////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(detail);
ECO_NS_END(eco);
#endif