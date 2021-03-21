#ifndef ECO_NET_CONTEXT_H
#define ECO_NET_CONTEXT_H
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/meta/Stamp.h>
#include <eco/net/TcpSession.h>
#include <eco/thread/topic/Role.h>
#include <eco/loc/Locale.h>


namespace eco{;
namespace net{;
eco::loc::Locale& app_locale();
////////////////////////////////////////////////////////////////////////////////
#define ECO_REQ(sev) \
ECO_LOG(sev) << eco::net::Log(m_context, name())(eco::net::req)
#define ECO_SUB(sev) \
ECO_LOG(sev) << eco::net::Log(m_context, name())(eco::net::sub)

#define ECO_RSP_1(sev) ECO_LOG_1(sev)\
<< eco::net::Log(m_context, name(), rsp_type())(eco::net::rsp)
#define ECO_RSP_2(sev, hdl) ECO_RSP_3(sev, hdl, (hdl).rsp_type())
#define ECO_RSP_3(sev, hdl, type) ECO_LOG_1(sev) \
<< eco::net::Log((hdl).context(), (hdl).name(), type)(eco::net::rsp)
#define ECO_RSP(...) ECO_MACRO(ECO_RSP_, __VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
#define ECO_REQ_SEV(sev) \
ECO_LOG_SEV(sev) << eco::net::Log(m_context, get_name())(eco::net::req)
#define ECO_SUB_SEV(sev) \
ECO_LOG_SEV(sev) << eco::net::Log(m_context, get_name())(eco::net::sub)
#define ECO_RSP_SEV(sev) \
ECO_LOG_SEV(sev) << eco::net::Log(m_context, get_name())(eco::net::rsp)


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
		IN const uint64_t sess_id = none_session)
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
		log < " =";
		return log;
	}

protected:
	eco::Bytes m_func;
	uint64_t m_sess_id;
	uint64_t m_conn_id;
};


////////////////////////////////////////////////////////////////////////////////
class TcpPeer;
typedef std::weak_ptr<TcpPeer> TcpPeerWptr;
class DataContext
{
public:
	inline DataContext()
		: m_protocol(0), m_category(0), m_head_size(0)
	{}

#ifdef ECO_NO_STD_MOVE
	inline DataContext(IN const DataContext& cv)
	{
		//ECO_THROW(-1) << "data context copy contructor.";
		DataContext& v = (DataContext&)cv;
		m_data = std::move(v.m_data);
		m_category = v.m_category;
		m_head_size = v.m_head_size;
		m_protocol = v.m_protocol;
		m_peer_wptr = std::move(v.m_peer_wptr);
	}
#endif

	inline DataContext(IN DataContext&& v)
		: m_data(std::move(v.m_data))
		, m_category(v.m_category)
		, m_head_size(v.m_head_size)
		, m_protocol(v.m_protocol)
		, m_peer_wptr(std::move(v.m_peer_wptr))
	{}

	inline DataContext& operator=(IN DataContext&& v)
	{
		m_data = std::move(v.m_data);
		m_category = v.m_category;
		m_head_size = v.m_head_size;
		m_protocol = v.m_protocol;
		m_peer_wptr = std::move(v.m_peer_wptr);
		return *this;
	}

	// message bytes size.
	inline uint32_t size() const
	{
		return m_data.size();
	}
	// message bytes.
	inline const char* c_str(IN uint32_t pos = 0) const
	{
		return &m_data[pos];
	}

public:
	// message data.
	eco::String			m_data;
	MessageCategory		m_category;
	uint32_t			m_head_size;

	// protocol and tcp peer.
	Protocol*			m_protocol;
	TcpPeerWptr			m_peer_wptr;
};


////////////////////////////////////////////////////////////////////////////////
class Context
{
public:
	eco::String			m_data;
	eco::Bytes			m_message;
	MessageMeta			m_meta;
	mutable TcpSession	m_session;
	
public:
	inline Context()
	{}

	inline TcpConnection& connection() const
	{
		return m_session.connection();
	}

	inline const uint32_t type() const
	{
		return m_meta.m_message_type;
	}

	// publish topic content snap.
	inline const Snap snap() const
	{
		return Snap(m_meta.get_req1() & 0xFF);
	}
	inline bool snap_none() const
	{
		return snap() == eco::snap_none;
	}
	inline bool snap_head() const
	{
		return eco::has(snap(), eco::snap_head);
	}
	inline bool snap_last() const
	{
		return eco::has(snap(), eco::snap_last);
	}

	// publish topic content stamp.
	inline const meta::Stamp stamp() const
	{
		return meta::Stamp(m_meta.get_req1() >> 4);
	}
	inline bool stamp_delete() const
	{
		return eco::has(stamp(), eco::meta::stamp_delete);
	}

	inline bool is_last() const
	{
		return m_meta.is_last();
	}

	inline bool has_error() const
	{
		return m_meta.has_error();
	}

	inline void release_data()
	{
		m_data.release();
		m_message.clear();
	}

#ifdef ECO_NO_STD_MOVE
	inline Context(IN const Context& cc)
	{
		//ECO_THROW(-1) << "context copy contructor.";
		Context& c = (Context&)cc;
		m_data.move_from(c.m_data);
		m_message = c.m_message;
		m_meta = c.m_meta;
		m_session = c.m_session;
	}

  inline Context& operator=(IN const Context& cc)
  {
    //ECO_THROW(-1) << "context copy contructor.";
    Context& c = (Context&)cc;
    m_data.move_from(c.m_data);
    m_message = c.m_message;
    m_meta = c.m_meta;
    m_session = c.m_session;
    return *this;
  }
#endif

	inline void send(IN MessageMeta& meta)
	{
		m_session.response(meta, *this);
	}

#ifndef ECO_NO_PROTOBUF
public:
	// response message to client.
	inline void resolve(
		uint32_t type,
		const google::protobuf::Message* msg = nullptr,
		bool last_ = true)
	{
		send(type, msg, last_, false);
	}

	// response message to client.
	template<typename object_t, typename set_t>
	inline void resolve_set(uint32_t type, const set_t& set)
	{
		if (set.empty()) return resolve(type, nullptr);
		for (auto it = set.begin(); it != set.end(); )
		{
			const object_t& obj = eco::object(*it);
			resolve(type, &obj, ++it == set.end());
		}
	}

	// response error message to client.
	inline void reject(uint32_t type)
	{
		app_locale().parse_error("", connection().lang());
		reject(type, &eco::this_thread::proto::error());
	}
	inline void reject(uint32_t type, eco::proto::Error& e, const char* mdl)
	{
		app_locale().parse_error(e, mdl, connection().lang());
		reject(type, &e);
	}
	inline void reject(uint32_t type, uint32_t eid)
	{
		eco::this_thread::error().key(eid);
		reject(type);
	}
	inline void reject(uint32_t type, const std::string& path_)
	{
		eco::this_thread::error().key(path_.c_str());
		reject(type);
	}
	inline void reject(uint32_t type, const google::protobuf::Message* msg)
	{
		send(type, msg, true, true);
	}

	inline void send(
		int type, const google::protobuf::Message* msg,
		bool last_, bool err);

	inline void send(
		int type, const google::protobuf::Message* msg,
		const char* name, int sev, bool err);
#endif
};


////////////////////////////////////////////////////////////////////////////////
class Log : public NetLog
{
public:
	// get eco mode log.
	inline static const char* get_log(IN uint16_t mode)
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

	inline Log(const Context& c, const char* func = "", uint32_t type = 0)
		: NetLog(c.connection().id(), func, c.m_session.id()), m_mode(0)
		, m_type(type != 0 ? type : c.type()), m_user(c.connection().user())
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
		log < '(' < m_conn_id;
		if (m_sess_id != none_session) log <= m_sess_id;
		log < ')' <= get_log(m_mode);
		if (m_type) log <= m_type;
		if (!eco::empty(m_user)) log <= '@' < m_user;
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


#ifndef ECO_NO_PROTOBUF
////////////////////////////////////////////////////////////////////////////////
void Context::send(int type, const google::protobuf::Message* msg,
	bool last, bool err)
{
	ProtobufCodec cdc(msg);
	MessageMeta meta;
	meta.codec(cdc).message_type(type).last(last).error(err);
	send(meta);
}
void Context::send(int type, const google::protobuf::Message* msg,
	const char* name, int sev, bool err)
{
	Context::send(type, msg, true, err);

	// logging protobuf.
	if (sev == eco::log::none) return;
	ECO_LOG_SEV(sev) << Log(*this, name)(rsp)
		< (msg ? eco::net::to_json(*msg) : empty_str);
}
#endif

////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif