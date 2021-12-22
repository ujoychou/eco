#ifndef ECO_STRING_H
#define ECO_STRING_H
/*******************************************************************************
@ name
eco basic type.

@ function
1.string_any.
2.variant.

@ exception


@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Cast.h>
#include <eco/Object.h>
#include <eco/HeapOperators.h>


ECO_NS_BEGIN(eco);

// function general result.
enum 
{
	ok				= 0,
	fail			= 1,
	error			= 2,
	timeout			= 3,

	// user defined error id.
	defined_error_start	= 100,
};
typedef int Result;


////////////////////////////////////////////////////////////////////////////////
// error mechanism: get this_thread error
class Error;
ECO_NS_BEGIN(this_thread);
// get current thread error.
ECO_API eco::Error& error();
ECO_API int  error_id();
ECO_API const char* error_path();
ECO_API const char* error_value();
ECO_API void error_key(int);
ECO_API void error_key(const char*);
ECO_API void error_add(const char*);
ECO_API void error_val(const char*);
ECO_API void error_clear();
ECO_API void error_append(IN uint32_t siz, IN char c);
ECO_API void error_append(IN const char* buf, IN uint32_t siz);
#define ECO_THROW(id_msg) throw eco::this_thread::error().key_throw(id_msg)
#define ECO_THIS_ERROR(id_path) eco::this_thread::error().key(id_path)
#define ECO_THIS_ERROR_ADD(id_path) eco::this_thread::error().add(id_path)
ECO_NS_END(this_thread);


////////////////////////////////////////////////////////////////////////////////
#define ECO_STREAM_OPERATOR_REF(stream_t, value_t, sep)\
inline stream_t& operator<(IN const value_t& v)\
{\
	return *this << v;\
}\
inline stream_t& operator<=(IN const value_t& v)\
{\
	return *this << sep << v;\
}
#define ECO_STREAM_OPERATOR_TPL(stream_t, value_t, sep1, sep2)\
template<typename T>\
inline stream_t& operator<<(IN const value_t& v)\
{\
	return (*this) << sep1 << v.value << sep2;\
}\
template<typename T>\
inline stream_t& operator<(IN const value_t& v)\
{\
	return (*this) << v;\
}\
template<typename T>\
inline stream_t& operator<=(IN const value_t& v)\
{\
	return (*this) <= sep1 < v.value < sep2;\
}

#define ECO_STREAM_OPERATOR(stream_t, member, sep)\
public:\
inline stream_t& append(double v, int prec, bool percent)\
{\
	eco::Double fmt(v, prec, percent);\
	member.append(fmt.c_str(), fmt.size());\
	return *this;\
}\
inline stream_t& operator<<(IN bool v)\
{\
	(*this) << eco::cast<std::string>(v).c_str();\
	return *this;\
}\
inline stream_t& operator<<(IN char v)\
{\
	member.append(1, v);\
	return *this;\
}\
inline stream_t& operator<<(IN int8_t v)\
{\
	Integer<int8_t> str(v);\
	member.append(str, str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN uint8_t v)\
{\
	Integer<uint8_t> str(v);\
	member.append(str, str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN int16_t v)\
{\
	Integer<int16_t> str(v);\
	member.append(str, str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN uint16_t v)\
{\
	Integer<uint16_t> str(v);\
	member.append(str, str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN int32_t v)\
{\
	Integer<int32_t> str(v);\
	member.append(str, str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN uint32_t v)\
{\
	Integer<uint32_t> str(v);\
	member.append(str, str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN int64_t v)\
{\
	Integer<int64_t> str(v);\
	member.append(str, str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN uint64_t v)\
{\
	Integer<uint64_t> str(v);\
	member.append(str, str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN long v)\
{\
	return operator<<(int32_t(v));\
}\
inline stream_t& operator<<(IN unsigned long v)\
{\
	return operator<<(uint32_t(v));\
}\
inline stream_t& operator<<(IN double v)\
{\
	Double str(v);\
	member.append(str, str.size());\
	return *this;\
}\
inline stream_t& operator<<(IN const char* v)\
{\
	member.append(v, (uint32_t)strlen(v));\
	return *this;\
}\
template<typename T>\
inline stream_t& operator<(IN T v)\
{\
	return *this << v;\
}\
template<typename T>\
inline stream_t& operator<=(IN T v)\
{\
	return *this << sep << v;\
}\
inline stream_t& operator<<(IN const eco::Bytes& v)\
{\
	member.append(v.c_str(), v.size());\
	return *this;\
}\
ECO_STREAM_OPERATOR_REF(stream_t, eco::Bytes, sep)\
inline stream_t& operator<<(IN const eco::String& v)\
{\
	member.append(v.c_str(), v.size());\
	return *this;\
}\
ECO_STREAM_OPERATOR_REF(stream_t, eco::String, sep)\
inline stream_t& operator<<(IN const std::string& v)\
{\
	member.append(v.c_str(), (uint32_t)v.size());\
	return *this;\
}\
ECO_STREAM_OPERATOR_REF(stream_t, std::string, sep)\
ECO_STREAM_OPERATOR_TPL(stream_t, eco::GroupT<T>, '(', ')')\
ECO_STREAM_OPERATOR_TPL(stream_t, eco::SquareT<T>,'[', ']')\
ECO_STREAM_OPERATOR_TPL(stream_t, eco::BraceT<T>, '{', '}')


#undef max
#undef min
const std::string empty_str;
////////////////////////////////////////////////////////////////////////////////
// check the string is empty.
inline bool empty(IN const char* v)
{
	return v[0] == 0;
}

// clear the string data.
inline void clear(OUT char* v)
{
	v[0] = 0;
}

// get string end size.
inline size_t fit_size(IN const char* str, IN const size_t size)
{
	size_t x = 0;
	size_t i = size - 1;
	for (; i != -1 && str[i] == 0; --i) ++x;
	if (i == 0 && str[i] == 0) ++x;
	return x;
}

// remove string end empty.
inline void fit(OUT std::string& str)
{
	size_t x = eco::fit_size(str.c_str(), str.size());
	str.resize(str.length() - x);
}


////////////////////////////////////////////////////////////////////////////////
class Bytes
{
public:
	const char*	m_data;
	uint32_t	m_size;

public:
	inline Bytes() : m_data(nullptr), m_size(0)
	{}

	inline Bytes(const char* data, uint32_t size = -1)
		: m_data(data), m_size(size)
	{
		if (m_size == -1 && data != nullptr)
			m_size = static_cast<uint32_t>(strlen(data));
	}

	inline const char* c_str() const
	{
		return m_data;
	}

	inline const uint32_t size() const
	{
		return m_size;
	}

	inline void clear()
	{
		m_data = nullptr;
		m_size = 0;
	}

	inline bool null() const
	{
		return m_data == nullptr;
	}
};
inline eco::Bytes func(IN const char* full_func_name)
{
	uint32_t len = (uint32_t)strlen(full_func_name);
	uint32_t end = eco::find_last(full_func_name, len, '(');
	if (end == -1) end = len;
	uint32_t start = eco::find_last(full_func_name, end, ':');
	start = (start == -1) ? 0 : start + 1;
	return eco::Bytes(full_func_name + start, end - start);
}
inline const char* clss(IN const char* full_clss_name)
{
	uint32_t end = (uint32_t)strlen(full_clss_name);
	uint32_t start = eco::find_last(full_clss_name, end, ':');
	start = (start == -1) ? 0 : start + 1;
	return (full_clss_name + start);
}


////////////////////////////////////////////////////////////////////////////////
template<typename T>
class GroupT	// ()
{
public:
	inline GroupT(IN const T& v) : value(v) {}
	const T& value;
};
template<typename T>
inline GroupT<T> group(IN const T& v)
{
	return GroupT<T>(v);
}

template<typename T>
class SquareT	// []
{
public:
	inline SquareT() {}
	inline SquareT(IN const T& v) : value(v) {}
	const T& value;
};
template<typename T>
inline SquareT<T> square(IN const T& v)
{
	return SquareT<T>(v);
}

template<typename T>
class BraceT	// {}
{
public:
	inline BraceT() {}
	inline BraceT(IN const T& v) : value(v) {}
	const T& value;
};
template<typename T>
inline BraceT<T> brace(IN const T& v)
{
	return BraceT<T>(v);
}


////////////////////////////////////////////////////////////////////////////////
class String
{
	ECO_NONCOPYABLE(String);
	ECO_STREAM_OPERATOR(String, (*this), ' ');
public:
	inline uint32_t size() const
	{
		return m_size;
	}

	inline uint32_t capacity() const
	{
		return m_capacity;
	}

	inline bool null() const
	{
		return m_data == 0;
	}

	inline bool empty() const
	{
		return eco::empty(c_str());
	}

	inline const char* c_str() const
	{
		return m_data == nullptr ? "" : m_data;
	}

	inline const char* find(IN const char* v) const
	{
		return eco::find(m_data, v);
	}

	inline uint32_t find_reverse(
		IN const char* sub_str,
		IN uint32_t rstart = -1,
		IN uint32_t rend = -1) const
	{
		return eco::find_reverse(m_data, m_size, sub_str, rstart, rend);
	}

public:
	inline String()	: m_data(nullptr), m_size(0), m_capacity(0)
	{}

	explicit inline String(IN uint32_t siz, IN bool reserved = false)
		: m_data(nullptr), m_size(0), m_capacity(0)
	{
		if (reserved)
			reserve(siz);
		else
			resize(siz);
	}

	explicit inline String(IN const char* v)
		: m_data(nullptr), m_size(0), m_capacity(0)
	{
		assign(v);
	}

	explicit inline String(IN const char* v, uint32_t size)
		: m_data(nullptr), m_size(0), m_capacity(0)
	{
		assign(v, size);
	}

	explicit inline String(IN const std::string& v)
		: m_data(nullptr), m_size(0), m_capacity(0)
	{
		assign(v.c_str(), (uint32_t)v.size());
	}

	inline String(IN String&& v)
		: m_data(v.m_data)
		, m_size(v.m_size)
		, m_capacity(v.m_capacity)
	{
		v.m_data = nullptr;
		v.m_size = 0;
		v.m_capacity = 0;
	}

	inline ~String()
	{
		release();
	}

	inline void move_from(IN String& v)
	{
		release();
		m_data = v.m_data;
		m_size = v.m_size;
		m_capacity = v.m_capacity;
		v.m_data = nullptr;
		v.m_size = 0;
		v.m_capacity = 0;
	}

	inline String& operator=(IN String&& v)
	{
		release();
		m_data = v.m_data;
		m_size = v.m_size;
		m_capacity = v.m_capacity;
		v.m_data = nullptr;
		v.m_size = 0;
		v.m_capacity = 0;
		return *this;
	}

	inline String& operator=(IN const char* v)
	{
		assign(v);
		return *this;
	}

	inline String& operator=(IN const std::string& v)
	{
		assign(v);
		return *this;
	}

	inline char& operator[](uint32_t pos)
	{
		return m_data[pos];
	}

	inline const char& operator[](uint32_t pos) const
	{
		return m_data[pos];
	}

	inline void swap(IN String& v)
	{
		std::swap(m_data, v.m_data);
		std::swap(m_size, v.m_size);
		std::swap(m_capacity, v.m_capacity);
	}

	inline void assign(IN const char* d)
	{
		assign(d, static_cast<uint32_t>(strlen(d)));
	}
	inline void assign(IN const char* d, IN uint32_t s)
	{
		resize(s);
		if (s > 0) memcpy(&m_data[0], d, s);
	}
	inline void assign(IN const std::string& v)
	{
		assign(v.c_str(), (uint32_t)v.size());
	}

	inline void append(IN const char* d)
	{
		append(d, static_cast<uint32_t>(strlen(d)));
	}
	inline void append(IN const char* d, IN uint32_t s)
	{
		if (s == 0) return;
		reserve(m_size + s);
		memcpy(&m_data[m_size], d, s);
		m_size += s;
		m_data[m_size] = 0;
	}
	inline void append(IN char c)
	{
		reserve(m_size + 1);
		m_data[m_size++] = c;
		m_data[m_size] = 0;
	}
	inline void append(IN uint32_t s, IN char c)
	{
		if (s == 1) return append(c);
		if (s == 0) return;
		reserve(m_size + s);
		memset(&m_data[m_size], c, s);
		m_size += s;
		m_data[m_size] = 0;
	}

	// compatible with "Buffer".
	inline uint32_t force_append(IN const char* buf, IN uint32_t buf_size)
	{
		append(buf, buf_size);
		return buf_size;
	}
	// compatible with "Buffer".
	inline uint32_t force_append(IN uint32_t s, IN char c)
	{
		append(s, c);
		return s;
	}

	inline void erase(uint32_t pos, uint32_t count)
	{
		if (count > 0 && m_size > pos)
		{
			uint32_t siz = m_size - pos;
			if (count < siz) siz = count;
			count = m_size - pos - siz;
			if (count > 0) memcpy(&m_data[pos], &m_data[pos + siz], count);
			m_size -= siz;
			m_data[m_size] = 0;
		}
	}

	inline void resize(IN uint32_t s)
	{
		reserve(s);
		m_size = s;
		if (m_data != nullptr)
			m_data[m_size] = 0;
	}

	inline void reserve(IN uint32_t c)
	{
		if (m_capacity < c)
		{
			// exponential growth.
			uint32_t old_size = m_size;
			uint32_t new_size = m_capacity * 2;
			if (new_size < c)
			{
				new_size = c;
			}

			// keep old value.
			char* new_data = (char*)HeapOperators::
				operator new[](new_size + 1);
			if (old_size > 0)
			{
				memcpy(new_data, m_data, old_size);
			}
			new_data[old_size] = 0;
			
			release();
			m_data = new_data;
			m_size = old_size;
			m_capacity = new_size;
		}
	}

	inline void fit()
	{
		uint32_t x = uint32_t(eco::fit_size(m_data, m_size) - 1);
		if (x > 0) resize(m_size - x);
	}

	inline void clear()
	{
		resize(0);
	}

	inline void release()
	{
		if (m_data != nullptr)
		{
			HeapOperators::operator delete[](m_data);
			m_data = nullptr;
		}
		m_size = 0;
		m_capacity = 0;
	}

	inline Bytes bytes() const
	{
		return Bytes(m_data, m_size);
	}

private:
	char* m_data;
	uint32_t m_size;
	uint32_t m_capacity;
};


////////////////////////////////////////////////////////////////////////////////
template<uint32_t siz = 160>
class Buffer
{
	ECO_STREAM_OPERATOR(Buffer, (*this), ' ');
public:
	inline Buffer()
	{
		assert(siz > 0);
		clear();
	}

	inline explicit Buffer(const char* msg)
	{
		assert(siz > 0);
		clear();
		append(msg);
	}

	inline operator const char*() const
	{
		return m_data;
	}

	inline const char* c_str() const
	{
		return m_data;
	}

	inline char* buffer(IN const uint32_t pos = 0)
	{
		return &m_data[pos];
	}

	inline uint32_t size() const
	{
		return m_size;
	}

	inline static uint32_t capacity()
	{
		return siz - 1;	// last char = '\0'
	}

	inline uint32_t avail() const
	{
		return capacity() - m_size;
	}

	inline int is_avail(IN int new_size) const
	{
		return avail() >= new_size;
	}

	inline int is_full() const
	{
		return avail() == 0;
	}

	template<uint32_t size_n>
	inline uint32_t append(IN const Buffer<size_n>& buf)
	{
		return append(buf.m_data, buf.size());
	}

	inline uint32_t append(IN const eco::Bytes& buf)
	{
		return append(buf.c_str(), buf.size());
	}

	inline uint32_t append(IN const std::string& buf)
	{
		return append(buf.c_str(), buf.size());
	}

	inline uint32_t append(IN const char* buf)
	{
		uint32_t init_size = m_size;
		for (; *buf != '\0' && avail() > 0; ++buf)
			m_data[m_size++] = *buf;
		m_data[m_size] = '\0';
		return m_size - init_size;
	}

	// append string.
	inline uint32_t append(IN const char* buf, IN uint32_t size)
	{
		uint32_t append_size = (avail() > size) ? size : avail();
		if (append_size > 0)
		{
			memcpy(&m_data[m_size], buf, append_size);
			m_size += append_size;
			m_data[m_size] = '\0';
		}
		return append_size;
	}

	// append char.
	inline uint32_t append(IN char c)
	{
		if (avail() >= 1)
		{
			m_data[m_size++] = c;
			m_data[m_size] = 0;
			return 1;
		}
		return 0;
	}
	inline uint32_t append(IN uint32_t siz, IN char c)
	{
		if (siz == 1) return append(c);
		if (avail() >= siz)
		{
			memset(&m_data[m_size], c, siz);
			m_size += siz;
			m_data[m_size] = 0;
			return siz;
		}
		return 0;
	}

	// force append string.
	inline uint32_t force_append(IN const char* buf, IN uint32_t siz)
	{
		force(siz);
		return append(buf, siz);
	}

	// append char.
	inline uint32_t force_append(IN uint32_t siz, IN char c)
	{
		force(siz);
		return append(siz, c);
	}

	// erase a space for force content using.
	inline void force(IN uint32_t siz)
	{
		int32_t pop_size = static_cast<int32_t>(siz - avail());
		if (pop_size > 0) pop(pop_size);
	}

	inline Buffer& operator=(IN const char* buf)
	{
		clear();
		append(buf);
		return *this;
	}

	inline Buffer& assign(IN const char* buf, IN uint32_t buf_size)
	{
		clear();
		append(buf, buf_size);
		return *this;
	}

	void clear()
	{
		m_size = 0;
		m_data[m_size] = 0;
	}

	void pop(IN uint32_t size)
	{
		int32_t new_size = static_cast<int32_t>(m_size - size);
		m_size = (new_size > 0) ? new_size : 0;
		m_data[m_size] = 0;
	}

private:
	char m_data[siz];
	uint32_t  m_size;
};


////////////////////////////////////////////////////////////////////////////////
class Error : public std::exception
{
	ECO_STREAM_OPERATOR(Error, (*this), ';');
public:
	inline int id() const
	{
		return this_thread::error_id();
	}

	inline const char* path() const
	{
		return this_thread::error_path();
	}

	inline const char* value() const
	{
		return this_thread::error_value();
	}

	virtual const char* what() const override
	{
		return this_thread::error_value();
	}

	// whether has error.
	inline operator bool() const
	{
		return this_thread::error_id() != 0
			|| !eco::empty(this_thread::error_path());
	}

	inline Error& key(int id)
	{
		this_thread::error_key(id);
		return *this;
	}
	inline Error& key(const char* v)
	{
		this_thread::error_key(v);
		return *this;
	}
	inline Error& add(const char* v)
	{
		this_thread::error_add(v);
		return *this;
	}

	inline Error& key_throw(int id)
	{
		this_thread::error_key(id);
		return *this;
	}
	inline Error& key_throw(const char* v)
	{
		this_thread::error_val(v);
		return *this;
	}

	// path: add pro: "a/b" / "c" = "a/b/c";
	inline Error& operator / (const char* v)
	{
		append(1, '/');
		append(v, (uint32_t)strlen(v));
		return *this;
	}

	inline void clear()
	{
		this_thread::error_clear();
	}

private:
	// append char.
	inline void append(IN uint32_t siz, IN char c)
	{
		this_thread::error_append(siz, c);
	}

	// append string.
	inline void append(IN const char* buf, IN uint32_t siz)
	{
		this_thread::error_append(buf, siz);
	}
};
template<typename StreamT>
StreamT& operator<<(OUT StreamT& stream, IN const eco::Error& e)
{
	const char* p = e.path();
	stream <= e.value();
	if (e.id() != 0) stream <= '#' < e.id();
	if (!eco::empty(p)) stream <= '#' < p;
	return stream;
}


////////////////////////////////////////////////////////////////////////////////
template<uint32_t size = 160, typename buffer_t = Buffer<size>>
class Format
{
public:
	inline explicit Format(IN const char* v = nullptr)
		: m_format(v), m_cur_pos(0)
	{}

	inline void reset(IN const char* v)
	{
		m_format = v;
		m_cur_pos = 0;
		m_stream.clear();
	}

	inline Format& operator()(IN const char* v)
	{
		reset(v);
		return *this;
	}

	inline bool next()
	{
		if (m_cur_pos == -1) return false;

		const char* str = m_format + m_cur_pos;
		m_cur_pos = eco::find_first(str, '%');
		if (m_cur_pos == -1)
		{
			if (str != m_format) m_stream.append(str);
			return false;
		}
		m_stream.append(str, m_cur_pos++);
		m_cur_pos += int(str - m_format);
		return true;
	}

	template<typename T>
	inline Format& operator%(IN const T v)
	{
		if (next()) m_stream << v;
		return *this;
	}
	inline Format& operator%(IN const std::string& v)
	{
		if (next()) m_stream << v;
		return *this;
	}

	inline Format& arg(IN const char* v)
	{
		if (next()) m_stream.append(v);
		return *this;
	}

	inline Format& arg(IN const char* v, IN uint32_t n)
	{
		if (next()) m_stream.append(v, n);
		return *this;
	}

	inline Format& arg(IN char c, IN const char* v)
	{
		uint32_t pos = eco::find_first(v, c);
		while (pos != std::string::npos)
		{
			arg(v, pos);
			v += pos + 1;
			pos = eco::find_first(v, c);
		}
		arg(v);
		return *this;
	}

	inline operator const char*() const
	{
		return c_str();
	}

	inline const char* c_str() const
	{
		if (m_cur_pos == 0)
		{
			return m_format;
		}
		if (m_cur_pos != -1)
		{
			m_stream.append(m_format + m_cur_pos);
			m_cur_pos = -1;
		}
		return m_stream.c_str();
	}

private:
	const char* m_format;
	mutable uint32_t m_cur_pos;
	mutable buffer_t m_stream;
};
typedef Format<0, String> FormatX;


////////////////////////////////////////////////////////////////////////////////
class StringAny
{
	ECO_STREAM_OPERATOR(StringAny, m_value, ' ');
public:
	inline StringAny(IN const char*);
	inline StringAny(IN StringAny&&);
	inline StringAny(IN const StringAny&);
	inline StringAny& operator=(IN const char*);
	inline StringAny& operator=(IN StringAny&&);
	inline StringAny& operator=(IN const StringAny&);
	inline bool empty() const;
	inline const char* c_str() const;
	inline std::string& str();
	inline eco::ValueType type() const;

public:
	inline bool operator==(IN const StringAny&) const;
	inline bool operator==(IN const char*) const;
	inline bool operator==(IN const std::string& v) const
	{
		return (operator==(v.c_str()));
	}
	template<typename string_t>
	inline bool operator!=(IN const string_t v) const
	{
		return !(operator==(v));
	}

public:
	inline StringAny();
	inline StringAny(IN bool v);
	inline StringAny(IN char v);
	inline StringAny(IN unsigned char v);
	inline StringAny(IN short v);
	inline StringAny(IN unsigned short v);
	inline StringAny(IN int v);
	inline StringAny(IN unsigned int v);
	inline StringAny(IN long v);
	inline StringAny(IN unsigned long v);
	inline StringAny(IN int64_t v);
	inline StringAny(IN uint64_t v);
	inline StringAny(IN double v, IN int precision = -1);

public:
	inline operator const char*() const;
	inline operator const char() const;
	inline operator const unsigned char() const;
	inline operator const short() const;
	inline operator const unsigned short() const;
	inline operator const int() const;
	inline operator const unsigned int() const;
	inline operator const long() const;
	inline operator const unsigned long() const;
	inline operator const int64_t() const;
	inline operator const uint64_t() const;
	inline operator const float() const;
	inline operator const double() const;
	inline operator const bool() const;

private:
	ValueType m_vtype;
	std::string m_value;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif