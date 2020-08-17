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
#include <eco/Eco.h>
#include <eco/net/Log.h>
#include <eco/net/Ecode.h>
#include <eco/net/Context.h>
#include <eco/loc/Locale.h>
#include <eco/thread/Map.h>
#include <eco/proto/Proto.h>


namespace eco{;
namespace net{;
class NullRequest {};
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
			IN MessageHandler& h,
			IN const uint32_t type = 0)
			: eco::net::Log(h.connection(),
			(type ? type : h.request_type()), h.get_name())
		{}

		inline Log(
			IN MessageHandler& h,
			IN const std::string& user) 
			: eco::net::Log(h.connection().id(), h.session().id(),
				h.request_type(), h.get_name(), user.c_str())
		{}

		inline Log(
			IN Context& c,
			IN const char* name,
			IN const uint32_t type = 0)
			: eco::net::Log(c, c.type(), name)
		{}

		inline Log(
			IN Context& c,
			IN const char* name,
			IN const std::string& user,
			IN const uint32_t type = 0)
			: eco::net::Log(c.connection().id(), c.m_session.id(),
				(type == 0 ? c.type() : type), name, user.c_str())
		{}

		inline Log& operator()(uint8_t mode)
		{
			m_mode = mode;
			return *this;
		}
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

	// decode message from bytes string.
	inline bool on_decode(
		IN const char* bytes, IN uint32_t size);

	// default option.
	inline int to_request() {}

	// receiving request notify, and handle request object.
	virtual void on_request() {}

	// default option.
	inline static int auth()
	{
		return auth_none;
	}

	// default logging severity.
	inline static int req_sev()
	{
		return eco::log::debug;
	}
	inline static int rsp_sev()
	{
		return eco::log::debug;
	}
	inline static int err_sev()
	{
		return eco::log::error;
	}

	// message name.
	virtual const char* get_name() = 0;

	// message type.
	inline const uint32_t request_type()
	{
		return m_context.m_meta.m_message_type;
	}

	// context/session/connection receive from peer.
	inline Context& context()
	{
		return m_context;
	}
	inline TcpSession& session()
	{
		return m_context.m_session;
	}
	inline TcpConnection& connection()
	{
		return session().connection();
	}

public:
	// post this handler to aysnc map.
	inline uint32_t post_async()
	{
		return Eco::get().post_async(shared_from_this());
	}

	// async management post item.
	static inline uint32_t post_async(IN MessageHandler::ptr& hdl)
	{
		return Eco::get().post_async(hdl);
	}

	// has async message handler.
	static inline bool has_async(IN uint32_t req_id)
	{
		return Eco::get().has_async(req_id);
	}

	// erase async handler.
	static inline void erase_async(IN uint32_t req_id)
	{
		return Eco::get().erase_async(req_id);
	}

	// pop async message handler.
	static inline MessageHandler::ptr pop_async(
		IN uint32_t req_id,	IN bool last = true)
	{
		return Eco::get().pop_async(req_id, last);
	}

protected:
	Context m_context;
};


////////////////////////////////////////////////////////////////////////////////
template<typename request_t, typename handler_t>
class RequestHandler : public MessageHandler
{
public:
	typedef request_t request_t;
	typedef std::shared_ptr<handler_t> ptr;
	
	inline RequestHandler()
	{
		eco::make(m_request);
	}

	// get name of this class type.
	inline static const char* name()
	{
		static const char* v = eco::clss(typeid(handler_t).name());
		return v;
	}

	virtual const char* get_name() override
	{
		return name();
	}

	// request that recv from remote peer.
	// request object type.
	typedef typename eco::Raw<request_t>::reference reference;
	typedef typename eco::Raw<request_t>::const_reference const_reference;
	// same with: inline auto& request()
	inline reference& request()
	{
		return eco::get_object(m_request);
	}

	// request value type.
	inline request_t& request_value()
	{
		return m_request;
	}

	// handler_t method.
	typedef std::shared_ptr<handler_t> handler_ptr;
	inline handler_ptr shared_from_this()
	{
		return std::dynamic_pointer_cast<handler_t>(
			MessageHandler::shared_from_this());
	}

	inline static handler_ptr pop_async(uint32_t req_id, bool last = true)
	{
		auto hdl = eco::net::MessageHandler::pop_async(req_id, last);
		if (hdl == 0)
		{
			auto type = handler_t::response_type();
			ECO_LOG(warn) << eco::net::Log(0, type, handler_t::name())(rsp);
		}
		return std::dynamic_pointer_cast<handler_t>(hdl);
	}

protected:
	request_t m_request;
};


////////////////////////////////////////////////////////////////////////////////
/* message handler macro define.
@auth_v: message recv when user must have been authed, set 1 else set 0.
if don't need to check authorized, set -1.
*/
#define ECO_HANDLE_OPTION(auth_v) \
public: inline static int auth() { return auth_v; }

#define ECO_HANDLE_LOGGING(req_sev_, rsp_sev_, err_sev_) \
public:\
inline static int req_sev() { return eco::log::##req_sev_; }\
inline static int rsp_sev() { return eco::log::##rsp_sev_; }\
inline static int err_sev() { return eco::log::##err_sev_; }

#define ECO_HANDLE_1(req_type) ECO_HANDLE_2(req_type, 0)
#define ECO_HANDLE_2(req_type, rsp_type)\
public:\
inline static const uint32_t request_type() { return req_type;}\
inline static const uint32_t response_type(){ return rsp_type;}
#define ECO_HANDLE_3(req_type, rsp_type, name_v)\
ECO_HANDLE_2(req_type, rsp_type)\
inline static const char* name() { return name_v; }\
virtual const char* get_name() override { return name_v;}
#define ECO_HANDLE_4(req_type, rsp_type, name_v, auth_v)\
ECO_HANDLE_OPTION(auth_v)\
ECO_HANDLE_3(req_type, rsp_type, name_v)
#define ECO_HANDLE(...) ECO_MACRO(ECO_HANDLE_,__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
}}
#endif