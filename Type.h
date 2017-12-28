#ifndef ECO_TYPE_H
#define ECO_TYPE_H
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
#include <functional>
#include <iostream>
#include <unordered_map>
#include <limits>
#include <map>
#include <eco/Cast.h>


namespace eco{;

#undef max
#undef min
////////////////////////////////////////////////////////////////////////////////
// closure method.
typedef std::function<void(void)> Closure;

// function general result.
enum Result
{
	ok				=  0,
	fail			= -1,
	error			= -2,
	timeout			= -3,
};


////////////////////////////////////////////////////////////////////////////////
// when app exit with exception, use _getch to show a console windows.
// and show the exception error.
inline char getch_exit()
{
	char ch = 0;
	std::cout << "please input a char to end." << std::endl;
	std::cin >> ch;
	return ch;
}

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


// thread local data.
#ifdef ECO_WIN
#	define EcoThreadLocal __declspec(thread)
#endif
#ifdef ECO_LINUX
#	define EcoThreadLocal __thread
#endif


////////////////////////////////////////////////////////////////////////////////
template<typename Object>
class Movable
{
public:
	inline Movable()
	{}

	inline Movable(IN Object& v)
		: m_object(std::move(v))
	{}

	inline Movable(IN Movable&& v)
		: m_object(std::move(v.m_object))
	{}

	inline Movable(IN const Movable& v)
		: m_object(std::move(v.m_object))
	{}

	inline Movable& operator=(IN const Movable& v)
	{
		m_object = std::move(v.m_object);
		return *this;
	}

	inline operator Object& ()
	{
		return m_object;
	}

	inline operator const Object& () const
	{
		return m_object;
	}

private:
	mutable Object m_object;
};

template<typename Object>
inline eco::Movable<Object> move(IN Object& obj)
{
	return eco::Movable<Object>(obj);
}


////////////////////////////////////////////////////////////////////////////////
class Bytes
{
public:
	char*		m_data;
	uint32_t	m_size;

public:
	inline Bytes() : m_data(nullptr), m_size(0)
	{}

	inline void clear()
	{
		m_data = nullptr;
		m_size = 0;
	}
};


////////////////////////////////////////////////////////////////////////////////
class String
{
	ECO_NONCOPYABLE(String);
public:
	inline String()
		: m_data(nullptr)
		, m_size(0)
		, m_capacity(0)
	{}

	explicit inline String(IN uint32_t siz, IN bool reserved = false)
		: m_data(nullptr)
		, m_size(0)
		, m_capacity(0)
	{
		if (reserved)
			reserve(siz);
		else
			resize(siz);
	}

	explicit inline String(IN const char* v)
		: m_data(nullptr)
		, m_size(0)
		, m_capacity(0)
	{
		asign(v);
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

	inline String& operator=(IN String&& v)
	{
		m_data = v.m_data;
		m_size = v.m_size;
		m_capacity = v.m_capacity;
		v.m_data = nullptr;
		v.m_size = 0;
		v.m_capacity = 0;
		return *this;
	}

	inline ~String()
	{
		release();
	}

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

	inline const char* c_str() const
	{
		return m_data;
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

	inline void asign(IN const char* d)
	{
		asign(d, static_cast<uint32_t>(strlen(d)));
	}
	inline void asign(IN const char* d, IN uint32_t s)
	{
		resize(s);
		memcpy(&m_data[0], d, s);
	}

	inline void append(IN const char* d)
	{
		append(d, static_cast<uint32_t>(strlen(d)));
	}
	inline void append(IN const char* d, IN uint32_t s)
	{
		reserve(m_size + s);
		memcpy(&m_data[m_size], d, s);
		m_size += s;
		m_data[m_size] = 0;
	}
	inline void append(IN uint32_t s, IN const char c)
	{
		reserve(m_size + s);
		memset(&m_data[m_size], c, s);
		m_size += s;
		m_data[m_size] = 0;
	}
	inline void append(IN const char c)
	{
		append(1, c);
	}
	inline void append(IN const eco::String& str)
	{
		append(str.m_data, str.m_size);
	}
	inline void append(IN const eco::Bytes& str)
	{
		append(str.m_data, str.m_size);
	}

	inline void resize(IN uint32_t s)
	{
		reserve(s);
		if (s > 0)
		{
			m_size = s;
			m_data[m_size] = 0;
		}
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
			char* new_data = new char[new_size + 1];
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

	inline void release()
	{
		if (m_data != nullptr)
		{
			delete[]m_data;
			m_data = nullptr;
		}
		m_size = 0;
		m_capacity = 0;
	}

	inline const char* find(IN const char* v) const
	{
		return eco::find(m_data, v);
	}

	inline const char* find_reverse(
		IN const char* v,
		IN const uint32_t find_end = 0) const
	{
		uint32_t len = strlen(v);
		if (m_size < find_end + len)
			return nullptr;

		const char* end = &m_data[find_end];
		const char* start = &m_data[m_size - len];
		for (; start != end; --start)
		{
			if (eco::find_cmp(start, v))
				return start;
		}
		return eco::find_cmp(start, v) ? start : nullptr;
	}

private:
	inline char* move()
	{
		char* d = m_data;

		m_data = nullptr;
		m_size = 0;
		m_capacity = 0;
		return d;
	}

private:
	char* m_data;
	uint32_t m_size;
	uint32_t m_capacity;
};


////////////////////////////////////////////////////////////////////////////////
template<uint32_t fix_size>
class FixBuffer
{
public:
	inline FixBuffer()
	{
		assert(fix_size > 0);
		clear();
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
		return m_cur_size;
	}

	inline static uint32_t capacity()
	{
		return fix_size - 1;	// last char = '\0'
	}

	inline uint32_t avail() const
	{
		return capacity() - m_cur_size;
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
	inline uint32_t append(IN const FixBuffer<size_n>& buf)
	{
		return append(buf, buf.size());
	}

	inline uint32_t append(IN const char* buf)
	{
		return append(buf, static_cast<uint32_t>(strlen(buf)));
	}

	// append string.
	inline uint32_t append(IN const char* buf, IN uint32_t buf_size)
	{
		uint32_t append_size = (avail() > buf_size) ? buf_size : avail();
		if (append_size > 0)
		{
			memcpy(&m_data[m_cur_size], buf, append_size);
			m_cur_size += append_size;
			m_data[m_cur_size] = '\0';
		}
		return append_size;
	}

	// append char.
	inline uint32_t append(IN const char v)
	{
		if (avail() >= 1)
		{
			m_data[m_cur_size] = v;
			m_cur_size += 1;
			m_data[m_cur_size] = '\0';
			return 1;
		}
		return 0;
	}

	// force append string.
	inline uint32_t force_append(IN const char* buf, IN uint32_t buf_size)
	{
		force(buf_size);
		return append(buf, buf_size);
	}

	// append char.
	inline uint32_t force_append(IN const char v)
	{
		force(1);
		return append(v);
	}

	// erase a space for force content using.
	inline void force(IN const uint32_t size)
	{
		int32_t erase_size = static_cast<int32_t>(size - avail());
		if (erase_size > 0) {
			erase(erase_size);
		}
	}

	inline FixBuffer& operator=(IN const char* buf)
	{
		clear();
		append(buf);
		return *this;
	}

	inline FixBuffer& assign(IN const char* buf, IN uint32_t buf_size)
	{
		clear();
		append(buf, buf_size);
		return *this;
	}

	void clear()
	{
		m_cur_size = 0;
		m_data[m_cur_size] = 0;
	}

	void erase(IN const uint32_t size)
	{
		int32_t new_size = static_cast<int32_t>(m_cur_size - size);
		m_cur_size = (new_size > 0) ? new_size : 0;
		m_data[m_cur_size] = 0;
	}

private:
	char m_data[fix_size];
	uint32_t  m_cur_size;
};


////////////////////////////////////////////////////////////////////////////////
template<int size>
class FixStream
{
public:
	typedef FixBuffer<size> Buffer;

	inline FixStream& operator<<(IN bool v)
	{
		(*this) << eco::cast<std::string>(v).c_str();
		return *this;
	}
	inline FixStream& operator<<(IN char v)
	{
		m_buffer.append(v);
		return *this;
	}
	inline FixStream& operator<<(IN unsigned char v)
	{
		m_buffer.append(static_cast<char>(v));
		return *this;
	}
	inline FixStream& operator<<(IN int16_t v)
	{
		Integer<int16_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline FixStream& operator<<(IN uint16_t v)
	{
		Integer<uint16_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline FixStream& operator<<(IN int32_t v)
	{
		Integer<int32_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline FixStream& operator<<(IN uint32_t v)
	{
		Integer<uint32_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline FixStream& operator<<(IN long v)
	{
		return operator<<(int32_t(v));
	}
	inline FixStream& operator<<(IN unsigned long v)
	{
		return operator<<(uint32_t(v));
	}
	inline FixStream& operator<<(IN int64_t v)
	{
		Integer<int64_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline FixStream& operator<<(IN uint64_t v)
	{
		Integer<uint64_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline FixStream& operator<<(IN double v)
	{
		Double str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline FixStream& operator<<(IN const char* v)
	{
		m_buffer.append(v);
		return *this;
	}
	inline FixStream& operator<<(IN const std::string& v)
	{
		m_buffer.append(v.c_str(), static_cast<uint32_t>(v.size()));
		return *this;
	}

	template<typename Object>
	inline FixStream& operator>>(IN Object& v)
	{
		v >> (*this);
		return *this;
	}

	inline Buffer& buffer()
	{
		return m_buffer;
	}
	inline const Buffer& get_buffer() const
	{
		return m_buffer;
	}

	inline operator const char*() const
	{
		return m_buffer.c_str();
	}

	inline const char* c_str() const
	{
		return m_buffer.c_str();
	}

private:
	Buffer m_buffer;
};
#define EcoStr eco::FixStream<256>()
#define EcoString(len) eco::FixStream<len>()


////////////////////////////////////////////////////////////////////////////////
template<
	typename KeyOne, typename KeyTwo,
	typename OneMap = std::unordered_map<KeyOne, KeyTwo>,
	typename TwoMap = std::unordered_map<KeyTwo, KeyOne> >
class BidiMap
{
public:
	OneMap m_one_map;
	TwoMap m_two_map;

	inline void set_value(
		IN const KeyOne& key_one,
		IN const KeyTwo& key_two)
	{
		m_one_map[key_one] = key_two;
		m_two_map[key_two] = key_one;
	}
};


////////////////////////////////////////////////////////////////////////////////
class BindV
{
public:
	inline BindV(IN std::string& val)
		: m_value(std::move(val))
	{}

	inline operator const char*() const
	{
		return m_value.c_str();
	}

	std::string m_value;
};


////////////////////////////////////////////////////////////////////////////////
inline bool equal(
	IN const double v1,
	IN const double v2,
	IN const double p = std::numeric_limits<double>::epsilon())
{
	return std::fabs(v1 - v2) < p;
}

inline bool is_zero(IN const double v)
{
	return std::fabs(v) < std::numeric_limits<double>::epsilon();
}

inline bool is_number(IN const char v)
{
	return v >= '0' && v <= '9';
}
inline bool is_number(IN const char* v)
{
	for (const char* c = v; *c != 0; ++c)
	{
		if (!is_number(*c))
			return false;
	}
	return true;
}
inline bool is_normal(IN const double v)
{
	return v > -std::numeric_limits<double>::max()
		&& v < std::numeric_limits<double>::max();
}
inline bool is_positive(IN const double v)
{
	return v > 0.0 && v < std::numeric_limits<double>::max();
}
inline bool is_negative(IN const double v)
{
	return v < 0.0 && v > -std::numeric_limits<double>::max();
}
inline bool is_nan(IN const double v)
{
	return _isnan(v) > 0;
}
inline bool is_infinity(IN const double v)
{
	return v == std::numeric_limits<double>::infinity();
}
inline void set_double(OUT double& d, IN const double v, IN int def = -1)
{
	d = (eco::is_nan(v) || eco::is_infinity(v)) ? def : v;
}
inline void set_double(OUT int& d, IN const double v, IN int def = -1)
{
	d = (eco::is_nan(v) || eco::is_infinity(v)) ? def : (int)v;
}


////////////////////////////////////////////////////////////////////////////////
#define EcoFmti(instance) '[' << instance << ']'
const std::string empty_str;


////////////////////////////////////////////////////////////////////////////////
template<typename StringSet>
void split(
	OUT StringSet& set,
	IN  const std::string& str,
	IN  const char ch)
{
	size_t start_pos = 0;
	size_t end_pos = str.find(ch);
	while (end_pos != std::string::npos)
	{
		set.push_back(str.substr(start_pos, end_pos - start_pos));
		start_pos = end_pos + 1;
		end_pos = str.find(ch, start_pos);
	}
	if (start_pos < str.size())
	{
		set.push_back(str.substr(start_pos, str.size() - start_pos));
	}
}


////////////////////////////////////////////////////////////////////////////////
}//
#endif