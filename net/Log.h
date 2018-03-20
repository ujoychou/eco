#ifndef ECO_NET_LOG_H
#define ECO_NET_LOG_H
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
#include <eco/log/Log.h>
#include <eco/net/Context.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class NetLog
{
public:
	inline NetLog(
		IN const uint64_t conn_id,
		IN const char* func = nullptr,
		IN const uint32_t sess_id = none_session)
		: m_func(func)
		, m_sess_id(sess_id)
		, m_conn_id(conn_id)
	{}
	inline NetLog(
		IN const uint64_t conn_id,
		IN const eco::Bytes& func,
		IN const uint32_t sess_id = none_session)
		: m_func(func)
		, m_sess_id(sess_id)
		, m_conn_id(conn_id)
	{}


	template<typename LogStream>
	inline LogStream& operator>>(OUT LogStream& log) const
	{
		log << eco::square(m_conn_id);
		if (m_sess_id != none_session) log <= eco::square(m_sess_id);
		if (!m_func.null()) log <= eco::brace() < m_func;
		return log;
	}

protected:
	eco::Bytes m_func;
	uint32_t m_sess_id;
	uint64_t m_conn_id;
};


////////////////////////////////////////////////////////////////////////////////
class Log : public NetLog
{
public:
	// get eco mode log.
	inline static const char* get_log(IN const uint16_t mode)
	{
		switch (mode)
		{
		case eco::net::req:
			return "req >";
		case eco::net::rsp:
			return "rsp =";
		case eco::net::sub:
			return "sub >";
		case eco::net::pub:
			return "pub =";
		}
		return "=== =";
	}

	inline Log(
		IN const ConnectionData* data,
		IN const uint32_t type,
		IN const char* func)
		: NetLog(0, func)
		, m_type(type)
		, m_user_name(nullptr)
		, m_user_id(0)
	{
		if (data != nullptr)
		{
			m_conn_id = data->get_id();
			m_user_id = data->get_user_id();
			m_user_name = data->get_user_name();
		}
	}

	inline Log(
		IN const TcpConnection& conn,
		IN const uint32_t type,
		IN const char* func)
		: NetLog(conn.get_id(), func, none_session)
		, m_type(type), m_user_id(0), m_user_name(nullptr)
	{
		auto data = conn.cast<ConnectionData>();
		if (!data.null())
		{
			m_user_id = data->get_user_id();
			m_user_name = data->get_user_name();
		}
	}

	inline Log(
		IN const SessionData& sess,
		IN const uint32_t type,
		IN const char* func)
		: NetLog(sess.get_connection().get_id(), func, sess.get_id())
		, m_type(type)
		, m_user_id(sess.get_user_id())
		, m_user_name(sess.get_user_name())
	{}

	inline Log(
		IN const TcpSession& sess,
		IN const uint32_t type,
		IN const char* func)
		: NetLog(sess.get_connection().get_id(), func, sess.get_id())
		, m_type(type), m_user_id(0), m_user_name(nullptr)
	{
		if (sess.get_id() == none_session)
		{
			auto data = sess.get_connection().cast<ConnectionData>();
			if (!data.null())
			{
				m_user_id = data->get_user_id();
				m_user_name = data->get_user_name();
			}
		}
		else
		{
			auto sess_d = sess.data();
			if (sess_d != nullptr)
			{
				m_user_id = sess_d->get_user_id();
				m_user_name = sess_d->get_user_name();
			}
		}
	}

	inline Log(
		IN const Context& c,
		IN const uint32_t type,
		IN const char* func)
		: NetLog(c.get_connection().get_id(), func, c.get_session().get_id())
		, m_type(type), m_user_id(0), m_user_name(nullptr)
	{
		if (c.get_session().get_id() == none_session)
		{
			auto data = c.get_connection().cast<ConnectionData>();
			if (!data.null())
			{
				m_user_id = data->get_user_id();
				m_user_name = data->get_user_name();
			}
		}
		else
		{
			auto sess_d = c.get_session().data();
			if (sess_d != nullptr)
			{
				m_user_id = sess_d->get_user_id();
				m_user_name = sess_d->get_user_name();
			}
		}
	}

	// scene: message handler logging.
	inline Log(
		IN const uint64_t conn_id,
		IN const uint32_t sess_id,
		IN const uint32_t type,
		IN const char* func,
		IN const char* user_name)
		: NetLog(conn_id, func, sess_id)
		, m_type(type)
		, m_user_name(user_name)
		, m_user_id(0)
	{}

	// scene: message handler logging.
	inline Log(
		IN const uint64_t conn_id,
		IN const uint32_t type,
		IN const char* func)
		: NetLog(conn_id, func, 0)
		, m_type(type)
		, m_user_name(0)
		, m_user_id(0)
	{}

	// logging this handler.
	template<typename LogStream>
	inline LogStream& operator>>(OUT LogStream& log) const
	{
		log << eco::square(m_conn_id);
		if (m_sess_id != none_session) log <= eco::square(m_sess_id);
		log <= get_log(log.flag());
		if (m_type) log <= m_type;
		if (m_user_name) log <= m_user_name;
		if (m_user_id) log < eco::group(m_user_id);
		if (!m_func.null()) log <= eco::brace() < m_func;
		return log;
	}

private:
	uint32_t	m_type;
	const char* m_user_name;
	uint64_t	m_user_id;
};


////////////////////////////////////////////////////////////////////////////////
template<typename Stream>
Stream& operator<<(OUT Stream& stream, IN const eco::net::Log& log)
{
	return log >> stream;
}
template<typename Stream>
Stream& operator<<(OUT Stream& stream, IN const eco::net::NetLog& log)
{
	return log >> stream;
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif