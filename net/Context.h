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


namespace eco{;
namespace net{;
class TcpPeer;
typedef std::weak_ptr<TcpPeer> TcpPeerWptr;
////////////////////////////////////////////////////////////////////////////////
class DataContext
{
	ECO_OBJECT(DataContext);
public:
	inline DataContext(IN TcpSessionHost* host = nullptr)
		: m_prot(nullptr)
		, m_prot_head(nullptr)
		, m_category(0)
	{
		if (host != nullptr)
		{
			m_session_host = *host;
		}
	}

	inline DataContext(IN DataContext&& v)
		: m_data(std::move(v.m_data))
		, m_category(v.m_category)
		, m_prot(v.m_prot)
		, m_prot_head(v.m_prot_head)
		, m_session_host(v.m_session_host)
		, m_peer_wptr(std::move(v.m_peer_wptr))
	{}

	inline DataContext& operator=(IN DataContext&& v)
	{
		m_data = std::move(v.m_data);
		m_category = v.m_category;
		m_prot = v.m_prot;
		m_prot_head = v.m_prot_head;
		m_session_host = v.m_session_host;
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
	ProtocolHead*		m_prot_head;
	TcpPeerWptr			m_peer_wptr;
	TcpSessionHost		m_session_host;
};


////////////////////////////////////////////////////////////////////////////////
class MetaContext
{
public:
	eco::String			m_data;
	MessageCategory		m_category;
	eco::Bytes			m_message;
	uint64_t			m_request_type;
	uint64_t			m_request_data;
	uint32_t			m_option;
	TcpSession			m_session;
	
public:
	inline MetaContext(
		IN DataContext& dc, 
		IN eco::Bytes& msg,
		IN MessageMeta& meta)
		: m_category(dc.m_category)
		, m_data(std::move(dc.m_data))
		, m_message(msg)
		, m_request_data(meta.m_request_data)
		, m_request_type(meta.m_message_type)
		, m_option(meta.m_option)
	{
		m_session.set_protocol(*dc.m_prot);
		m_session.set_host(dc.m_session_host);
	}

	inline void release()
	{
		m_data.release();
		m_message.clear();
		m_session.reset();
	}
};


////////////////////////////////////////////////////////////////////////////////
class Context
{
public:
	MessageCategory		m_category;
	uint32_t			m_option;
	uint64_t			m_request_data;
	TcpSession			m_session;

	inline Context()
		: m_category(0), m_option(0)
		, m_request_data(0), m_session(eco::null)
	{}

	inline void set_context(IN MetaContext& mc)
	{
		m_category = mc.m_category;
		m_option = mc.m_option;
		m_request_data = mc.m_request_data;
		m_session = mc.m_session;
	}
};


}}
////////////////////////////////////////////////////////////////////////////////
#endif