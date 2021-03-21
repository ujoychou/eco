#ifndef ECO_NET_TCP_DISPATCH_H
#define ECO_NET_TCP_DISPATCH_H
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
#include <eco/Type.h>
#include <eco/net/RequestHandler.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
// message handled by "handler", suguest using in server.
template<typename handler_t>
inline void handle_context(IN Context& c)
{
	const TcpConnection& conn = c.m_session.connection();

	// 1.filter request
	const char* e = nullptr;
	// sessesion auth or session request after authed.
	if (handler_t::auth() != MessageHandler::auth_none)
	{
		if (eco::has(c.m_meta.m_category, category_session))
		{
			if (handler_t::auth() && !c.m_session.authorized())
				e = "session isn't authed when recv";
			if (!handler_t::auth() && c.m_session.authorized())
				e = "session is authed when recv";
		}
		else
		{
			if (handler_t::auth() && !conn.authorized())
				e = "connection isn't authed when recv";
			if (!handler_t::auth() && conn.authorized())
				e = "connection is authed when recv";
		}
	}
	if (e != nullptr)
	{
		ECO_ERROR << Log(c, handler_t::name())(req) <= e;
		return;
	}

	// 2.when app is unready, return.
	if (!eco::App::get()->ready())
	{
		ECO_ERROR << Log(c, handler_t::name())(req)
			<= "app_unready " < eco::this_thread::error();
		eco::this_thread::error().clear();
		return;
	}
	
	try
	{
		// 2.decode message by a newer handler.
		// heap is used to be passed by deriving from "enable_shared_from_this".
		std::shared_ptr<handler_t> hdl(std::make_shared<handler_t>());
		if (TypeId<handler_t::request_t>() != TypeId<eco::net::NullRequest>())
		{
			if (!hdl->on_decode(c.m_message.m_data, c.m_message.m_size))
			{
				ECO_ERROR << Log(c, handler_t::name())(req)
					<= "decode message fail.";
				return;
			}
			// release io raw data to save memory.
			c.release_data();
		}
		hdl->context() = std::move(c);

		// 3.handle request.
		hdl->to_request();
		hdl->on_request();
	} 
	catch (std::exception& e)
	{
		ECO_LOG(error, handler_t::name()) << e.what();
	}
}


////////////////////////////////////////////////////////////////////////////////
template<typename message_t, typename codec_t>
inline void handle_context(
	std::function<void(message_t&, Context&)>& func, Context& c)
{
	assert(!c.has_error());
	codec_t codec;
	message_t msg;
	codec.set_message(eco::make(msg));
	codec.decode(c.m_message.m_data, c.m_message.m_size);
	func(msg, c);
}


////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpDispatch
{
public:
	typedef std::function<void(IN Context&)> HandlerFunc;

	virtual ~TcpDispatch() {}

	/*@ register message and message handler.*/
	virtual void register_handler(IN int id, IN HandlerFunc&& hdl) = 0;

	/*@ register message default handler.*/
	virtual void register_default(IN HandlerFunc&& hdl) = 0;

public:
	/*@ register message and message handler, and message dedicated by typeid.
	* @ tmpl.message: message type to be registered.
	* @ tmpl.Handler: to handle "tmpl.message" type.
	*/
	template<typename Handler>
	inline void dispatch()
	{
		register_handler(Handler::req_type(), std::bind(
			&handle_context<Handler>, std::placeholders::_1));
	}

	/*@ register default message handler to process unregistered message type.
	* @ tmpl.Handler: default message handler.
	*/
	template<typename Handler>
	inline void dispatch_default()
	{
		register_default(std::bind(
			&handle_context<Handler>, std::placeholders::_1));
	}

	/*@ dispatch message without message object.*/
	inline void dispatch(IN int id, IN HandlerFunc hdl)
	{
		register_handler(id, std::move(hdl));
	}

	/*@ register message and message handler function.*/
	template<typename msg_t, typename codec_t>
	inline void dispatch(int id, std::function<void(msg_t&, Context&)> func)
	{
		register_handler(id, std::bind(&handle_context<rsp_t, codec_t>,
			func, std::placeholders::_1));
	}

	/*@ register message and message handler function.*/
	template<typename msg_t, typename codec_t, typename obj_t, typename func_t>
	inline void dispatch(IN int id, IN obj_t& obj, IN func_t func)
	{
		std::function<void(msg_t&, Context&)> wrap = std::bind(
			func, &obj, std::placeholders::_1, std::placeholders::_2);
		register_handler(id, std::bind(&handle_context<msg_t, codec_t>,
			wrap, std::placeholders::_1));
	}

#ifndef ECO_NO_PROTOBUF
	template<typename msg_t>
	inline void dispatch(int id, std::function<void(msg_t&, Context&)> func)
	{
		register_handler(id, std::bind(&handle_context<msg_t, ProtobufCodec>,
			func, std::placeholders::_1));
	}

	template<typename msg_t, typename obj_t, typename func_t>
	inline void dispatch(IN int id, IN obj_t& obj, IN func_t func)
	{
		dispatch<msg_t, ProtobufCodec, obj_t, func_t>(id, obj, func);
	}
#endif
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif