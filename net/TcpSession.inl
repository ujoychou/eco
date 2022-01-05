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


ECO_NS_BEGIN(eco);
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

	void close();

	void response(MessageMeta& meta, const Context& c);

	friend class TcpSession;
	TcpSessionImpl* m_impl;
};


////////////////////////////////////////////////////////////////////////////////
void TcpSession::close()
{
	TcpSessionInner inner(m_impl);
	return inner.close();
}
bool TcpSession::authorized() const
{
	return true;
}
TcpConnection& TcpSession::connection() const
{
	return m_impl.m_conn;
}
SessionData::ptr TcpSession::data() const
{
	return m_impl.m_session_wptr.lock();
}
const SessionId TcpSession::id() const
{
	return m_impl.m_session_id;
}
template<typename SessionDataT>
inline std::shared_ptr<SessionDataT> TcpSession::cast()
{
	auto sess = m_impl.m_session_wptr.lock();
	if (sess == nullptr)
	{
		ECO_THROW(e_session_expired)
			<< "cast data fail, session data has expired.";
	}
	return std::dynamic_pointer_cast<SessionDataT>(sess);
}


////////////////////////////////////////////////////////////////////////////////
void TcpSession::response(MessageMeta& meta, const Context& c)
{
	TcpSessionInner inner(m_impl);
	return inner.response(meta, c);
}
void TcpSession::send(IN const MessageMeta& meta)
{
	if (m_impl.m_session_id != none_session)
	{
		SessionData::ptr lock = m_impl.m_session_wptr.lock();
		if (lock != nullptr)
		{
			MessageMeta& m = const_cast<MessageMeta&>(meta);
			m.session_id(m_impl.m_session_id);
			m_impl.m_conn.send(meta);
		}
		return;
	}
	connection().send(meta);
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif