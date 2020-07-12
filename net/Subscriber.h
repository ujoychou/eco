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
#include <eco/net/Log.h>
#include <eco/thread/Map.h>
#include <eco/net/TcpConnection.h>
#include <eco/thread/topic/TopicServer.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class Publisher : public eco::Object<Publisher>
{
public:
	typedef uint32_t Option;
	typedef std::function<void(eco::Content&, eco::net::TcpConnection&)> Func;

	inline Publisher() : m_type(0), m_name(0) {}
	inline Publisher(uint32_t t, const char* log) : m_type(t), m_name(log) {}

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
		if (m_name != nullptr)
		{
			ECO_PUB(info, conn, m_type, m_name);
		}
	}

public:
	Func m_func;
	uint32_t m_type;
	const char* m_name;
};


////////////////////////////////////////////////////////////////////////////////
class Subscriber;
class Dispatcher;
class ECO_API TopicUidMap
{
private:
	// 注册发布主题：Subscriber
	friend class Subscriber;
	static Publisher::ptr get(const TopicUid& id);
	static void sub(IN const TopicUid& id, IN Publisher::ptr& pub);
	static void erase(IN const TopicUid& id);

	// 注册发布主题：Dispatcher
	friend class Dispatcher;
	static Publisher::ptr get(const TopicUid& id, uint32_t type_id);
	static void set(IN uint32_t type_id, IN Publisher::ptr& pub);
	static void set(IN const TopicUid& id, IN Publisher::ptr& pub);
};


////////////////////////////////////////////////////////////////////////////////
class Subscriber : public eco::net::ConnectionData, public eco::Subscriber
{
public:
	// subscribe mode: topic and register map. thread-safe.
	template<
		typename topic_t,
		typename object_t = topic_t::object,
		typename topic_server_t,
		typename topic_id_t>
	inline void subscribe(
		IN topic_server_t& server,
		IN const topic_id_t& topic_id,
		IN uint32_t pub_type,
		IN TopicEvent::ptr event = nullptr,
		IN const char* log_name = nullptr)
	{
		Publisher::ptr pub(new Publisher(pub_type, log_name));
		pub->register_func<eco::Raw<object_t>::pointer>();
		TopicUidMap::sub(TopicUid(topic_id, &server), pub);
		server.subscribe<topic_t>(topic_id, this, event);
	}

	// subscribe mode: unregister map. thread-safe.
	template<typename topic_server_t, typename topic_id_t>
	inline void unsubscribe(
		IN topic_server_t& server,
		IN const topic_id_t& topic_id,
		IN TopicEvent::ptr event = nullptr)
	{
		int res = server.unsubscribe(topic_id, this, event);
		if (res == 2) TopicUidMap::erase(TopicUid(topic_id, &server));
	}

protected:
	virtual void on_default(IN eco::Topic& topic, IN eco::Content& c) {};

	// eco::Subscriber on_publish：dispatch by tuid.
	virtual void on_publish(IN eco::Topic& topic, IN eco::Content& c) override
	{
		auto pub = TopicUidMap::get(topic.get_uid());
		if (pub != nullptr)
		{
			pub->m_func(c, connection());
			return;
		}
		on_default(topic, c);
	}
};


////////////////////////////////////////////////////////////////////////////////
class Dispatcher : public eco::net::ConnectionData, public eco::Subscriber
{
public:
	// dispatch mode: register topic id when app.on_init. thread-unsafe.
	template<typename object_t, typename topic_server_t, typename topic_id_t>
	inline static void publish(
		IN topic_server_t& server,
		IN const topic_id_t& topic_id,
		IN uint32_t pub_type,
		IN const char* log_name = nullptr)
	{
		Publisher::ptr pub(new Publisher(pub_type, log_name));
		pub->register_func<eco::Raw<object_t>::pointer>();
		TopicUidMap::set(TopicUid(topic_id, &server), pub);
	}

	// dispatch mode: register type id when app.on_init. thread-unsafe.
	template<typename object_t>
	inline static void publish(
		IN uint32_t pub_type,
		IN const char* log_name = nullptr)
	{
		Publisher::ptr pub(new Publisher(pub_type, log_name));
		pub->register_func<eco::Raw<object_t>::pointer>();
		TopicUidMap::set(eco::TypeId<object_t>(), pub);
	}

	// subscribe mode: topic and register map. thread-safe.
	template<
		typename topic_t,
		typename object_t = topic_t::object,
		typename topic_server_t,
		typename topic_id_t>
		inline void subscribe(
			IN topic_server_t& server,
			IN const topic_id_t& topic_id,
			IN TopicEvent::ptr event = nullptr)
	{
		server.subscribe<topic_t>(topic_id, this, event);
	}

protected:
	virtual void on_default(IN eco::Topic& topic, IN eco::Content& c) {};

	// eco::Subscriber on_publish.
	virtual void on_publish(IN eco::Topic& topic, IN eco::Content& c) override
	{
		Publisher::ptr pub = TopicUidMap::get(topic.get_uid(), c.type_id());
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