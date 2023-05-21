#pragma once
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
#include <eco/net/TcpSession.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
// handle tcp session response
template<typename TCodec, typename TRsp, typename TErr>
inline bool handle_rsp(TcpSession& sess, TRsp& rsp, TErr& err)
{
	TCodec codec;
	if (sess.error())
	{
		// decode error
		codec.set_message(eco::make(err));
		if (!codec.decode(sess.data(), sess.size()))
		{
			eco::Error(eco::net::ERROR_DECODE_ERR_FAIL) << sess.size();
		}
	}
	else
	{
		// decode response
		codec.set_message(eco::make(rsp));
		if (codec.decode(sess.data(), sess.size())) 
		{
			return true;
		}
		eco::Error(eco::net::ERROR_DECODE_RSP_FAIL) << sess.size();
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
class TcpRequestSync
{
	typedef void (TcpRequestSync::*RspCall)(TcpSession&, void* rsp, void* err);
	void* rsp;
	void* err;
	RspCall call;
	eco::Result result;

public:
	inline TcpRequestSync(RspCall call, void* rsp, void* err)
		: call(call), rsp(rsp), err(err)
	{}

	inline void operator()(TcpSession& sess)
	{
		this->call(sess, rsp, err);
	}

	inline void on_rsp1(TcpSession& sess, void* rsp, void* err)
	{
		sess = std::move(*(TcpSession*)rsp);
	}

	template<typename TCodec, typename TRsp, typename TErr>
	inline void on_rsp2(TcpSession& sess, void* rsp, void* err)
	{
		handle_rsp<TCodec, TRsp, TErr>(sess, *(TRsp*)rsp, *(TErr*)err);
	}

	template<typename TCodec, typename TRsp, typename TErr>
	inline void on_rsp3(TcpSession& sess, void* rsp_set, void* err)
	{
		TRsp rsp;
		std::vector<TRsp>& rsp_vec = *(std::vector<TRsp>*)(rsp_set);
		if (handle_rsp<TCodec, TRsp, TErr>(sess, rsp, *(TErr*)err))
		{
			rsp_vec.emplace_back(rsp);
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
class TcpRequestAsync
{
public:
	inline TcpRequestAsync() {}
	virtual ~TcpRequestAsync() {}

	// make drived tcp_session_async object
	template<typename TAsync, typename... Args>
	inline static TcpRequestAsync* make(Args... args)
	{
		TcpRequestAsync* async = new TAsync(std::forward<Args>(args));
		async->call = &TcpRequestAsync::on_rsp;
		return async;
	}

	// called by tcp handle thread when recv response from server
	inline void operator()(TcpSession& sess)
	{
		this->call(sess);
	}

private:
	// call function: from c api to c++ api
	inline void on_rsp(TcpSession& sess)
	{
		this->handle(sess);
		if (sess.last()) { delete this; }
	}

	// handle tcp session response
	virtual void handle(TcpSession& sess) = 0;

	typedef void (TcpRequestAsync::*RspCall)(TcpSession&, TcpSessAsync*);
	RspCall call;
};


////////////////////////////////////////////////////////////////////////////////
template<typename TCodec, typename TRsp, typename TErr>
struct TcpRequestAsync1 : public TcpRequestAsync
{
	typedef std::function<void(TcpSession&)> RspFunc;
	RspFunc on_rsp;

public:
	inline TcpRequestAsync1(RspFunc&& func) : on_rsp(func) {}
	virtual ~TcpRequestAsync1() {}

	virtual void handle(TcpSession& sess) override
	{
		return this->on_rsp(sess);
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename TCodec, typename TRsp, typename TErr>
struct TcpRequestAsync2 : public TcpRequestAsync
{
	typedef std::function<void(TRsp&, TErr&, bool)> RspFunc;
	RspFunc on_rsp;

public:
	inline TcpRequestAsync2(RspFunc&& func) : on_rsp(func) {}
	virtual ~TcpRequestAsync2() {}

	virtual void handle(TcpSession& sess) override
	{
		TRsp rsp;
		TErr err;
		handle_rsp<TCodec>(sess, rsp, err);
		this->on_rsp(rsp, err, eco::Error());
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename TCodec, typename TRsp, typename TErr>
struct TcpRequestAsync3 : public TcpRequestAsync
{
	typedef std::function<void(TRsp&, TErr&, bool, bool)> RspFunc;
	RspFunc on_rsp;

public:
	inline TcpRequestAsync3(RspFunc&& func) : on_rsp(func) {}
	virtual ~TcpRequestAsync3() {}

	virtual void handle(TcpSession& sess) override
	{
		TRsp rsp;
		TErr err;
		handle_rsp<TCodec>(sess, rsp, err);
		this->on_rsp(rsp, err, eco::Error(), sess.last());
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename TCodec, typename TRsp, typename TErr>
struct TcpRequestAsync4 : public TcpRequestAsync
{
	typedef std::function<void(std::vector<TRsp>&, TErr&, bool)> RspFunc;
	TErr err;
	std::vector<TRsp> rsp_vec;
	RspFunc on_rsp;

public:
	inline TcpRequestAsync4(RspFunc&& func) : on_rsp(func) {}
	virtual ~TcpRequestAsync4() {}

	virtual void handle(TcpSession& sess) override
	{
		TRsp rsp;
		if (handle_rsp<TCodec>(sess, rsp, this->err))
		{
			this->rsp_vec.emplace_back(rsp);
		}
		if (sess.last())
		{
			this->on_rsp(this->rsp_vec, this->err, eco::Error());
			this->rsp_vec.clear();
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);