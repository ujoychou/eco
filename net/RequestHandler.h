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
#define ECO_LOG_REQ EcoInfo(eco::net::req) << Log(this)

#define ECO_LOG_RSP(rsp, hdl, type) \
if (rsp.has_error()){ \
EcoError(eco::net::rsp) << MessageHandler::Log(hdl, type) << rsp.error(); \
}else \
EcoInfo(eco::net::rsp) << MessageHandler::Log(hdl, type)


////////////////////////////////////////////////////////////////////////////////
class MessageHandler : public eco::Object<MessageHandler>
{
public:
	// logging.
	class Log : public eco::net::Log
	{
	public:
		inline Log(
			IN const MessageHandler* handler,
			IN const uint32_t type = 0) : eco::net::Log(handler->session(),
				type == 0 ? handler->get_request_type() : type,
				handler->get_name())
		{}

		inline Log(
			IN const MessageHandler& handler,
			IN const uint32_t type = 0) : eco::net::Log(handler.session(),
				type == 0 ? handler.get_request_type() : type,
				handler.get_name())
		{}

		inline Log(
			IN const MessageHandler* handler,
			IN const std::string& user_name) : eco::net::Log(
				handler->connection().get_id(), handler->session().get_id(),
				handler->get_request_type(),
				handler->get_name(), user_name.c_str())
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
	inline void async_resp(
		IN Codec& codec, 
		IN uint32_t type = 0,
		IN bool last = true)
	{
		if (type == 0) type = get_response_type();

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
};


////////////////////////////////////////////////////////////////////////////////
#define ECO_HANDLER(req_type, req_name, is_authed)\
public:\
	inline static const uint32_t request_type()\
	{\
		return req_type;\
	}\
	inline static const char* name()\
	{\
		return req_name;\
	}\
	inline static bool authed()\
	{\
		return is_authed;\
	}\
	virtual const char* get_name() const override\
	{\
		return req_name; \
	}

#define ECO_RESPONSE(rsp_type)\
public:\
	virtual const uint32_t get_response_type() const override\
	{\
		return rsp_type;\
	}\
	inline static const uint32_t response_type()\
	{\
		return rsp_type;\
	}

////////////////////////////////////////////////////////////////////////////////
}}
#endif