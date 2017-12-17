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
////////////////////////////////////////////////////////////////////////////////
class TcpPeer;
class DataContext
{
	ECO_OBJECT(DataContext);
public:
	inline DataContext()
		: m_prot(nullptr)
		, m_prot_head(nullptr)
	{}

	inline DataContext(IN DataContext&& v)
		: m_data(std::move(v.m_data))
		, m_prot(v.m_prot)
		, m_prot_head(v.m_prot_head)
		, m_peer_wptr(std::move(v.m_peer_wptr))
	{}

	inline DataContext& operator=(IN DataContext&& v)
	{
		m_data = std::move(v.m_data);
		m_prot = v.m_prot;
		m_prot_head = v.m_prot_head;
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
	std::weak_ptr<TcpPeer>	m_peer_wptr;
};


////////////////////////////////////////////////////////////////////////////////
class MetaContext
{
public:
	eco::String			m_data;
	eco::Bytes			m_message;
	MessageCategory		m_category;
	uint32_t			m_request_id;
	uint64_t			m_request_type;
	TcpSession			m_session;
	
public:
	inline MetaContext() : m_category(0), m_request_id(0), m_request_type(0)
	{}

	inline void set_context(IN DataContext& dc, IN MessageMeta& m)
	{
		m_data.swap(dc.m_data);
		m_category = dc.m_category;
		m_request_id = m.m_request_id;
		m_request_type = m.m_message_type;
	}

	inline void clear()
	{
		m_data.clear();
		m_message.clear();
	}
};


////////////////////////////////////////////////////////////////////////////////
class Context
{
public:
	uint32_t			m_request_id;
	MessageCategory		m_category;
	TcpSession			m_session;

	inline Context() : m_category(0), m_request_id(0)
	{}

	inline void set_context(IN MetaContext& mc)
	{
		m_category = mc.m_category;
		m_request_id = mc.m_request_id;
	}
};


}}
////////////////////////////////////////////////////////////////////////////////
#endif