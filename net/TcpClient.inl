#ifndef ECO_NET_TCP_CLIENT_INL
#define ECO_NET_TCP_CLIENT_INL
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-12-14.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/net/TcpClient.h>


ECO_NS_BEGIN(eco);
namespace net{;
////////////////////////////////////////////////////////////////////////////////
template<typename codec_t, typename err_t>
inline void TcpClient::call_reject(
	const char* name_, eco::net::Context& c,
	std::function<void(err_t&, eco::net::Context&)>& reject)
{
	err_t err;
	codec_t codec;
	codec.set_message(eco::make(err));
	if (codec.decode(c.m_message.m_data, c.m_message.m_size))
	{
		reject(err, c);
		return;
	}
	ECO_LOG(error, name_) < "parse object fail: " 
		< typeid(err_t).name() <= c.m_message.m_size;
	reject(err, c);
}


////////////////////////////////////////////////////////////////////////////////
template<typename codec_t, typename err_t, typename rsp_t>
inline bool TcpClient::call_resolve(
	const char* name_, eco::net::Context& c, rsp_t& rsp,
	std::function<void(err_t&, eco::net::Context&)>& reject)
{
	codec_t codec;
	codec.set_message(eco::make(rsp));
	if (!codec.decode(c.m_message.m_data, c.m_message.m_size))
	{
		ECO_LOG(error, name_) < "parse object fail "
			< typeid(rsp_t).name() <= c.m_message.m_size;
		c.m_meta.error(true);
		reject(err_t(), c);
		return false;
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
template<typename codec_t, typename err_t>
TcpClient::ResponseFunc TcpClient::context_func(
	std::function<void(err_t&, eco::net::Context&)>& reject,
	std::function<void(eco::net::Context&)>& resolve)
{
	return [=](eco::net::Context& c) mutable {
		if (c.has_error())
			call_reject<codec_t, err_t>("async", c, reject);
		else
			resolve(c);
	};
}


////////////////////////////////////////////////////////////////////////////////
template<typename codec_t, typename err_t, typename rsp_t>
TcpClient::ResponseFunc TcpClient::context_func(
	std::function<void(err_t&, eco::net::Context&)>& reject,
	std::function<void(rsp_t&, eco::net::Context&)>& resolve)
{
	auto* f_reject = &TcpClient::call_reject<codec_t, err_t>;
	auto* f_resolve = &TcpClient::call_resolve<codec_t, err_t, rsp_t>;
	return [=](eco::net::Context& c) mutable {
		if (c.has_error()) {
			f_reject("async", c, reject);
			return;
		}

		rsp_t rsp;
		if (f_resolve("async", c, rsp, reject)) {
			resolve(rsp, c);
		}
	};
}


////////////////////////////////////////////////////////////////////////////////
template<typename codec_t, typename err_t, typename rsp_set_t>
TcpClient::ResponseFunc TcpClient::context_func(
	std::shared_ptr<rsp_set_t>& rsp_set,
	std::function<void(err_t&, eco::net::Context&)>& reject,
	std::function<void(std::shared_ptr<rsp_set_t>&)>& resolve)
{
	return [=](eco::net::Context& c) mutable {
		if (c.has_error()) {
			call_reject<codec_t, err_t>("async_set", c, reject);
			return;
		}

		typename rsp_set_t::value_type rsp;
		if (call_resolve<codec_t, err_t>("async_set", c, rsp, reject)) {
			rsp_set->push_back(std::move(rsp));	// add data to response array.
			if (c.is_last()) resolve(rsp_set);
		}
	};
}


////////////////////////////////////////////////////////////////////////////////
template<typename codec_t, typename err_t>
inline void TcpClient::async(
	IN uint32_t req_type, IN const void* req,
	IN std::function<void(err_t&, eco::net::Context&)>&& reject,
	IN std::function<void(eco::net::Context&)>&& resolve)
{
	codec_t req_codec(req);
	MessageMeta req_meta;
	req_meta.codec(req_codec).message_type(req_type);
	return async(req_meta, context_func<codec_t, err_t>(reject, resolve));
}


////////////////////////////////////////////////////////////////////////////////
template<typename codec_t, typename err_t, typename rsp_t>
inline void TcpClient::async(
	IN uint32_t req_type, IN const void* req,
	IN std::function<void(err_t&, eco::net::Context&)>&& reject,
	IN std::function<void(rsp_t&, eco::net::Context&)>&& resolve)
{
	codec_t req_codec(req);
	MessageMeta req_meta;
	req_meta.codec(req_codec).message_type(req_type);
	return async(req_meta, context_func<codec_t, err_t, rsp_t>(reject, resolve));
}


////////////////////////////////////////////////////////////////////////////////
template<typename codec_t, typename err_t, typename rsp_set_t>
inline void TcpClient::async_set(
	IN uint32_t req_type, IN const void* req,
	IN std::function<void(err_t&, eco::net::Context&)>&& reject,
	IN std::function<void(std::shared_ptr<rsp_set_t>&)>&& resolve)
{
	// reject err object.
	std::shared_ptr<rsp_set_t> rsp_set(new rsp_set_t());
	codec_t req_codec(req);
	MessageMeta req_meta;
	req_meta.codec(req_codec).message_type(req_type);
	return async(req_meta, context_func<codec_t, err_t, rsp_set_t>(
		rsp_set, reject, resolve));
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif