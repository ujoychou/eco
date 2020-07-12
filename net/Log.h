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
#define ECO_REQ(sev) ECO_LOG(sev) << Log(*this)(eco::net::req)
#define ECO_SUB(sev) ECO_LOG(sev) << Log(*this)(eco::net::sub)
#define ECO_PUB(sev, sess_or_conn, msg_type, msg_name) \
ECO_LOG(sev) << eco::net::Log(sess_or_conn, msg_type, msg_name)(eco::net::pub)

#define ECO_HDL(sev, hdl) ECO_LOG_1(sev)\
<< eco::net::MessageHandler::Log(hdl, (hdl).get_response_type())(eco::net::pub)

#define ECO_RSP_1(sev) ECO_LOG_1(sev)\
<< eco::net::Log(nullptr, response_type(), name())(eco::net::pub)
#define ECO_RSP_2(sev, resp) ECO_RSP_3(sev, resp, *this)
#define ECO_RSP_3(sev, resp, hdl)\
if (resp.has_error()){ \
	ECO_LOG_1(sev) << eco::net::MessageHandler::Log(\
	hdl, (hdl).get_response_type())(eco::net::rsp) <= resp.error(); \
} else \
	ECO_LOG_1(sev) << eco::net::MessageHandler::Log(\
	hdl, (hdl).get_response_type())(eco::net::rsp)
#define ECO_RSP(...) ECO_MACRO(ECO_RSP_,__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
#define ECO_REQX(sev) ECO_LOGX(sev) << Log(*this)(eco::net::req)
#define ECO_SUBX(sev) ECO_LOGX(sev) << Log(*this)(eco::net::sub)

#define ECO_PUBX(sev, sess_or_conn, msg_type, msg_name) ECO_LOGX(sev) << \
eco::net::Log(sess_or_conn, msg_type, msg_name)(eco::net::pub)

#define ECO_HDLX(sev, hdl) ECO_LOGX_1(sev) << \
eco::net::MessageHandler::Log(hdl, (hdl).get_response_type())(eco::net::pub);

#define ECO_RSPX_1(sev) ECO_LOGX_1(sev) \
<< eco::net::Log(nullptr, response_type(), name())(eco::net::pub);
#define ECO_RSPX_2(sev, resp) ECO_RSPX_3(sev, resp, *this)
#define ECO_RSPX_3(sev, hdl, resp)\
if (resp.has_error()){ \
	ECO_LOGX_1(sev) << eco::net::MessageHandler::Log(\
	hdl, (hdl).get_response_type())(eco::net::rsp) <= resp.error(); \
} else \
	ECO_LOGX_1(sev) << eco::net::MessageHandler::Log(\
	hdl, (hdl).get_response_type())(eco::net::rsp)
#define ECO_RSPX(...) ECO_MACRO(ECO_RSPX_,__VA_ARGS__)


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


	template<typename stream_t>
	inline stream_t& operator>>(OUT stream_t& log) const
	{
		log << '(' << m_conn_id;
		if (m_sess_id != none_session) log <= m_sess_id;
		log < ')';
		if (!m_func.null()) log <= m_func;
		log < " = ";
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
		, m_type(type), m_user_name(nullptr), m_user_id(0), m_mode(0)
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
		, m_type(type), m_user_id(0), m_user_name(nullptr), m_mode(0)
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
		, m_type(type), m_mode(0)
		, m_user_id(sess.get_user_id())
		, m_user_name(sess.get_user_name())
	{}

	inline Log(
		IN const TcpSession& sess,
		IN const uint32_t type,
		IN const char* func)
		: NetLog(sess.get_connection().get_id(), func, sess.get_id())
		, m_type(type), m_user_id(0), m_user_name(nullptr), m_mode(0)
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
		: NetLog(c.get_connection().get_id(), func, c.m_session.get_id())
		, m_type(type), m_user_id(0), m_user_name(nullptr), m_mode(0)
	{
		if (c.m_session.get_id() == none_session)
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
			auto sess_d = c.m_session.data();
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
		: NetLog(conn_id, func, sess_id), m_mode(0)
		, m_type(type), m_user_name(user_name), m_user_id(0)
	{}

	// scene: message handler logging.
	inline Log(
		IN const uint64_t conn_id,
		IN const uint32_t type,
		IN const char* func)
		: NetLog(conn_id, func, 0), m_mode(0)
		, m_type(type), m_user_name(0), m_user_id(0)
	{}

	// set mode.
	inline Log& operator()(uint8_t mode)
	{
		m_mode = mode;
		return *this;
	}

	// logging this handler.
	template<typename stream_t>
	inline stream_t& operator>>(OUT stream_t& log) const
	{
		log << '(' << m_conn_id;
		if (m_sess_id != none_session) log <= m_sess_id;
		log < ')' <= get_log(m_mode);
		if (m_type) log <= m_type;
		if (m_user_name) log <= '@' < m_user_name;
		if (m_user_id) log <= '@' < m_user_id;
		if (!m_func.null()) log <= m_func;
		log < " = ";
		return log;
	}

protected:
	uint8_t		m_mode;
	uint32_t	m_type;
	const char* m_user_name;
	uint64_t	m_user_id;
};


////////////////////////////////////////////////////////////////////////////////
template<typename stream_t>
stream_t& operator<<(OUT stream_t& stream, IN const eco::net::Log& log)
{
	return log >> stream;
}
template<typename stream_t>
stream_t& operator<<(OUT stream_t& stream, IN const eco::net::NetLog& log)
{
	return log >> stream;
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif