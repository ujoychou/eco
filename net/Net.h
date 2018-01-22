#ifndef ECO_NET_H
#define ECO_NET_H
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
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>



////////////////////////////////////////////////////////////////////////////////
/* @ eco log format.
@ when eco.net has error, just like tcpserver or tcpclient. we must trace
and find out the reasons. so we will focus on multi factor: who-which
peer, in which class, in which thread, at what time, and most important
info-error description. and amostly error happened when recv a request,
or send a response, so get the request and response info.

@ error format:
1.peer_id + req_flag\rsp_flag: emsg @eid.
2.req\rsp detail.
*/


////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace net{;


typedef uint32_t SessionId;
typedef size_t ConnectionId;
const SessionId none_session = 0;
////////////////////////////////////////////////////////////////////////////////
// get local machine network info: ip\hostname\mac address.
ECO_API const char* get_ip();
ECO_API const char* get_hostname();
ECO_API const char* get_mac();


/*@ network to host integer 16bit.*/
ECO_API uint16_t ntoh16(IN const char* str);
inline void ntoh(OUT uint16_t& val, IN const char* str)
{
	val = ntoh16(str);
}
inline void ntoh(OUT uint16_t& val, OUT uint32_t& pos, IN const char* str)
{
	val = ntoh16(str);
	pos += sizeof(uint16_t);
}
/*@ host to network integer 16bit.*/
ECO_API void hton(OUT char* str, IN const uint16_t val);
inline void hton(OUT char* str, OUT uint32_t& pos, IN const uint16_t val)
{
	hton(str, val);
	pos += sizeof(uint16_t);
}
template<typename String>
void append_hton(OUT String& str, IN const uint16_t val)
{
	uint32_t init_size = str.size();
	str.resize(init_size + sizeof(uint16_t));
	hton(&str[init_size], val);
}
/*@ network to host integer 16bit.*/
inline void ntoh(OUT int16_t& val, IN const char* str)
{
	val = ntoh16(str);
}
inline void ntoh(OUT int16_t& val, OUT uint32_t& pos, IN const char* str)
{
	val = ntoh16(str);
	pos += sizeof(uint16_t);
}


/*@ network to host u integer 32bit.*/
ECO_API uint32_t ntoh32(IN const char* str);
inline void ntoh(OUT uint32_t& val, IN const char* str)
{
	val = ntoh32(str);
}
inline void ntoh(OUT uint32_t& val, OUT uint32_t& pos, IN const char* str)
{
	val = ntoh32(str);
	pos += sizeof(uint32_t);
}
/*@ host to network u integer 32bit.*/
ECO_API void hton(OUT char* str, IN const uint32_t val);
inline void hton(OUT char* str, OUT uint32_t& pos, IN const uint32_t val)
{
	hton(str, val);
	pos += sizeof(uint32_t);
}
template<typename String>
void append_hton(OUT String& str, IN const uint32_t val)
{
	uint32_t init_size = str.size();
	str.resize(init_size + sizeof(uint32_t));
	hton(&str[init_size], val);
}
/*@ network to host integer 32bit.*/
inline void ntoh(OUT int32_t& val, IN const char* str)
{
	val = ntoh32(str);
}
inline void ntoh(OUT int32_t& val, OUT uint32_t& pos, IN const char* str)
{
	val = ntoh32(str);
	pos += sizeof(uint32_t);
}


/*@ network to host u integer 64bit.*/
inline uint64_t ntoh64(IN const char* str)
{
	uint64_t v = ntoh32(str);		// high bit.
	v <<= 32;
	v += ntoh32(str + 4);			// low bit.
	return v;
}
inline void ntoh(OUT uint64_t& val, IN const char* str)
{
	val = ntoh64(str);
}
inline void ntoh(OUT uint64_t& val, OUT uint32_t& pos, IN const char* str)
{
	val = ntoh64(str);
	pos += sizeof(uint64_t);
}
/*@ host to network u integer 64bit.*/
inline void hton(OUT char* str, IN const uint64_t val)
{
	uint32_t low = static_cast<uint32_t>(val);
	uint32_t high = static_cast<uint32_t>(val >> 32);
	uint32_t pos = 0;
	hton(str, pos, high);
	hton(str, pos, low);
}
inline void hton(OUT char* str, OUT uint32_t& pos, IN const uint64_t val)
{
	hton(str, val);
	pos += sizeof(uint64_t);
}
template<typename String>
void append_hton(OUT String& str, IN const uint64_t val)
{
	uint32_t init_size = str.size();
	str.resize(init_size + sizeof(uint64_t));
	hton(&str[init_size], val);
}
/*@ network to host integer 64bit.*/
inline void ntoh(OUT int64_t& val, IN const char* str)
{
	val = ntoh64(str);
}
inline void ntoh(OUT int64_t& val, OUT uint32_t& pos, IN const char* str)
{
	val = ntoh64(str);
	pos += sizeof(uint64_t);
}


}}
////////////////////////////////////////////////////////////////////////////////
#endif