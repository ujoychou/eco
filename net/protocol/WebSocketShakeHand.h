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

	// parse websocket request shakehand.
	inline bool parse(IN const char* request)
	{
		eco::String server_key;
		if (!parse_key(server_key, request))
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
		OUT eco::String& server_key, IN const char* request)
	{
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
			const char* find = eco::find(
				data, key_end - data, "Sec-WebSocket-Key");
			if (find != nullptr)
			{
				// format: "Sec-WebSocket-Key: t/b9b1gBUKvemepe7PhatQ=="
				key_end += 2;	// skip ": ", note there is a space.
				server_key.asign(key_end, val_end - key_end);
				if (server_key[server_key.size() - 1] == '\r')
				{
					server_key.resize(server_key.size() - 1);
				}
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
