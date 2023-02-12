#ifndef ECO_NET_CODEC_H
#define ECO_NET_CODEC_H
/*******************************************************************************
@ name


@ function


@ exception


@ remark


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/rx/RxApi.h>
#include <eco/net/Ecode.h>
#include <eco/String.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class Codec
{
public:
	inline Codec(const void* msg) : m_msg((void*)msg)
	{}
	inline bool empty() const
	{
		return (m_msg == nullptr);
	}

	/*@ set message data to be encoded or to be decoded.*/
	virtual void set_message(void* message) = 0;

	/*@ get bytes size of encoded message.*/
	virtual uint32_t byte_size() const = 0;

	/*@ append encode data string.*/
	virtual void encode_append(OUT eco::String& bytes, IN uint32_t size) const
	{
		uint32_t init_size = bytes.size();
		bytes.resize(init_size + size);
		encode(&bytes[init_size], size);
	}

	/*@ encode message to message bytes.
	* @ para.bytes: encoded message bytes.
	* @ para.size: bytes size of encoded message, return by "get_byte_size".
	*/
	virtual void encode(OUT char* bytes, IN uint32_t size) const = 0;

	/*@decode message from bytes string.
	* @return: message ptr if success, nullptr if fail.
	*/
	virtual void* decode(IN const char* bytes, IN uint32_t size)
	{
		return nullptr;
	}

protected:
	void* m_msg;
};


////////////////////////////////////////////////////////////////////////////////
class WrapCodec : public Codec
{
public:
	typedef void* (*MakeFunc)();

	template<typename type_t>
	inline static void* make()
	{
		return new type_t();
	}

	// constructor.
	inline explicit WrapCodec(Codec& c, MakeFunc f) : Codec(&c), m_make(f) 
	{}

	// codec object.
	inline Codec& get_message() 
	{
		return *(Codec*)m_msg;
	}
	inline const Codec& message() const
	{
		return *(Codec*)m_msg;
	}

	virtual void set_message(void* v) override
	{
		get_message().set_message(v);
	}

	virtual uint32_t byte_size() const override
	{
		return message().byte_size();
	}

	virtual void encode(OUT char* bytes, IN uint32_t size) const override
	{
		message().encode(bytes, size);
	}

	virtual void* decode(IN const char* bytes, IN uint32_t size) override
	{
		get_message().set_message(m_make());
		return get_message().decode(bytes, size);
	}

private:
	MakeFunc m_make;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif