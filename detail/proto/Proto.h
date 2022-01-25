#ifndef ECO_PROTO_H
#define ECO_PROTO_H
#ifndef ECO_NO_PROTOBUF
/*******************************************************************************
@ 名称

@ 功能

@ 异常

@ 备注

--------------------------------------------------------------------------------
@ 历史记录 @
@ ujoy modifyed on 2016-11-02

--------------------------------------------------------------------------------
* 版权所有(c) 2016 - 2019, siberia corp, 保留所有权利。

*******************************************************************************/
#include <eco/Object.h>
#ifndef ECO_PROTOBUF2
#	include <eco/detail/proto/Eco.pb.h>
#	include <eco/detail/proto/Monitor.pb.h>
#else
#	include <eco/detail/proto2/Eco.pb.h>
#	include <eco/detail/proto2/Monitor.pb.h>
#endif


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
enum 
{
	// locale.
	proto_locale_get_req		= 101,
	proto_locale_get_rsp		= 102,

	// user defined protocol start from 1000.
	proto_user_defined			= 1000,
};


////////////////////////////////////////////////////////////////////////////////
template<typename StreamT>
StreamT& operator<<(OUT StreamT& stream, IN const eco::proto::Error& e)
{
	stream <= e.value() <= '#';
	if (e.id() != 0) stream < e.id();
	if (!e.path().empty()) stream < e.path();
	return stream;
}
template<typename StreamT>
StreamT& operator<<(OUT StreamT& stream, IN const eco::proto::Property& p)
{
	stream <= p.user_id() < '-';
	if (p.object() != 0) stream < p.object();
	if (!p.entity().empty()) stream < p.entity();
	stream <= p.name() <= p.value();
	return stream;
}


////////////////////////////////////////////////////////////////////////////////
class WordFmt
{
	ECO_STREAM_OPERATOR(WordFmt, value(), ';');
public:
	inline WordFmt(eco::proto::Word* w = nullptr) : m_data(w)
	{}

	inline void clear()
	{
		m_data->mutable_path()->clear();
		m_data->mutable_value()->clear();
	}

	// set path: "a/b/c"
	inline WordFmt& path(const char* v)
	{
		m_data->set_path(v);
		m_data->mutable_value()->clear();
		return *this;
	}
	inline WordFmt& operator()(const char* v)
	{
		return path(v);
	}

	// path: add pro: "a/b" / "c" = "a/b/c";
	inline WordFmt& operator / (const char* v)
	{
		*m_data->mutable_path() += '/';
		*m_data->mutable_path() += v;
		return *this;
	}

	// param: add param "v1;v2" + "v3" = "v1;v2;v3".
	inline WordFmt& append(const char* v, uint32_t n)
	{
		value().append(v, n);
		return *this;
	}
	inline WordFmt& append(uint32_t n, char v)
	{
		value().append(n, v);
		return *this;
	}

private:
	inline std::string& value()
	{
		return *m_data->mutable_value();
	}

	eco::proto::Word* m_data;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif
#endif