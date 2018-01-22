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
	bool session_mode() const;
	void async_send(IN MessageMeta& meta);
	void async_auth(IN MessageMeta& meta);
	void async_resp(
		IN Codec& codec,
		IN const uint32_t type,
		IN const Context& c,
		IN const bool last);

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

SessionData::ptr TcpSession::data()
{
	return m_impl.m_session_wptr.lock();
}

template<typename SessionDataT>
inline std::shared_ptr<SessionDataT> TcpSession::cast()
{
	return std::dynamic_pointer_cast<SessionDataT>(data());
}

const SessionId TcpSession::get_id() const
{
	return m_impl.m_session_id;
}

bool TcpSession::session_mode() const
{
	TcpSessionInner inner(m_impl);
	return inner.session_mode();
}

void TcpSession::async_resp(
	IN Codec& codec,
	IN const uint32_t type,
	IN const Context& context,
	IN const bool last)
{
	TcpSessionInner inner(m_impl);
	return inner.async_resp(codec, type, context, last);
}

void TcpSession::async_send(IN MessageMeta& meta)
{
	TcpSessionInner inner(m_impl);
	return inner.async_send(meta);
}

void TcpSession::async_auth(IN MessageMeta& meta)
{
	TcpSessionInner inner(m_impl);
	return inner.async_auth(meta);
}

////////////////////////////////////////////////////////////////////////////////
}}
#endif