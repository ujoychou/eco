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
#define ECO_PUB(sev, sess, type, name) \
ECO_LOG(sev) << eco::net::Log(sess, type, name)(eco::net::pub)

#define ECO_RSP_1(sev) ECO_LOG_1(sev)\
<< eco::net::Log(connection(), response_type(), name())(eco::net::rsp)
#define ECO_RSP_2(sev, hdl) ECO_RSP_3(sev, hdl, (hdl).response_type())
#define ECO_RSP_3(sev, hdl, type) \
ECO_LOG_1(sev) << eco::net::MessageHandler::Log(hdl, type)(eco::net::rsp)
#define ECO_RSP(...) ECO_MACRO(ECO_RSP_, __VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
#define ECO_REQX(sev) ECO_LOGX(sev) << Log(*this)(eco::net::req)
#define ECO_SUBX(sev) ECO_LOGX(sev) << Log(*this)(eco::net::sub)
#define ECO_PUBX(sev, sess, type, name) \
ECO_LOGX(sev) << eco::net::Log(sess, type, name)(eco::net::pub)

#define ECO_RSPX_1(sev) ECO_LOGX_1(sev)\
<< eco::net::Log(connection(), response_type(), name())(eco::net::rsp)
#define ECO_RSPX_2(sev, hdl) ECO_RSPX_3(sev, hdl, (hdl).response_type())
#define ECO_RSPX_3(sev, hdl, type) \
ECO_LOGX_1(sev) << eco::net::MessageHandler::Log(hdl, type)(eco::net::rsp)
#define ECO_RSPX(...) ECO_MACRO(ECO_RSPX_, __VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
#define ECO_REQ_SEV(sev) ECO_LOG_SEV(sev) << Log(*this)(eco::net::req)
#define ECO_SUB_SEV(sev) ECO_LOG_SEV(sev) << Log(*this)(eco::net::sub)
#define ECO_RSP_SEV(sev) ECO_LOG_SEV(sev) << Log(*this)(eco::net::rsp)


////////////////////////////////////////////////////////////////////////////////
class NetLog
{
public:
	inline NetLog(
		IN const uint64_t conn_id,
		IN const char* func = nullptr,
		IN const uint64_t sess_id = none_session)
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
	uint64_t m_sess_id;
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
		IN const TcpConnection& conn,
		IN const uint32_t type,
		IN const char* func)
		: NetLog(conn.id(), func, none_session)
		, m_type(type), m_user(conn.user()), m_mode(0)
	{}

	inline Log(
		IN const Context& c,
		IN const uint32_t type,
		IN const char* func)
		: NetLog(c.connection().id(), func, c.m_session.id())
		, m_type(type), m_user(c.connection().user()), m_mode(0)
	{}

	inline Log(
		IN const uint64_t conn_id,
		IN const uint64_t sess_id,
		IN const uint32_t type,
		IN const char* func,
		IN const char* user)
		: NetLog(conn_id, func, sess_id)
		, m_mode(0), m_type(type), m_user(user)
	{}

	inline Log(
		IN const uint64_t conn_id,
		IN const uint32_t type,
		IN const char* func)
		: NetLog(conn_id, func, 0), m_mode(0), m_type(type), m_user(0)
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
		if (m_user) log <= '@' < m_user;
		if (!m_func.null()) log <= m_func;
		log < " = ";
		return log;
	}

protected:
	uint8_t		m_mode;
	uint32_t	m_type;
	const char* m_user;
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