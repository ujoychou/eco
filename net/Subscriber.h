#ifndef ECO_NET_SUBSCRIBER_H
#define ECO_NET_SUBSCRIBER_H
/*******************************************************************************
@ name


@ function


@ exception


@ remark


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/thread/Map.h>
#include <eco/net/TcpConnection.h>
#include <eco/thread/topic/TopicServer.h>


ECO_NS_BEGIN(eco);
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class Publisher : public eco::Object<Publisher>
{
public:
	typedef uint32_t Option;
	typedef std::function<void(eco::Content&, eco::net::TcpConnection&)> Func;

	inline Publisher(uint32_t type) : m_type(type) {}

	template<typename object_pointer>
	inline Publisher& register_func()
	{
		m_func = std::bind(&Publisher::on_publish<object_pointer>, this,
			std::placeholders::_1, std::placeholders::_2);
		return *this;
	}

	template<typename object_pointer>
	inline void on_publish(eco::Content& c, eco::net::TcpConnection& conn)
	{
		auto& v = *(object_pointer)(c.data().get_object());
		conn.publish(v, m_type, c.meta());
	}

public:
	Func m_func;
	uint32_t m_type;
	const char* m_name;
};


////////////////////////////////////////////////////////////////////////////////
class Subscriber;
class ECO_API TopicUidMap
{
private:
	// ע�ᷢ�����⣺Subscriber
	friend class Subscriber;
	static Publisher::ptr get(const TopicUid& id);
	static void sub(IN const TopicUid& id, IN Publisher::ptr& pub);
	static void erase(IN const TopicUid& id);

	// ע�ᷢ�����⣺Dispatcher
	static Publisher::ptr get(const TopicUid& id, uint32_t type_id);
	static void set(IN uint32_t type_id, IN Publisher::ptr& pub);
	static void set(IN const TopicUid& id, IN Publisher::ptr& pub);
};


////////////////////////////////////////////////////////////////////////////////
class Subscriber : public eco::net::ConnectionData, public eco::Subscriber
{
public:
	// dispatch mode: register type id when app.on_init. thread-unsafe.
	template<typename object_t>
	inline static void publish(IN uint32_t pub_type)
	{
		Publisher::ptr pub(new Publisher(pub_type));
		pub->register_func<object_t*>();
		TopicUidMap::set(eco::TypeId<object_t>(), pub);
	}

	// dispatch mode: register topic id when app.on_init. thread-unsafe.
	template<typename object_t, typename topic_server_t, typename topic_id_t>
	inline static void publish(
		IN topic_server_t& server,
		IN const topic_id_t& topic_id,
		IN uint32_t pub_type)
	{
		Publisher::ptr pub(new Publisher(pub_type));
		pub->register_func<object_t*>();
		TopicUidMap::set(TopicUid(topic_id, &server), pub);
	}

public:
	// subscribe mode: topic and register map. thread-safe.
	template<
		typename topic_t,
		typename topic_server_t,
		typename topic_id_t>
	inline void subscribe(
		IN topic_server_t& server,
		IN const topic_id_t& topic_id,
		IN uint32_t pub_type)
	{
		Publisher::ptr pub(new Publisher(pub_type));
		pub->register_func<topic_t::object*>();
		TopicUidMap::sub(TopicUid(topic_id, &server), pub);
		server.subscribe<topic_t>(topic_id, this);
	}

	// subscribe mode: topic and register map. thread-safe.
	template<
		typename topic_t,
		typename object_t,
		typename topic_server_t,
		typename topic_id_t>
		inline void subscribe(
		IN topic_server_t& server,
		IN const topic_id_t& topic_id,
		IN uint32_t pub_type)
	{
		Publisher::ptr pub(new Publisher(pub_type));
		pub->register_func<object_t*>();
		TopicUidMap::sub(TopicUid(topic_id, &server), pub);
		server.subscribe<topic_t>(topic_id, this);
	}

	// subscribe mode: unregister map. thread-safe.
	template<typename topic_server_t, typename topic_id_t>
	inline void unsubscribe(
		IN topic_server_t& server,
		IN const topic_id_t& topic_id)
	{
		int res = server.unsubscribe(topic_id, this);
		if (res == 2) TopicUidMap::erase(TopicUid(topic_id, &server));
	}

protected:
	virtual void on_default(IN eco::Topic& topic, IN eco::Content& c) {};

	// eco::Subscriber on_publish��dispatch by tuid.
	virtual void on_publish(IN eco::Topic& topic, IN eco::Content& c) override
	{
		Publisher::ptr pub = TopicUidMap::get(topic.get_uid(), c.object_id());
		if (pub != nullptr)
		{
			pub->m_func(c, connection());
			return;
		}
		on_default(topic, c);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif