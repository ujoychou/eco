#ifndef ECO_CODEC_IP_H
#define ECO_CODEC_IP_H
/*******************************************************************************
@ name

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Cast.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(codec);
////////////////////////////////////////////////////////////////////////////////
class Ip
{
public:
	inline static const char* scale_lower()
	{
		return "0123456789abcdefghijklmnopqrstuvwxyz";
	}
	inline static const char* scale_upper()
	{
		return "0123456789abcdefghijklmnopqrstuvwxyz";
	}
	inline static const char* scale_all()
	{
		return "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	}

	inline static std::string encode(const char* ip, const char* scale = scale_all())
	{
		const uint32_t digit = strlen(scale);

		// append '.' to string.
		std::string ip_str = ip;
		ip_str += ".";
		const char* ip_c = ip_str.c_str();

		// ip string -> ip int value.
		uint32_t ip_int = 0;
		uint32_t base = 1;
		uint32_t pos = eco::find_first(ip_c, '.');
		while (pos != -1)
		{
			uint32_t v = eco::cast<uint32_t>(ip_c, pos);
			ip_int += v * base;
			base *= 256;
			ip_c += pos + 1;
			pos = eco::find_first(ip_c, '.');
		}

		// int value -> code.
		std::string result;
		while (ip_int > 0)
		{
			auto left = ip_int % digit;
			ip_int = ip_int / digit;
			result.push_back(scale[left]);
		}
		return result;
	}

	inline static std::string decode(const char* code, const char* scale = scale_all())
	{
		const uint32_t digit = strlen(scale);

		// code -> int value.
		uint32_t ip_int = 0;
		uint32_t base = 1;
		auto size = strlen(code);
		for (size_t i = 0; i < size; i++)
		{
			uint32_t v = eco::find_first(scale, code[i]);
			ip_int += v * base;
			base *= digit;
		}

		// int value -> ip
		std::string result;
		while (ip_int > 0)
		{
			auto left = ip_int % 256;
			ip_int = ip_int / 256;
			auto v = eco::cast<std::string>(left);
			result.append(v);
			result.push_back('.');
		}
		result.pop_back();
		return result;
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(codec);
ECO_NS_END(eco);