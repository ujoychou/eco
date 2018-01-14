#ifndef ECO_NET_REQUEST_HANDLER_H
#define ECO_NET_REQUEST_HANDLER_H
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
#include <eco/net/Context.h>
#include <eco/net/TcpSession.h>
#include <eco/net/TcpConnection.h>
#include <eco/net/RequestFilter.h>
#include <eco/log/Log.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class MessageHandler : public eco::Object<MessageHandler>
{
public:
	virtual ~MessageHandler() = 0 {}
};


////////////////////////////////////////////////////////////////////////////////
template <typename RequestHandler>
class RequestLogging
{
public:
	inline RequestLogging(RequestHandler& h) : m_handler(h)
	{}

	inline RequestLogging& operator>>(OUT eco::log::LogStream& logger)
	{
		m_handler.on_logging(logger);
		return *this;
	}

private:
	RequestHandler& m_handler;
};


////////////////////////////////////////////////////////////////////////////////
template<typename Request>
class RequestHandler : public MessageHandler
{
public:
	typedef RequestHandler<Request> Handler;

	// decode message from bytes string.
	virtual bool on_decode(
		IN const char* bytes,
		IN const uint32_t size) = 0;

	// format request logging.
	virtual void on_logging(
		OUT eco::log::LogStream& logger) const
	{}

	// receiving request notify, and handle request object.
	virtual void on_request()
	{}

public:
	typedef RequestHandler<Request> Handler;
	typedef RequestLogging<Handler> Logging;
	friend class RequestLogging<Handler>;

	// context that recv from client.
	inline Context& context()
	{
		return m_context;
	}
	inline const Context& get_context() const
	{
		return m_context;
	}

	// request that recv from remote peer.
	inline Request& request()
	{
		return m_request;
	}
	inline const Request& get_request() const
	{
		return m_request;
	}

	// context that recv from remote peer.
	inline TcpSession& session() const
	{
		return (TcpSession&)m_context.m_session;
	}

	// context that recv from remote peer.
	inline TcpConnection& connection() const
	{
		return session().connection();
	}

	// response message to the request.
	inline void async_resp(
		IN Codec& codec, 
		IN const uint32_t type,
		IN bool last = true)
	{
		if (session().session_mode())
			session().async_resp(codec, type, m_context, last);
		else
			connection().async_resp(codec, type, m_context, last);
	}

public:
	template<typename Handler>
	inline static std::shared_ptr<Handler> cast(
		IN eco::net::MessageHandler::ptr& v)
	{
		return std::dynamic_pointer_cast<Handler>(v);
	}

	template<typename Handler>
	inline static eco::net::MessageHandler::ptr cast(IN Handler* v)
	{
		return std::dynamic_pointer_cast<Handler>(v->shared_from_this());
	}

protected:
	Request m_request;
	Context m_context;
};


////////////////////////////////////////////////////////////////////////////////
#define ECO_HANDLER(request_type, request_type_name, auto_log, filter_authed)\
public:\
	inline static uint64_t get_request_type()\
	{\
		return request_type;\
	}\
	inline static const char* get_request_type_name()\
	{\
		return request_type_name;\
	}\
	inline static eco::net::RequestFilter get_filter()\
	{\
		eco::net::RequestFilter f;\
		f.set_authed(filter_authed);\
		return f;\
	}\
	inline static bool auto_logging()\
	{\
		return auto_log;\
	}


////////////////////////////////////////////////////////////////////////////////
}}
#endif