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


namespace eco{;
namespace net{;
class TcpPeer;
typedef std::weak_ptr<TcpPeer> TcpPeerWptr;
////////////////////////////////////////////////////////////////////////////////
class DataContext
{
public:
	inline DataContext()
		: m_protocol(0), m_category(0), m_head_size(0)
	{}

	inline DataContext(IN const DataContext& v)
		: m_protocol(0), m_category(0), m_head_size(0)
	{
		assert(false);
		ECO_ERROR << "data context copy contructor.";
	}

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

	inline const bool is_last() const
	{
		return m_meta.is_last();
	}

	inline const uint32_t has_error() const
	{
		return m_meta.has_error();
	}

	inline void release_data()
	{
		m_data.release();
		m_message.clear();
	}

	inline Context& operator=(IN Context&& c)
	{
		m_data = std::move(m_data);
		m_message = c.m_message;
		m_meta = c.m_meta;
		m_session = c.m_session;
		return *this;
	}

	inline void response(IN MessageMeta& meta)
	{
		m_session.response(meta, *this);
	}
};


}}
////////////////////////////////////////////////////////////////////////////////
#endif