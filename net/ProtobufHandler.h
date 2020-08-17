#ifndef ECO_NET_PROTOBUF_HANDLER_H
#define ECO_NET_PROTOBUF_HANDLER_H
#ifndef ECO_NO_PROTOBUF
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
#include <eco/App.h>
#include <eco/net/RequestHandler.h>
#include <eco/net/protocol/ProtobufCodec.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
template<typename message_t, typename handler_t>
class ProtobufHandler : public RequestHandler<message_t, handler_t>
{
public:
	typedef ProtobufHandler<message_t, handler_t> Handler;

	inline bool on_decode(IN const char* bytes, IN uint32_t size)
	{
		ProtobufCodec codec(&request());
		return codec.decode(bytes, size);
	}

	inline void to_request()
	{
		if (handler_t::req_sev() == 0) return;
		ECO_REQ_SEV(handler_t::req_sev()) << ProtobufCodec::to_json(request());
	}

public:
	inline void send(
		IN const google::protobuf::Message* msg,
		IN int type, IN bool last_, IN bool err)
	{
		ProtobufCodec cdc(msg);
		MessageMeta meta;
		meta.codec(cdc).message_type(type).last(last_).error(err);
		context().response(meta);

		// logging protobuf.
		if (!last_) return;
		int sev = err ? handler_t::err_sev() : handler_t::rsp_sev();
		if (sev == eco::log::none) return;
		ECO_RSP_SEV(sev) << (msg ? ProtobufCodec::to_json(*msg) : empty_str);
	}

	// response message to client.
	inline void resolve(
		IN const google::protobuf::Message* msg = nullptr,
		IN bool last_ = true)
	{
		send(msg, handler_t::response_type(), last_, false);
	}

	// response message to client.
	template<typename object_t, typename set_t>
	inline void resolve_set(IN const set_t& set)
	{
		for (auto it = set.begin(); it != set.end(); )
		{
			const object_t& obj = eco::object(*it);
			resolve(&obj, ++it == set.end());
		}
	}

	// response error message to client.
	inline void reject()
	{
		auto& e = eco::this_thread::error().data();
		get_error(e, "");
		reject(&e);
	}
	inline void reject(eco::proto::Error& e, const char* mdl = "")
	{
		get_error(e, mdl);
		reject(&e);
	}
	inline void reject(uint32_t eid)
	{
		eco::this_thread::error().id(eid);
		reject();
	}
	inline void reject(const std::string& path_)
	{
		eco::this_thread::error().path(path_);
		reject();
	}
	inline void reject(IN const google::protobuf::Message* msg)
	{
		send(msg, handler_t::response_type(), true, true);
	}

public:
	inline static handler_ptr send(
		IN int async_id,
		IN const google::protobuf::Message* msg,
		IN int type, IN bool last_, IN bool err)
	{
		auto h = pop_async(async_id, opt.is_last());
		if (h) h->send(msg, type, last_, err);
		return h;
	}
	inline static handler_ptr resolve(
		IN int async_id,
		IN const google::protobuf::Message* msg = nullptr,
		IN bool last_ = true)
	{
		return send(msg, async_id, handler_t::response_type(), last_, false);
	}
	inline static void reject(
		IN int async_id,
		IN const google::protobuf::Message* msg)
	{
		return send(msg, async_id, handler_t::response_type(), true, true);
	}
	inline static void reject(
		IN int async_id, IN eco::proto::Error& e, IN const char* mdl = "")
	{
		auto h = pop_async(async_id, true);
		if (h) h->reject(e, mdl);
		return h;
	}
	inline static void reject(
		IN int async_id, uint32_t eid, const char* param = "")
	{
		auto h = pop_async(async_id, true);
		if (h) h->reject(eid, param);
		return h;
	}
	inline static void reject(
		IN int async_id, const char* path, const char* param = "")
	{
		auto h = pop_async(async_id, true);
		if (h) h->reject(path, param);
		return h;
	}

public:
	inline void get_error(eco::proto::Error& e, const char* mdl)
	{
		const char* lang_ = connection().lang();
		if (!eco::empty(lang_))
		{
			const char* msg = (!e.path().empty())
				? eco::loc::locale().get_error(e.path().c_str(),
					e.mutable_message()->c_str(), mdl, lang_)
				: eco::loc::locale().get_error(e.id(),
					e.mutable_message()->c_str(), mdl, lang_);
			e.set_message(msg);
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
class GetLocaleHandler : 
	public eco::net::ProtobufHandler<NullRequest, GetLocaleHandler>
{
	ECO_HANDLE_OPTION(auth_none);
	ECO_HANDLE_LOGGING(none, none, none);	// disable logging locale.
	ECO_HANDLE(eco::proto_locale_get_req, eco::proto_locale_get_rsp, "locale");
public:
	virtual void on_request() override
	{
		ECO_REQ(debug);
		resolve(&eco::App::get()->locale().data());
		ECO_RSP(debug);
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif
#endif