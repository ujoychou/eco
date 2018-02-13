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
#include <eco/net/Log.h>
#include <eco/proto/Proto.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
#define ECO_LOG_REQ(sev) \
EcoLog(sev, eco::log::text_size)(eco::net::req) << Log(*this)
#define ECO_LOG_SUB(sev) \
EcoLog(sev, eco::log::text_size)(eco::net::sub) << Log(*this)
#define ECO_LOG_RSP(sev, rsp, handler) \
if (rsp.has_error()){ \
	EcoError(eco::net::rsp) << MessageHandler::Log(handler, response_type()) \
	<= rsp.error(); \
}else \
	EcoLog(sev, eco::log::text_size)(eco::net::rsp) \
	<< MessageHandler::Log(handler, response_type())


////////////////////////////////////////////////////////////////////////////////
class MessageHandler : public eco::Object<MessageHandler>
{
public:
	// logging.
	class Log : public eco::net::Log
	{
	public:
		inline Log(
			IN const MessageHandler& h,
			IN const uint32_t type = 0)
			: eco::net::Log(h.session(), 
				(type == 0 ? h.get_request_type() : type), h.get_name())
		{}

		inline Log(
			IN const MessageHandler& h,
			IN const std::string& user_name) 
			: eco::net::Log(h.connection().get_id(), h.session().get_id(),
				h.get_request_type(), h.get_name(), user_name.c_str())
		{}

		inline Log(
			IN Context& c,
			IN const char* name,
			IN const uint32_t type = 0)
			: eco::net::Log(c, (type == 0 ? c.get_type() : type), name)
		{}

		inline Log(
			IN Context& c,
			IN const char* name,
			IN const std::string& user_name,
			IN const uint32_t type = 0)
			: eco::net::Log(c.connection().get_id(), c.session().get_id(),
				(type == 0 ? c.get_type() : type), name, user_name.c_str())
		{}
	};

public:
	virtual ~MessageHandler() = 0 
	{}

	// message name.
	virtual const char* get_name() const = 0;

	// message response type.
	virtual const uint32_t get_response_type() const
	{
		return 0;
	}

	// message type.
	inline const uint32_t get_request_type() const
	{
		return m_context.m_meta.m_message_type;
	}

	// context/session/connection receive from peer.
	inline Context& context()
	{
		return m_context;
	}
	inline const Context& get_context() const
	{
		return m_context;
	}
	inline TcpSession& session() const
	{
		return (TcpSession&)m_context.m_session;
	}
	inline TcpConnection& connection() const
	{
		return session().connection();
	}

protected:
	Context m_context;
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

	// receiving request notify, and handle request object.
	virtual void on_request()
	{}

public:
	// request that recv from remote peer.
	inline Request& request()
	{
		return m_request;
	}
	inline const Request& get_request() const
	{
		return m_request;
	}

	// response message to the request.
	inline void async_response(
		IN Codec& codec, 
		IN uint32_t type = 0,
		IN bool last = true)
	{
		if (type == 0)
			type = get_response_type();
		context().async_response(codec, type, last);
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
};


////////////////////////////////////////////////////////////////////////////////
#define ECO_HANDLER(req_type, rsp_type, type_name, is_authed)\
public:\
	inline static const uint32_t request_type()\
	{\
		return req_type;\
	}\
	inline static const uint32_t response_type()\
	{\
		return rsp_type;\
	}\
	virtual const uint32_t get_response_type() const override\
	{\
		return rsp_type;\
	}\
	inline static const char* name()\
	{\
		return type_name;\
	}\
	virtual const char* get_name() const override\
	{\
		return type_name; \
	}\
	inline static bool authed()\
	{\
		return is_authed; \
	}
	

////////////////////////////////////////////////////////////////////////////////
}}
#endif