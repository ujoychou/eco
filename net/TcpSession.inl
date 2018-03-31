#ifndef ECO_NET_TCP_SESSION_INL
#define ECO_NET_TCP_SESSION_INL
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


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpSessionInner
{
private:
	inline TcpSessionInner(TcpSessionImpl& impl) : m_impl(&impl)
	{}

	inline TcpSessionInner(IN const TcpSessionImpl& impl)
		: m_impl((TcpSessionImpl*)&impl)
	{}

	inline TcpSessionImpl& impl()
	{
		return *m_impl;
	}
	inline const TcpSessionImpl& impl() const
	{
		return *m_impl;
	}

	bool auth();
	void close();
	void async_auth(IN const MessageMeta& meta);
	void async_response(
		IN Codec& codec,
		IN const uint32_t type,
		IN const Context& c,
		IN const bool last,
		IN const bool encrypted);

	friend class TcpSession;
	TcpSessionImpl* m_impl;
};


//##############################################################################
//##############################################################################
void TcpSession::close()
{
	TcpSessionInner inner(m_impl);
	return inner.close();
}

bool TcpSession::auth()
{
	TcpSessionInner inner(m_impl);
	return inner.auth();
}

bool TcpSession::authed() const
{
	return !m_impl.m_session_wptr.expired();
}

TcpConnection& TcpSession::connection()
{
	return m_impl.m_conn;
}

const TcpConnection& TcpSession::get_connection() const
{
	return m_impl.m_conn;
}

SessionData::ptr TcpSession::data() const
{
	return m_impl.m_session_wptr.lock();
}

template<typename SessionDataT>
inline std::shared_ptr<SessionDataT> TcpSession::cast()
{
	auto sess = m_impl.m_session_wptr.lock();
	if (sess == nullptr)
	{
		EcoThrow(e_session_expired)
			<< "cast data fail, session data has expired.";
	}
	return std::dynamic_pointer_cast<SessionDataT>(sess);
}

const SessionId TcpSession::get_id() const
{
	return m_impl.m_session_id;
}


////////////////////////////////////////////////////////////////////////////////
void TcpSession::async_response(
	IN Codec& codec,
	IN const uint32_t type,
	IN const Context& context,
	IN const bool last,
	IN const bool encrypted)
{
	TcpSessionInner inner(m_impl);
	return inner.async_response(codec, type, context, last, encrypted);
}
void TcpSession::async_auth(IN const MessageMeta& meta)
{
	TcpSessionInner inner(m_impl);
	return inner.async_auth(meta);
}


////////////////////////////////////////////////////////////////////////////////
void TcpSession::async_send(IN const MessageMeta& meta)
{
	if (m_impl.m_session_id != none_session)
	{
		SessionData::ptr lock = m_impl.m_session_wptr.lock();
		if (lock != nullptr)
		{
			MessageMeta& m = const_cast<MessageMeta&>(meta);
			m.set_session_id(m_impl.m_session_id);
			m_impl.m_conn.async_send(meta);
		}
	}
	else
	{
		connection().async_send(meta);
	}
}

////////////////////////////////////////////////////////////////////////////////
}}
#endif