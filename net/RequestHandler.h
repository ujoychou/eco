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
#include <eco/thread/Eco.h>
#include <eco/net/Ecode.h>
#include <eco/net/Context.h>
#include <eco/loc/Locale.h>
#include <eco/thread/Map.h>
#include <eco/detail/proto/Proto.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
class NullRequest {};
////////////////////////////////////////////////////////////////////////////////
class MessageHandler;
class Async
{
	ECO_SINGLETON(Async);
public:
	// get next req id.
	inline uint32_t next_req_id()
	{
		// promise that: request_id != 0.
		if (++m_request_id == 0) { ++m_request_id; }
		return m_request_id;
	}

	// async management post item.
	inline uint32_t post_async(IN std::shared_ptr<MessageHandler>&& hdl)
	{
		// note: request_id != 0.
		uint32_t req_id = next_req_id();
		m_async_map.set(req_id, hdl);
		return req_id;
	}

	// has async message handler.
	inline bool has_async(IN uint32_t req_id)
	{
		return m_async_map.has(req_id);
	}

	// erase async handler.
	inline void erase_async(IN uint32_t req_id)
	{
		m_async_map.erase(req_id);
	}

	// pop async message handler.
	inline std::shared_ptr<MessageHandler> pop_async(
		IN uint32_t req_id, IN bool last)
	{
		int eid = 0;
		if (last) { return m_async_map.pop(req_id, eid); }

		std::shared_ptr<MessageHandler> handler;
		m_async_map.find(handler, req_id);
		return handler;
	}

private:
	std_atomic_uint32_t m_request_id;
	eco::HashMap<uint32_t, std::shared_ptr<MessageHandler>> m_async_map;
};


////////////////////////////////////////////////////////////////////////////////
class MessageHandler : public std::enable_shared_from_this<MessageHandler>
{
	ECO_OBJECT(MessageHandler);
protected:
	Context m_context;

public:
	// authorized data info.
	enum
	{
		auth_false	= 0,	// unauthorized.
		auth_true	= 1,	// authorized.
		auth_none	= 2,	// no need to authorize, all request can be passed.
	};
	typedef uint8_t AuthMode;

	inline MessageHandler() {}
	virtual ~MessageHandler() {}

	// decode message from bytes string.
	inline bool on_decode(
		IN const char* bytes, IN uint32_t size);

	// default option.
	inline void to_request() {}

	// receiving request notify, and handle request object.
	inline void on_request();

	// default option.
	virtual AuthMode get_auth()
	{
		return auth_none;
	}

	// default logging severity.
	virtual eco::log::SeverityLevel get_req_sev()
	{
		return req_sev();
	}
	virtual eco::log::SeverityLevel get_rsp_sev()
	{
		return rsp_sev();
	}
	virtual eco::log::SeverityLevel get_err_sev()
	{
		return err_sev();
	}

	// message name.
	virtual const char* get_name()
	{
		return eco::value_empty.c_str();
	}
	// request/response type.
	virtual uint32_t get_req_type()
	{
		return 0;
	}
	virtual uint32_t get_rsp_type()
	{
		return 0;
	}

	// handler implement those static method.
	inline static AuthMode auth()
	{
		return auth_none;
	}
	inline static eco::log::SeverityLevel rsp_sev()
	{
		return eco::log::debug;
	}
	inline static eco::log::SeverityLevel req_sev()
	{
		return eco::log::debug;
	}
	inline static eco::log::SeverityLevel err_sev()
	{
		return eco::log::error;
	}
	inline static uint32_t req_type()
	{
		return 0;
	}
	inline static uint32_t rsp_type()
	{
		return 0;
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
		return Async::get().post_async(shared_from_this());
	}

	// async management post item.
	static inline uint32_t post_async(IN MessageHandler::ptr&& hdl)
	{
		return Async::get().post_async(std::move(hdl));
	}

	// has async message handler.
	static inline bool has_async(IN uint32_t req_id)
	{
		return Async::get().has_async(req_id);
	}

	// erase async handler.
	static inline void erase_async(IN uint32_t req_id)
	{
		return Async::get().erase_async(req_id);
	}

	// pop async message handler.
	static inline MessageHandler::ptr pop_async(
		IN uint32_t req_id,	IN bool last = true)
	{
		return Async::get().pop_async(req_id, last);
	}
	
#ifndef ECO_NO_PROTOBUF
public:
	inline void send(
		IN int type,
		IN const google::protobuf::Message* msg,
		IN bool last_, IN bool err)
	{
		int sev = err ? get_err_sev() : get_rsp_sev();
		if (!last_)
			context().send(type, msg, false, err);
		else
			context().send(type, msg, get_name(), sev, err);
	}

	// response message to client.
	inline void resolve(
		IN const google::protobuf::Message* msg = nullptr,
		IN bool last_ = true)
	{
		send(get_rsp_type(), msg, last_, false);
	}
	// response message set to client.
	template<typename object_t, typename set_t>
	inline void resolve_set(IN const set_t& set)
	{
		if (set.empty()) return resolve(nullptr);
		for (auto it = set.begin(); it != set.end(); )
		{
			const object_t& obj = eco::object(*it);
			resolve(&obj, ++it == set.end());
		}
	}

	// response error message to client.
	inline void reject()
	{
		app_locale().parse_error("", connection().lang());
		reject(&eco::this_thread::proto::error());
	}
	inline void reject(eco::proto::Error& e, const char* mdl = "")
	{
		app_locale().parse_error(e, "", connection().lang());
		reject(&e);
	}
	inline void reject(uint32_t eid)
	{
		eco::Error().key(eid);
		reject();
	}
	inline void reject(const std::string& path_)
	{
		eco::Error().key(path_.c_str());
		reject();
	}
	inline void reject(IN const google::protobuf::Message* msg)
	{
		send(get_rsp_type(), msg, true, true);
	}

public:
	inline static void send(
		IN int async_id,
		IN const google::protobuf::Message* msg,
		IN int type, IN bool last_, IN bool err)
	{
		auto h = pop_async(async_id, last_);
		if (h) h->send(type, msg, last_, err);
	}
	inline static void resolve(
		IN int async_id,
		IN const google::protobuf::Message* msg = nullptr,
		IN bool last_ = true)
	{
		auto h = pop_async(async_id, last_);
		if (h) h->resolve(msg, last_);
	}
	inline static void reject(
		IN int async_id,
		IN const google::protobuf::Message* msg)
	{
		auto h = pop_async(async_id, true);
		if (h) h->reject(msg);
	}
	inline static void reject(
		IN int async_id,
		IN eco::proto::Error& e,
		IN const char* mdl = "")
	{
		auto h = pop_async(async_id, true);
		if (h) h->reject(e, mdl);
	}
	inline static void reject(IN int async_id, uint32_t eid)
	{
		auto h = pop_async(async_id, true);
		if (h) h->reject(eid);
	}
	inline static void reject(IN int async_id, const char* path)
	{
		auto h = pop_async(async_id, true);
		if (h) h->reject(path);
	}
#endif
};


////////////////////////////////////////////////////////////////////////////////
template<typename req_t, typename handler_t>
class RequestHandler : public MessageHandler
{
public:
	typedef req_t request_t;
	typedef std::shared_ptr<handler_t> ptr;
	
	inline RequestHandler()
	{
		eco::make(m_request);
	}

	// get name of this class type.
	inline static const char* name()
	{
		return s_handler_name;
	}
	virtual const char* get_name() override
	{
		return s_handler_name;
	}

	// request that recv from remote peer.
	// request object type.
	typedef typename eco::Raw<request_t>::reference reference;
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
	// request/response type.
	virtual uint32_t get_req_type() override
	{
		return handler_t::req_type();
	}
	virtual uint32_t get_rsp_type() override
	{
		return handler_t::rsp_type();
	}

	// logging severity level.
	virtual eco::log::SeverityLevel get_req_sev()
	{
		return handler_t::req_sev();
	}
	virtual eco::log::SeverityLevel get_rsp_sev()
	{
		return handler_t::rsp_sev();
	}
	virtual eco::log::SeverityLevel get_err_sev()
	{
		return handler_t::err_sev();
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
		return std::dynamic_pointer_cast<handler_t>(
			MessageHandler::pop_async(req_id, last));
	}

protected:
	request_t m_request;
	static const char* s_handler_name;
};

// default handler name.
template<typename request_t, typename handler_t>
const char* RequestHandler<request_t, handler_t>::s_handler_name
= eco::clss(typeid(handler_t).name());


////////////////////////////////////////////////////////////////////////////////
/* message handler macro define.
@auth_v: message recv when user must have been authed, set 1 else set 0.
if don't need to check authorized, set -1.
*/
#define ECO_HANDLE_OPTION(auth_v) \
public: inline static int auth() { return auth_v; }

#define ECO_HANDLE_LOG(req_sev_, rsp_sev_, err_sev_) \
public:\
inline static int req_sev() { return eco::log::##req_sev_; }\
inline static int rsp_sev() { return eco::log::##rsp_sev_; }\
inline static int err_sev() { return eco::log::##err_sev_; }

#define ECO_HANDLE_1(reqtype) ECO_HANDLE_2(reqtype, 0)
#define ECO_HANDLE_2(reqtype, rsptype)\
public:\
inline static const uint32_t req_type() { return reqtype;}\
inline static const uint32_t rsp_type() { return rsptype;}
#define ECO_HANDLE_3(reqtype, rsptype, name_v)\
ECO_HANDLE_2(reqtype, rsptype)\
inline static const char* name() { return name_v; }\
virtual const char* get_name() override { return name_v;}
#define ECO_HANDLE_4(reqtype, rsptype, name_v, auth_v)\
ECO_HANDLE_OPTION(auth_v)\
ECO_HANDLE_3(reqtype, rsptype, name_v)
#define ECO_HANDLE(...) ECO_MACRO(ECO_HANDLE_,__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
}}
#endif