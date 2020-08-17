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
		ECO_ERROR << Log(conn, c.type(), handler_t::name())(req) <= e;
		return;
	}

	// 2.when app is unready, return.
	if (!eco::App::get()->ready())
	{
		ECO_ERROR << Log(conn, c.type(), handler_t::name())(req)
			<= "app system is unready.";
		return;
	}
	
	try
	{
		// 2.decode message by a newer handler.
		// heap is used to be passed by deriving from "enable_shared_from_this".
		std::shared_ptr<handler_t> hdl(new handler_t);
		if (TypeId<handler_t::request_t>() != TypeId<eco::net::NullRequest>())
		{
			if (!hdl->on_decode(c.m_message.m_data, c.m_message.m_size))
			{
				ECO_ERROR << Log(conn, c.type(), handler_t::name())(req)
					<= "decode message fail";
				return;
			}
		}
		// release io raw data to save memory.
		c.release_data();
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
// message handled by "functor", suguest using in client.
template<typename rsp_t>
inline void* handle_context_make(std::auto_ptr<rsp_t>& obj)
{
	obj.reset(new rsp_t());
	return obj.get();
}
template<typename rsp_t>
inline void* handle_context_make(std::shared_ptr<rsp_t>& obj)
{
	obj.reset(new rsp_t());
	return obj.get();
}
template<typename rsp_t>
inline void* handle_context_make(rsp_t& obj)
{
	return &obj;
}
template<typename rsp_t>
inline void* handle_context_make(rsp_t* obj)
{
	return new rsp_t();
}


////////////////////////////////////////////////////////////////////////////////
template<typename err_t, typename codec_t>
inline void handle_context(
	std::function<void(err_t*, Context&)>& func, Context& c)
{
	codec_t codec;
	if (c.has_error())
	{
		err_t err;
		codec.set_message(handle_context_make(err));
		codec.decode(c.m_message.m_data, c.m_message.m_size);
		func(&err, c);
		return;
	}
	func(nullptr, c);
}
template<typename err_t, typename rsp_t, typename codec_t>
inline void handle_context(
	std::function<void(err_t*, rsp_t*, Context&)>& func, Context& c)
{
	codec_t codec;
	if (c.has_error())
	{
		err_t err;
		codec.set_message(handle_context_make(err));
		codec.decode(c.m_message.m_data, c.m_message.m_size);
		func(&err, nullptr, c);
	}
	else
	{
		rsp_t obj;
		codec.set_message(handle_context_make(obj));
		codec.decode(c.m_message.m_data, c.m_message.m_size);
		func(nullptr, &obj, c);
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void handle_context(
	IN std::function<void(IN eco::Bytes&, IN Context&)>& func,
	IN Context& c)
{
	func(c.m_message, c);
}
inline void handle_context_default(
	IN std::function<void(IN eco::Bytes&, IN Context&)>& func,
	IN Context& c)
{
	func(c.m_message, c);
}


////////////////////////////////////////////////////////////////////////////////
// message array handled by "functor", suguest using in client.
template<typename err_t, typename rsp_set_t>
struct ArrayHandler
{
	typedef std::shared_ptr<rsp_set_t> rsp_set_ptr_t;
	typedef std::function<void(err_t*, rsp_set_ptr_t&, Context&)> Func;
	inline ArrayHandler() : m_array(new rsp_set_t)
	{}

	inline ArrayHandler(Func&& func) : m_array(new rsp_set_t), m_func(func)
	{}

	inline ArrayHandler(ArrayHandler&& h)
		: m_array(std::move(h.m_array))
		, m_func(std::move(h.m_func))
	{}

	inline ArrayHandler(const ArrayHandler& h)
		: m_array(h.m_array), m_func(h.m_func)
	{}

	Func m_func;
	rsp_set_ptr_t m_array;
};


////////////////////////////////////////////////////////////////////////////////
template<typename err_t, typename rsp_set_t, typename codec_t>
inline void handle_context_array(ArrayHandler<err_t, rsp_set_t>& h, Context& c)
{
	codec_t codec;
	if (c.has_error())
	{
		err_t err;
		codec.set_message(handle_context_make(err));
		h.m_func(&err, nullptr, c);
	}
	else
	{
		typename rsp_set_t::value_type rsp;
		codec.set_message(handle_context_make(rsp));
		codec.decode(c.m_message.c_str(), c.m_message.size());
		h.m_array.push_back(rsp);
		if (c.is_last())
		{
			h.m_func(nullptr, h.m_array, c);
			h.m_array.reset(new rsp_set_t);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpDispatch
{
public:
	typedef std::function<void(IN Context&)> HandlerFunc;

	virtual ~TcpDispatch() {}

	/*@ register message and message handler.*/
	virtual void register_handler(IN uint64_t id, IN HandlerFunc hdl) = 0;

	/*@ register message default handler.*/
	virtual void register_default(IN HandlerFunc hdl) = 0;

public:
	/*@ register message and message handler, and message dedicated by typeid.
	* @ tmpl.message: message type to be registered.
	* @ tmpl.Handler: to handle "tmpl.message" type.
	*/
	template<typename Handler>
	inline void dispatch()
	{
		register_handler(Handler::request_type(),
			std::bind(&handle_context<Handler>, std::placeholders::_1));
	}

	/*@ register default message handler to process unregistered message type.
	* @ tmpl.Handler: default message handler.
	*/
	template<typename Handler>
	inline void dispatch_default()
	{
		register_default(
			std::bind(&handle_context<Handler>, std::placeholders::_1));
	}

	/*@ dispatch message without message object.*/
	inline void dispatch(IN uint64_t id, IN HandlerFunc hdl)
	{
		register_handler(id, std::move(hdl));
	}

	/*@ register message and message handler function.*/
	template<typename err_t, typename rsp_t, typename codec_t>
	inline void dispatch(
		IN uint64_t id,
		IN std::function<void(IN err_t*, IN rsp_t*, IN Context&)> func)
	{
		register_handler(id, std::bind(&handle_context<err_t, rsp_t, codec_t>,
			func, std::placeholders::_1));
	}

	/*@ register message default handler func.*/
	inline void dispatch(
		IN uint64_t id,
		IN std::function<void(IN eco::Bytes&, IN Context&)> func)
	{
		register_handler(id, std::bind(&handle_context,
			func, std::placeholders::_1));
	}

	/*@ register message default handler func.*/
	inline void dispatch_default(
		IN std::function<void(IN eco::Bytes&, IN Context&)> func)
	{
		register_default(std::bind(&handle_context_default,
			func, std::placeholders::_1));
	}

	/*@ register message and message handler function.*/
	template<typename err_t, typename rsp_set_t, typename codec_t>
	inline void dispatch_array(
		IN uint64_t id,
		IN typename ArrayHandler<err_t, rsp_set_t>::Func f)
	{
		ArrayHandler<rsp_set_t> handler(std::move(f));
		register_handler(id, std::bind(
			&handle_context_array<err_t, rsp_set_t, codec_t>,
			std::move(handler), std::placeholders::_1));
	}

#ifndef ECO_NO_PROTOBUF
	/*@ register message and message handler function.*/
	template<typename err_t, typename rsp_t>
	inline void dispatch(
		IN uint64_t id,
		IN std::function<void(err_t*, rsp_t*, Context&)> func)
	{
		register_handler(id, std::bind(
			&handle_context<err_t, rsp_t, ProtobufCodec>,
			func, std::placeholders::_1));
	}

	template<typename err_t, typename rsp_t, typename object_t, typename func_t>
	inline void dispatch(IN uint64_t id, IN object_t& obj, IN func_t func)
	{
		dispatch<err_t, rsp_t, ProtobufCodec>(id, std::bind(
			func, &obj, std::placeholders::_1,
			std::placeholders::_2, std::placeholders::_3));
	}

	template<typename err_t, typename rsp_set_t>
	inline void dispatch_array(
		IN uint64_t id,
		IN typename ArrayHandler<err_t, rsp_set_t>::Func f)
	{
		ArrayHandler<rsp_set_t> handler(std::move(f));
		register_handler(id, std::bind(
			&handle_context_array<err_t, rsp_set_t, ProtobufCodec>,
			std::move(handler), std::placeholders::_1));
	}
#endif
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif