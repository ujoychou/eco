#ifndef ECO_NET_DISPATCH_REGISTRY_H
#define ECO_NET_DISPATCH_REGISTRY_H
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
#include <eco/Type.h>
#include <eco/net/RequestHandler.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
// message handled by "handler", suguest using in server.
template<typename HandlerT>
inline void handle_context(IN Context& c)
{
	const TcpConnection& conn = c.m_session.get_connection();

	// 1.filter request
	const char* e = nullptr;
	// sessesion auth or session request after authed.
	if (HandlerT::auth_flag() > 0)
	{
		if (eco::has(c.m_meta.m_category, category_session))
		{
			if (HandlerT::auth_flag() && !c.m_session.authed())
				e = "session isn't authed when recv";
			if (!HandlerT::auth_flag() && c.m_session.authed())
				e = "session is authed when recv";
		}
		else
		{
			if (HandlerT::auth_flag() && !conn.authed())
				e = "connection isn't authed when recv";
			if (!HandlerT::auth_flag() && conn.authed())
				e = "connection is authed when recv";
		}
	}
	
	if (e != nullptr)
	{
		EcoError(eco::net::req) << Log(c.m_session,
			c.m_meta.m_message_type, HandlerT::name()) <= e;
		return;
	}
	
	try
	{
		// 2.decode message by a newer handler.
		// heap is used to be passed by deriving from "enable_shared_from_this".
		std::shared_ptr<HandlerT> hdl(new HandlerT);
		if (!hdl->on_decode(c.m_message.m_data, c.m_message.m_size))
		{
			EcoError(eco::net::req) << Log(c.m_session, c.m_meta.m_message_type,
				HandlerT::name()) <= "decode message fail";
			return;
		}
		// release io raw data to save memory.
		c.release_data();
		hdl->context() = std::move(c);

		// 3.handle request.
		hdl->on_request();
	} 
	catch (eco::Error& e) {
		EcoError << e;
	}
	catch (std::exception& e) {
		EcoLogStr(error, 0) << e.what();
	}
}


////////////////////////////////////////////////////////////////////////////////
// message handled by "functor", suguest using in client.
template<typename MessageT, typename CodecT>
inline void handle_context(
	IN std::function<void(IN MessageT&, IN Context&)>& func,
	IN Context& c)
{
	CodecT codec;
	MessageT object;
	codec.set_message(&object);
	codec.decode(c.m_message.m_data, c.m_message.m_size);
	func(object, c);
}

// message array handled by "functor", suguest using in client.
#define ECO_HANDLE_CONTEXT_ARRAY_FUNC(MessageT) \
std::function<void(std::vector<std::shared_ptr<MessageT>>&, Context&)>
// 
template<typename MessageT, typename CodecT>
inline void handle_context_array(
	IN ECO_HANDLE_CONTEXT_ARRAY_FUNC(MessageT)& func,
	IN Context& c)
{
	static std::vector<MessageT*> s_message_set;

	// 1.deccode message.
	CodecT codec;
	MessageT* msg(new MessageT);
	codec.set_message(msg);
	codec.decode(&c.m_message);
	s_message_set.push_back(msg);

	// 2.handle message.
	if (eco::has(c.m_option, opt_last))
	{
		// 3.release message object.
		func(s_message_set, c);
		auto it = s_message_set.begin();
		for (; it != s_message_set.end(); ++it){
			delete *it;
		}
		s_message_set.clear();
	}
}

inline void handle_context_default(
	IN std::function<void(IN eco::Bytes&, IN Context&)>& func,
	IN Context& c)
{
	func(c.m_message, c);
}


////////////////////////////////////////////////////////////////////////////////
#define ECO_REGISTER_HANDLER(disp, msg_type, msg, codec, func, obj_ptr) \
disp.register_handler<msg, codec>(msg_type,\
std::bind(&func, obj_ptr, std::placeholders::_1, std::placeholders::_2));

#define ECO_REGISTER_FUNCTOR(disp, msg_type, msg, codec, func) \
disp.register_handler<msg, codec>(msg_type,\
std::bind(&func, std::placeholders::_1, std::placeholders::_2));


////////////////////////////////////////////////////////////////////////////////
class ECO_API DispatchRegistry
{
public:
	typedef std::function<void(IN Context&)> HandlerFunc;

	virtual ~DispatchRegistry() {}

	/*@ register message and message handler.*/
	virtual void register_handler(
		IN const uint64_t id, 
		IN HandlerFunc hdl) = 0;

	/*@ register message default handler.*/
	virtual void register_default_handler(
		IN HandlerFunc hdl) = 0;

	/*@ register message and message handler, and message dedicated by typeid.
	* @ tmpl.message: message type to be registered.
	* @ tmpl.Handler: to handle "tmpl.message" type.
	*/
	template<typename HandlerT>
	inline void register_handler()
	{
		register_handler(HandlerT::request_type(),
			std::bind(&handle_context<HandlerT>, std::placeholders::_1));
	}

	/*@ register default message handler to process unregistered message type.
	* @ tmpl.Handler: default message handler.
	*/
	template<typename HandlerT>
	inline void register_default()
	{
		register_default_handler(
			std::bind(&handle_context<HandlerT>, std::placeholders::_1));
	}

	/*@ register message and message handler function.*/
	template<typename MessageT, typename CodecT>
	inline void register_handler(
		IN uint64_t id,
		IN std::function<void(IN MessageT&, IN Context&)> func)
	{
		register_handler(id, std::bind(&handle_context<MessageT, CodecT>,
			func, std::placeholders::_1));
	}

	/*@ register message default handler func.*/
	inline void register_default(
		IN std::function<void(IN eco::Bytes&, IN Context&)> func)
	{
		register_default_handler(std::bind(&handle_context_default, 
			func, std::placeholders::_1));
	}

	/*@ register message and message handler function.*/
	template<typename MessageT, typename CodecT>
	inline void register_handler_array(
		IN uint64_t id,
		IN ECO_HANDLE_CONTEXT_ARRAY_FUNC(MessageT) func)
	{
		register_handler(id, std::bind(&handle_context_array<MessageT, CodecT>,
			func, std::placeholders::_1));
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif