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
#include <eco/Project.h>
#include <eco/net/protocol/Protocol.h>
#include <eco/net/TcpSession.h>
#include <eco/net/TcpConnection.h>


namespace eco{;
namespace net{;
class TcpPeer;
typedef std::weak_ptr<TcpPeer> TcpPeerWptr;
////////////////////////////////////////////////////////////////////////////////
class DataContext
{
	ECO_OBJECT(DataContext);
public:
	inline DataContext(IN TcpSessionOwner* owner = nullptr)
		: m_prot(nullptr), m_category(0)
	{
		if (owner != nullptr)
			m_session_owner = *owner;
	}

	inline DataContext(IN DataContext&& v)
		: m_data(std::move(v.m_data))
		, m_category(v.m_category)
		, m_prot(v.m_prot)
		, m_session_owner(v.m_session_owner)
		, m_peer_wptr(std::move(v.m_peer_wptr))
	{}

	inline DataContext& operator=(IN DataContext&& v)
	{
		m_data = std::move(v.m_data);
		m_category = v.m_category;
		m_prot = v.m_prot;
		m_session_owner = v.m_session_owner;
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

	// protocol and tcp peer.
	Protocol*			m_prot;
	TcpPeerWptr			m_peer_wptr;
	TcpSessionOwner		m_session_owner;
};


////////////////////////////////////////////////////////////////////////////////
class Context
{
public:
	eco::String			m_data;
	eco::Bytes			m_message;
	MessageMeta			m_meta;
	TcpSession			m_session;
	
public:
	inline Context()
	{}

	inline TcpSession& session()
	{
		return m_session;
	}
	inline const TcpSession& get_session() const
	{
		return m_session;
	}

	inline TcpConnection& connection()
	{
		return m_session.connection();
	}
	inline const TcpConnection& get_connection() const
	{
		return m_session.get_connection();
	}

	inline const uint32_t get_type() const
	{
		return m_meta.m_message_type;
	}

	inline const uint32_t last() const
	{
		return m_meta.last();
	}

	inline const uint32_t get_req4() const
	{
		return m_meta.get_req4();
	}
	inline const uint64_t get_req8() const
	{
		return m_meta.get_req8();
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

	inline void async_response(
		IN Codec& codec,
		IN const uint32_t type,
		IN const bool last = true,
		IN const bool encrypted = true)
	{
		m_meta.m_session_id = m_session.get_id();
		m_session.async_response(codec, type, *this, last, encrypted);
	}
};


}}
////////////////////////////////////////////////////////////////////////////////
#endif