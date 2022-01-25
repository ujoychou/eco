#ifndef ECO_NET_WEBSOCKET_SHAKE_HAND
#define ECO_NET_WEBSOCKET_SHAKE_HAND
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
* copyright(c) 2017 - 2020, ujoy, reserved all right.

*******************************************************************************/
#include <codec/md5.h>
#include <codec/sha1.h>
#include <codec/base64.h>
#include <eco/Object.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class WebSocketShakeHand : public eco::Object<WebSocketShakeHand>
{
public:
	inline WebSocketShakeHand()
	{}

	inline static const char* head_end()
	{
		return "\r\n\r\n";
	}

	inline static const char* magic_key()
	{
		return "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	}

	inline eco::String& response()
	{
		return m_resp;
	}

	// get websocet client request.
	inline eco::String& format()
	{
		m_resp.resize(0);
		m_resp.reserve(1024);
		m_resp.append("GET / HTTP/1.1\r\n");
		m_resp.append("Host: 127.0.0.1:41205\r\n");
		m_resp.append("Connection: Upgrade\r\n");
		m_resp.append("Pragma: no-cache\r\n");
		m_resp.append("Cache-Control: no-cache\r\n");
		m_resp.append("Upgrade: websocket\r\n");
		m_resp.append("Sec-WebSocket-Version: 13\r\n");
		m_resp.append("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
			"AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.146 "
			"Safari/537.36\r\n");
		m_resp.append("Accept-Encoding: gzip, deflate, br\r\n");
		m_resp.append("Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\n");
		m_resp.append("Sec-WebSocket-Key: 0A0r7tnSAYscOhMUO8tIcw==\r\n");
		m_resp.append("Sec-WebSocket-Extensions: permessage-deflate; "
			"client_max_window_bits\r\n\r\n");
		return m_resp;
	}

	// parse websocket request shakehand response from server.
	inline bool parse_rsp(IN const char* request, IN const char* key)
	{
		eco::String server_key;
		if (!parse_server_key(server_key, request, false))
		{
			return false;
		}

		// check server key parse by server..
		if (strcmp(server_key.c_str(), key) != 0)
		{
			return false;
		}

		return true;
	}

	// parse websocket request shakehand request from client.
	// and make a response message.
	inline bool parse_req(IN const char* request, IN const uint32_t size)
	{
		// #.chrome & mozilla browers.
		eco::String server_key;
		if (parse_server_key(server_key, request, true))
		{
			SHA1 sha;
			unsigned int message_digest[5];
			sha << server_key.c_str();
			sha.Result(message_digest);
			for (int i = 0; i < 5; i++)
			{
				message_digest[i] = htonl(message_digest[i]);
			}

			m_resp.reserve(1024);
			m_resp.resize(0);
			m_resp.append("HTTP/1.1 101 Switching Protocols\r\n");
			m_resp.append("Connection: upgrade\r\n");
			m_resp.append("Sec-WebSocket-Accept: ");
			m_resp.append(base64_encode(
				(const unsigned char*)(message_digest), 20).c_str());
			m_resp.append("\r\n");
			m_resp.append("Upgrade: websocket\r\n\r\n");
			return true;
		}
		// #.safari browers.
		else
		{
			eco::String host;
			eco::String orgin;
			eco::String md5_key;
			if (parse_key(host, request, "Host") &&
				parse_key(orgin, request, "Origin") &&
				parse_md5(md5_key, request, size))
			{
				m_resp.reserve(1024);
				m_resp.resize(0);
				m_resp << ("HTTP/1.1 101 WebSocket Protocol Handshake\r\n");
				m_resp << ("Upgrade: WebSocket\r\n");
				m_resp << ("Connection: Upgrade\r\n");
				m_resp << ("Sec-WebSocket-Origin: ");
				m_resp << (orgin);
				m_resp << ("\r\n");
				m_resp << ("Sec-WebSocket-Location: ");
				m_resp << ("ws://");
				m_resp << (host);
				m_resp << ("/\r\n\r\n");		// empty line.
				m_resp << (md5_key);
				return true;
			}
		}
		return false;
	}

private:
	// parse websocket request shakehand key.
	inline bool parse_md5(
		OUT eco::String& md5,
		IN  const char* request,
		IN  const uint32_t size)
	{
		eco::String last;
		eco::String sec_key1;
		eco::String sec_key2;
		if (parse_key(sec_key1, request, "Sec-WebSocket-Key1") &&
			parse_key(sec_key2, request, "Sec-WebSocket-Key2") &&
			parse_last_line(last, request, size))
		{
			eco::String key;
			eco::String key2;
			key.reserve(16);
			parse_key_number(key, sec_key1.c_str());
			parse_key_number(key2, sec_key2.c_str());
			key << key2 << last;
			md5.assign(MD5(key.c_str(), key.size()).digest(), MD5::digest_size);
			return true;
		}
		return false;
	}

	// parse websocket request shakehand key.
	inline uint64_t parse_key_number(
		OUT eco::String& num,
		IN  const char* key)
	{
		num.resize(16);
		memset(&num[0], 0 ,num.size());

		// extract the key number and count the empty space of key.
		uint32_t count = 0;
		uint32_t i = 0;
		for (const char* t = key; *t != '\0' && i < num.size(); ++t)
		{
			if (*t >= '0' && *t <= '9')
				num[i++] = *t;
			else if (*t == ' ')
				++count;
		}
		num.resize(i);
		if (count == 0)
		{
			return false;
		}

		// get network bytes.
		uint32_t result = uint32_t(eco::cast<uint64_t>(num.c_str()) / count);
		eco::net::hton(&num[0], (i = 0), result);
		num.resize(i);
		return true;
	}

	// parse websocket request shakehand key.
	inline bool parse_last_line(
		OUT eco::String& value,
		IN  const char* request,
		IN  const uint32_t size)
	{
		const char* data = request;
		uint32_t start = eco::find_last(request, size, '\n');
		uint32_t len = size - start - 1;
		if (start != -1 && len > 0)
		{
			value.assign(&request[start + 1], len);
			return true;
		}
		return false;
	}

	// parse websocket request shakehand key.
	inline bool parse_key(
		OUT eco::String& value,
		IN  const char* data,
		IN  const char* key)
	{
		while (true)
		{
			// find position.
			const char* key_end = strchr(data, ':');
			const char* val_end = strchr(data, '\n');
			if (key_end == nullptr || val_end == nullptr)
			{
				return false;
			}
			while (val_end < key_end)
			{
				data = val_end + 1;
				val_end = strchr(data, '\n');
				if (val_end == nullptr)
				{
					return false;
				}
			}

			// match "key" like "sec-websocket-key:" or "sec-websocket-key :"
			size_t klen = strlen(key);
			size_t size = key_end - data;
			const char* find = eco::find(data, size, key, klen);
			if (find != nullptr && (size == klen || find[klen] == ' '))
			{
				// format: "key: value/r/n".
				key_end += 2;	// skip ": ", note there is a space.
				value.assign(key_end, static_cast<uint32_t>(val_end - key_end));
				if (value[value.size() - 1] == '\r')
					value.resize(value.size() - 1);
				return true;
			}
			data = val_end + 1;		// next "key:value".
		}// end 
		return false;	// for warning.
	}

	// parse websocket request shakehand key.
	inline bool parse_server_key(
		OUT eco::String& server_key, 
		IN  const char* request,
		IN  const bool ws_req)
	{
		const char* key = ws_req ? "Sec-WebSocket-Key" : "Sec-WebSocket-Accept";
		if (parse_key(server_key, request, key))
		{
			if (ws_req) server_key.append(magic_key());
			return true;
		}
		return false;
	}

private:
	eco::String m_resp;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif
