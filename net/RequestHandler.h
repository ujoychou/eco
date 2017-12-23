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
#include <eco/net/TcpPeer.h>
#include <eco/net/TcpSession.h>
#include <eco/net/RequestFilter.h>
#include <eco/log/Log.h>


namespace eco{;
namespace net{;



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
class RequestHandler
{
public:
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

	// context that recv from remote peer.
	inline TcpSession& session()
	{
		return m_context.m_session;
	}
	inline const TcpSession& get_session() const
	{
		return m_context.m_session;
	}

	// context that recv from remote peer.
	inline TcpPeer& peer() const
	{
		return m_context.m_session.get_peer();
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

	// response message to the request.
	inline void response(
		IN eco::net::Codec& codec,
		IN uint16_t iid,
		IN MessageCategory category = category_general)
	{
		response(codec, iid, false, category);
	}

	// response message to the request.
	inline void response(
		IN eco::net::Codec& codec,
		IN uint16_t iid,
		IN bool last,
		IN MessageCategory category = category_general)
	{
		eco::net::MessageMeta meta;
		meta.m_category = category;
		meta.m_model = model_rsp;
		meta.m_codec = &codec;
		meta.m_request_id = m_context.m_request_id;
		eco::set(meta.m_option, opt_last, last);
		if (eco::has(m_context.m_category, category_sync_mode))
		{
			eco::add(meta.m_category, category_sync_mode);
		}
		else
		{
			meta.set_message_type(iid);
		}
		m_context.m_session.send(meta);
	}

private:
	Request m_request;
	Context m_context;
};


////////////////////////////////////////////////////////////////////////////////
#define ECO_HANDLER(request_type, request_type_name, is_logging, filter_value)\
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
		eco::net::RequestFilter f(filter_value);\
		return f;\
	}\
	inline static bool auto_logging()\
	{\
		return is_logging;\
	}


////////////////////////////////////////////////////////////////////////////////
}}
#endif