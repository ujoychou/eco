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
#include <eco/net/Context.h>
#include <eco/net/TcpSession.h>
#include <eco/net/TcpConnection.h>
#include <eco/net/Log.h>
#include <eco/net/Ecode.h>
#include <eco/proto/Proto.h>
#include <eco/thread/Map.h>
#include <eco/Eco.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class MessageHandler : public std::enable_shared_from_this<MessageHandler>
{
	ECO_OBJECT(MessageHandler);
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
			: eco::net::Log(c, c.get_type(), name)
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

	enum AuthMode
	{
		auth_none		= -1,
		unauthorized	=  0,
		authorized		=  1,
	};

public:
	inline MessageHandler() {}
	virtual ~MessageHandler() = 0 {}

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

public:
	// post this handler to aysnc map.
	inline uint32_t post_async()
	{
		return eco().post_async(shared_from_this());
	}

	// async management post item.
	static inline uint32_t post_async(IN MessageHandler::ptr& hdl)
	{
		return eco().post_async(hdl);
	}

	// has async message handler.
	static inline bool has_async(IN uint32_t req_id)
	{
		return eco().has_async(req_id);
	}

	// erase async handler.
	static inline void erase_async(IN uint32_t req_id)
	{
		return eco().erase_async(req_id);
	}

	// pop async message handler.
	static inline MessageHandler::ptr pop_async(
		IN uint32_t req_id,	IN bool last = true)
	{
		return eco().pop_async(req_id, last);
	}

protected:
	Context m_context;
};


////////////////////////////////////////////////////////////////////////////////
template<typename request_t, typename handler_t>
class RequestHandler : public MessageHandler
{
public:
	typedef RequestHandler<request_t, handler_t> Handler;

	// decode message from bytes string.
	virtual bool on_decode(
		IN const char* bytes,
		IN const uint32_t size) = 0;

	// receiving request notify, and handle request object.
	virtual void on_request()
	{}

public:
	inline RequestHandler()
	{
		eco::make(m_request);
	}

	// request that recv from remote peer.
	// request object type.
	typedef typename eco::Raw<request_t>::reference reference;
	// same with: inline auto& request()
	inline reference& request()
	{
		return eco::object(m_request);
	}
	// same with: inline const auto& get_request() const
	inline const reference& get_request() const
	{
		return  eco::get_object(m_request);
	}

	// request value type.
	inline request_t& value()
	{
		return m_request;
	}
	inline const request_t& get_value()
	{
		return m_request;
	}

	// response message to the request.
	inline void response(
		IN Codec& codec, 
		IN const uint32_t type,
		IN const bool last = true,
		IN const bool encrypted = false)
	{
		context().response(codec, type, last, encrypted);
	}

	// response message to the request.
	inline void response(
		IN Codec& codec,
		IN const bool last = true,
		IN const bool encrypted = false)
	{
		auto type = get_response_type();
		context().response(codec, type, last, encrypted);
	}

	// response message auto the request.
	template<typename rsp_t>
	inline static void async_response(
		IN rsp_t& rsp,
		IN uint32_t req_id,
		IN bool last = true,
		IN const bool encrypted = false,
		IN const bool logging = true)
	{
		if (rsp.has_error())
		{
			eco::win::gbk_to_utf8(*rsp.mutable_error()->mutable_message());
		}
		auto h = pop_async(req_id, last);
		if (h == 0 && logging)
		{
			ECO_LOG(warn)(eco::net::rsp) << eco::net::Log(nullptr, 
				handler_t::response_type(), handler_t::name());
			return;
		}
		h->response(rsp, last, encrypted);
		if (logging) ECO_RSP(debug, rsp, *h);
	}

public:
	// handler_t method.
	typedef std::shared_ptr<handler_t> handler_ptr;
	inline handler_ptr shared_from_this()
	{
		return std::dynamic_pointer_cast<handler_t>(
			MessageHandler::shared_from_this());
	}

	inline static handler_ptr find_async(
		uint32_t req_id, bool last = true, bool logging = true)
	{
		auto hdl = eco::net::MessageHandler::pop_async(req_id, last);
		if (hdl == 0 && logging)
		{
			ECO_LOG(warn)(eco::net::rsp) << eco::net::Log(
				nullptr, handler_t::response_type(), handler_t::name());
		}
		return std::dynamic_pointer_cast<handler_t>(hdl);
	}

	inline static handler_ptr pop_async(
		uint32_t req_id, bool last = true, bool logging = true)
	{
		auto hdl = find_async(req_id, last, logging);
		if (hdl == 0) { ECO_THROW(eco::net::e_server_handler_error); }
		return hdl;
	}

	// get name of this class type.
	inline static const char* name()
	{
		static const char* v = eco::clss(typeid(handler_t).name());
		return v; 
	}
	virtual const char* get_name() const override
	{
		return name();
	}
	inline static int auth() { return authorized; }

protected:
	request_t m_request;
};


////////////////////////////////////////////////////////////////////////////////
/* message handler macro define.
@auth_v: message recv when user must have been authed, set 1 else set 0.
if don't need to check authorized, set -1.
*/
#define ECO_OPTION(auth_v) public: inline static int auth() { return auth_v; }
#define ECO_HANDLE_1(req_type) ECO_HANDLE_2(req_type, 0)
#define ECO_HANDLE_2(req_type, rsp_type)\
public:\
inline static const uint32_t request_type() { return req_type;}\
inline static const uint32_t response_type(){ return rsp_type;}\
virtual const uint32_t get_response_type() const override { return rsp_type; }
#define ECO_HANDLE_3(req_type, rsp_type, name_v)\
ECO_HANDLE_2(req_type, rsp_type)\
inline static const char* name() { return name_v; }\
virtual const char* get_name() const override { return name_v;}
#define ECO_HANDLE_4(req_type, rsp_type, name_v, auth_v)\
ECO_OPTION(auth_v)\
ECO_HANDLE_3(req_type, rsp_type, name_v)
#define ECO_HANDLE(...) ECO_MACRO(ECO_HANDLE_,__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
}}
#endif