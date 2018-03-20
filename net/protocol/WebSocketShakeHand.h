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
#include <eco/codec/base64.h>
#include <eco/codec/sha1.h>
#include <eco/Type.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class WebSocketShakeHand : public eco::Object<WebSocketShakeHand>
{
public:
	inline WebSocketShakeHand()
	{}

	inline static uint32_t size()
	{
		return 1024;
	}

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
		if (!parse_key(server_key, request, false))
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
	inline bool parse_req(IN const char* request)
	{
		eco::String server_key;
		if (!parse_key(server_key, request, true))
		{
			return false;
		}
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

private:
	// parse websocket request shakehand key.
	inline bool parse_key(
		OUT eco::String& server_key, 
		IN  const char* request,
		IN  const bool websocket_req)
	{
		const char* key_name = websocket_req
			? "Sec-WebSocket-Key" : "Sec-WebSocket-Accept";
		const char* data = request;
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

			// match the "key".
			const char* find = eco::find(data, key_end - data, key_name);
			if (find != nullptr)
			{
				// format: "Sec-WebSocket-Key: t/b9b1gBUKvemepe7PhatQ=="
				key_end += 2;	// skip ": ", note there is a space.
				server_key.asign(key_end, static_cast<uint32_t>(val_end - key_end));
				if (server_key[server_key.size() - 1] == '\r')
					server_key.resize(server_key.size() - 1);
				if (websocket_req)
					server_key.append(magic_key());
				return true;
			}
			data = val_end + 1;		// next "key:value".
		}// end 
		return false;	// for warning.
	}

private:
	eco::String m_resp;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif
