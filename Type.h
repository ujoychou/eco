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
#include <limits>
#include <iostream>
#include <unordered_map>
#include <eco/Cast.h>
#include <eco/ExportApi.h>


ECO_NS_BEGIN(eco);
#undef max
#undef min
const std::string empty_str;
////////////////////////////////////////////////////////////////////////////////
// closure method.
typedef std::function<void(void)> Closure;

// function general result.
enum 
{
	ok				=  0,
	fail			= -1,
	error			= -2,
	timeout			= -3,
};
typedef int Result;


// judge integer big/little endian.
inline bool big_endian()
{
	uint32_t v = 0;
	char* b = (char*)&v;
	b[0] = 0x01;
	return (v & 0xFF) == 0;
}
inline bool little_endian()
{
	return !big_endian();
}


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
template<
	typename KeyOne, typename KeyTwo,
	typename OneMap = std::unordered_map<KeyOne, KeyTwo>,
	typename TwoMap = std::unordered_map<KeyTwo, KeyOne> >
class BidiMap
{
public:
	OneMap one;
	TwoMap two;

	inline void set_value(
		IN const KeyOne& key_one,
		IN const KeyTwo& key_two)
	{
		one[key_one] = key_two;
		two[key_two] = key_one;
	}

	inline KeyTwo* get_by_one(IN const KeyOne& key)
	{
		auto it = one.find(key);
		return it != one.end() ? &it->second : nullptr;
	}
	inline const KeyTwo* get_by_one(IN const KeyOne& key) const
	{
		auto it = one.find(key);
		return it != one.end() ? &it->second : nullptr;
	}

	inline KeyOne* get_by_two(IN const KeyTwo& key)
	{
		auto it = two.find(key);
		return it != two.end() ? &it->second : nullptr;
	}
	inline const KeyOne* get_by_two(IN const KeyTwo& key) const
	{
		auto it = two.find(key);
		return it != two.end() ? &it->second : nullptr;
	}
};


////////////////////////////////////////////////////////////////////////////////
class Bytes
{
public:
	const char*	m_data;
	uint32_t	m_size;

public:
	inline Bytes() : m_data(nullptr), m_size(0)
	{}

	inline explicit Bytes(
		IN const char* data,
		IN const uint32_t size = -1)
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
class GroupT
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
class SquareT
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
class BraceT
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
class Brace;
inline const Brace* brace() { return 0; }
class End;
inline const End* end() { return 0; }


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

	explicit inline String(IN const std::string& v)
		: m_data(nullptr)
		, m_size(0)
		, m_capacity(0)
	{
		asign(v.c_str(), v.size());
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
	inline void asign(IN const std::string& v)
	{
		asign(v.c_str(), v.size());
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
	inline void append(IN const std::string& v)
	{
		append(v.c_str(), v.size());
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

	// compatible with "FixBuffer".
	inline uint32_t force_append(IN const char* buf, IN uint32_t buf_size)
	{
		append(buf, buf_size);
		return buf_size;
	}
	// compatible with "FixBuffer".
	inline uint32_t force_append(IN const char v)
	{
		append(v);
		return 1;
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
		uint32_t len = (uint32_t)strlen(v);
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
		uint32_t init_size = m_cur_size;
		for (; *buf != '\0' && avail() > 0; ++buf)
			m_data[m_cur_size++] = *buf;
		m_data[m_cur_size] = '\0';
		return m_cur_size - init_size;
	}

	// append string.
	inline uint32_t append(IN const char* buf, IN uint32_t size)
	{
		uint32_t append_size = (avail() > size) ? size : avail();
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
class Error;
template<typename Buffer>
class StreamT
{
public:
	inline StreamT() : m_flag(0), m_status(0)
	{}

	inline StreamT& operator<<(IN const bool v)
	{
		(*this) << eco::cast<std::string>(v).c_str();
		return *this;
	}
	inline StreamT& operator<<(IN const char v)
	{
		m_buffer.append(v);
		return *this;
	}
	inline StreamT& operator<<(IN const unsigned char v)
	{
		m_buffer.append(static_cast<char>(v));
		return *this;
	}
	inline StreamT& operator<<(IN const int16_t v)
	{
		Integer<int16_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline StreamT& operator<<(IN const uint16_t v)
	{
		Integer<uint16_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline StreamT& operator<<(IN const int32_t v)
	{
		Integer<int32_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline StreamT& operator<<(IN const uint32_t v)
	{
		Integer<uint32_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline StreamT& operator<<(IN const long v)
	{
		return operator<<(int32_t(v));
	}
	inline StreamT& operator<<(IN unsigned long v)
	{
		return operator<<(uint32_t(v));
	}
	inline StreamT& operator<<(IN const int64_t v)
	{
		Integer<int64_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline StreamT& operator<<(IN const uint64_t v)
	{
		Integer<uint64_t> str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline StreamT& operator<<(IN const double v)
	{
		Double str(v);
		m_buffer.append(str, str.size());
		return *this;
	}
	inline StreamT& operator<<(IN const char* v)
	{
		m_buffer.append(v);
		return *this;
	}
	inline StreamT& operator<<(IN const eco::String& v)
	{
		m_buffer.append(v.c_str(), static_cast<uint32_t>(v.size()));
		return *this;
	}
	inline StreamT& operator<<(IN const std::string& v)
	{
		m_buffer.append(v.c_str(), static_cast<uint32_t>(v.size()));
		return *this;
	}
	inline StreamT& operator<<(IN const eco::Bytes& v)
	{
		m_buffer.append(v.c_str(), v.size());
		return *this;
	}
	inline StreamT& operator<<(IN const eco::Error& v);
	inline StreamT& operator<=(IN const eco::Error& v);

	// append "()[]{}"
	template<typename T>
	inline StreamT& operator<<(IN const eco::GroupT<T>& v)
	{
		(*this) << '(' << v.value << ')';
		return *this;
	}
	template<typename T>
	inline StreamT& operator<<(IN const eco::SquareT<T>& v)
	{
		(*this) << '[' << v.value << ']';
		return *this;
	}
	template<typename T>
	inline StreamT& operator<<(IN const eco::BraceT<T>& v)
	{
		(*this) << '{' << v.value << '}';
		return *this;
	}

	// same with operator <<.
	template<typename T>
	inline StreamT& operator<(IN const T& v)
	{
		return (*this) << v;
	}
	// append blank space ' ';
	template<typename T>
	inline StreamT& operator<=(IN const T& v)
	{
		return (*this) << ' ' << v;
	}

public:
	enum
	{
		status_brace = 0x0001,
	};
	typedef uint16_t Status;

	inline StreamT& operator<<(IN const eco::Brace*)
	{
		(*this) << (!eco::has(m_status, status_brace) ? '{' : '}');
		eco::toggle(m_status, status_brace);
		return *this;
	}
	inline StreamT& operator<<(IN const eco::End*)
	{
		if (eco::has(m_status, status_brace)) (*this) << brace();
		return *this;
	}

public:
	// set stream flag.
	inline StreamT& operator()(IN const uint32_t flag)
	{
		eco::add(m_flag, flag);
		return *this;
	}
	inline uint16_t& flag()
	{
		return m_flag;
	}
	inline const uint16_t get_flag() const
	{
		return m_flag;
	}
	inline bool has_flag(IN const uint16_t v) const
	{
		return eco::has(m_flag, v);
	}

public:
	inline void clear()
	{
		m_buffer.clear();
	}

	inline StreamT& operator=(IN const char* buf)
	{
		m_buffer = buf;
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

	inline Bytes get_bytes() const
	{
		return Bytes((char*)m_buffer.c_str(), m_buffer.size());
	}

	inline operator const char*() const
	{
		return m_buffer.c_str();
	}

	inline const char* c_str() const
	{
		return m_buffer.c_str();
	}

	inline const uint32_t size() const
	{
		return m_buffer.size();
	}

private:
	uint16_t	m_flag;
	Status		m_status;
	Buffer		m_buffer;
};


////////////////////////////////////////////////////////////////////////////////
// eco string stream, heap memory.
const uint32_t fix_size = 256;
typedef eco::StreamT<String> Stream;
typedef eco::StreamT<FixBuffer<fix_size> > FixStream;
#define EcoStream(len) eco::StreamT<eco::FixBuffer<len> >()
#define EcoFix eco::FixStream()


////////////////////////////////////////////////////////////////////////////////
// get default Error log format.
class Error : public std::exception
{
public:
	virtual ~Error() throw() {}
	inline explicit Error(int eid = 0) : m_id(eid) {}
	inline explicit Error(const char* msg) : m_id(-1) { m_msg << msg; }
	inline explicit Error(int eid, const char* msg) : m_id(eid) { m_msg << msg; }
	inline explicit Error(const char* msg, int eid) : m_id(eid) { m_msg << msg; }
	inline explicit Error(int eid, const std::string& msg) : m_id(eid)
	{
		m_msg << msg;
	}
	inline explicit Error(const std::string& msg, int eid) : m_id(eid)
	{
		m_msg << msg;
	}

	inline void set_error(int eid, const std::string& msg)
	{
		m_id = eid;
		m_msg.buffer().clear();
		m_msg << msg;
	}
	inline void set_error(int eid, const char* msg)
	{
		m_id = eid;
		m_msg.buffer().clear();
		m_msg << msg;
	}
	inline void clear()
	{
		m_id = 0;
		m_msg.buffer().clear();
	}

	inline bool has_error() const
	{
		return m_id != 0;
	}

	// error code.
	inline Error& id(int eid)
	{
		m_id = eid;
		return *this;
	}
	inline int id() const
	{
		return m_id;
	}

	// error message.
	inline const char* message() const
	{
		return m_msg.get_buffer().c_str();
	}
	virtual const char* what() const override
	{
		return m_msg.get_buffer().c_str();
	}

	inline Error& operator()(int eid)
	{
		m_id = eid;
		return (*this);
	}
	inline Error& operator()(const char* msg)
	{
		m_msg << msg;
		return (*this);
	}
	inline operator bool() const
	{
		return m_id != 0;
	}

public:
	template<typename value_type>
	inline Error& operator<<(IN const value_type v)
	{
		m_msg << v;
		return (*this);
	}
	template<typename value_type>
	inline Error& operator<=(IN const value_type v)
	{
		m_msg <= v;
		return (*this);
	}
	inline Error& operator<=(IN const eco::String& v)
	{
		m_msg <= v;
		return (*this);
	}
	inline Error& operator<<(IN const eco::String& v)
	{
		m_msg << v;
		return (*this);
	}
	inline Error& operator<=(IN const eco::Bytes& v)
	{
		m_msg <= v;
		return (*this);
	}
	inline Error& operator<<(IN const eco::Bytes& v)
	{
		m_msg << v;
		return (*this);
	}
	inline Error& operator<=(IN const eco::Error& v)
	{
		m_msg <= v;
		return (*this);
	}
	inline Error& operator<<(IN const eco::Error& v)
	{
		m_msg << v;
		return (*this);
	}

private:
	int m_id;
	FixStream m_msg;
};


////////////////////////////////////////////////////////////////////////////////
#define ECO_THROW_1(eid) throw eco::Error(eid)
#define ECO_THROW_2(eid, emsg) throw eco::Error(eid, emsg) << ' '
#define ECO_THROW(...) ECO_MACRO(ECO_THROW_,__VA_ARGS__)
#define ECO_THROW_FUNC_1(eid) throw eco::Error(eid, __func__) << ' '
#define ECO_THROW_FUNC_2(eid, emsg) \
throw eco::Error(eid, __func__) << ' ' << emsg << ' '
#define ECO_THROW_FUNC(...) ECO_MACRO(ECO_THROW_FUNC_, __VA_ARGS__)

template<typename Buffer>
inline StreamT<Buffer>& StreamT<Buffer>::operator<<(IN const eco::Error& v)
{
	(*this) << v.message() << " #" << v.id();
	return *this;
}
template<typename Buffer>
inline StreamT<Buffer>& StreamT<Buffer>::operator<=(IN const eco::Error& v)
{
	(*this) << ' ' << v.message() << " #" << v.id();
	return *this;
}


////////////////////////////////////////////////////////////////////////////////
class Cout
{
public:
	template<typename value_t>
	inline Cout& operator<<(IN const value_t& val)
	{
		std::cout << val;
		return (*this);
	}

	inline ~Cout()
	{
		for (uint8_t i = 0; i<m_turn_line; ++i)
		{
			std::cout << std::endl;
		}
	}

	inline explicit Cout(IN const uint8_t turn_line) : m_turn_line(turn_line)
	{}

private:
	uint8_t m_turn_line;
};
// print stream message to std out auto append std::endl.
inline Cout cout(IN uint8_t turn_line = 1)
{
	return Cout(turn_line);
}
#define EcoCout eco::cout()


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
inline bool is_number_char(IN const char v)
{
	return is_number(v) || is_char(v);
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
	return int(_isnan(v)) > 0;
}
inline bool is_infinity(IN const double v)
{
	return v == std::numeric_limits<double>::infinity();
}
template<typename Number>
inline void set_double(OUT Number& d, IN const double v, IN Number def = -1)
{
	d = (eco::is_nan(v) || eco::is_infinity(v)) ? def : v;
}
inline double get_double(IN const double v, IN double def = -1)
{
	return (eco::is_nan(v) || eco::is_infinity(v)) ? def : v;
}
inline char yn(IN const bool v)
{
	return v ? 'y' : 'n';
}

// float number.
inline bool equal(IN const float v1, IN const float v2,
	IN const float p = std::numeric_limits<float>::epsilon())
{
	return std::fabs(v1 - v2) < p;
}
inline bool is_zero(IN const float v)
{
	return std::fabs(v) < std::numeric_limits<float>::epsilon();
}
inline double round(double v)
{
	return (v > 0.0) ? floor(v + 0.5) : ceil(v - 0.5);
}


////////////////////////////////////////////////////////////////////////////////
template<uint32_t size>
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
	}

	template<typename T>
	inline Format& operator%(IN const T v)
	{
		if (m_cur_pos != -1)
		{
			const char* str = m_format + m_cur_pos;
			m_cur_pos = eco::find_first(str, '%');
			if (m_cur_pos == -1)
			{
				m_stream << str;
				return *this;
			}
			m_stream.buffer().append(str, m_cur_pos++);
			m_cur_pos += int(str - m_format);
			m_stream << v;
		}
		return *this;
	}

	inline operator const char* () const
	{
		return c_str();
	}

	inline const char* c_str() const
	{
		if (m_cur_pos != -1)
		{
			m_stream << (m_format + m_cur_pos);
			m_cur_pos = -1;
		}
		return m_stream.c_str();
	}

private:
	const char* m_format;
	mutable uint32_t m_cur_pos;
	mutable eco::StreamT<eco::FixBuffer<size> > m_stream;
};
typedef Format<fix_size> FixFormat;


////////////////////////////////////////////////////////////////////////////////
template<typename T>
class AutoArray
{
	ECO_NONCOPYABLE(AutoArray);
public:
	typedef std::shared_ptr<AutoArray> ptr;

	inline AutoArray()
	{}

	inline ~AutoArray()
	{
		for (size_t i = 0; i < m_data.size(); i++)
		{
			delete ((T*)m_data[i]);
		}
	}

	inline T* release(IN size_t i)
	{
		T* data = (T*)m_data[i];
		m_data[i] = nullptr;
		return data;
	}

	inline void release()
	{
		m_data.clear();
	}

	inline size_t size() const
	{
		return m_data.size();
	}

	inline T* operator [](const size_t i)
	{
		return (T*)m_data[i];
	}

	inline const T* operator [](const size_t i) const
	{
		return (T*)m_data[i];
	}

	inline std::vector<void*>& get()
	{
		return m_data;
	}

	inline const std::vector<void*>& get() const
	{
		return m_data;
	}

private:
	std::vector<void*> m_data;
};


////////////////////////////////////////////////////////////////////////////////
class StringAny
{
public:
	inline StringAny(IN const char*);
	inline StringAny& operator=(IN const char*);
	inline bool empty() const;
	inline const char* c_str() const;
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
ECO_API uint32_t __typeid(const char* type_info_name);
template<typename type_t>
class TypeId
{
public:
	inline operator const uint32_t() const
	{
		return value;
	}
	static const uint32_t value;
};
template<typename type_t>
const uint32_t TypeId<type_t>::value = __typeid(typeid(type_t).name());

////////////////////////////////////////////////////////////////////////////////
#define ECO_TYPE_1(type_t) ECO_TYPE__(type_t)\
public:\
	inline static const eco::Type* type()\
	{\
		static eco::Type t(type_id(), #type_t, nullptr,\
			&eco::Singleton<eco::TypeInit<type_t> >::instance());\
		return &t;\
	}\
	inline bool kind_of(const type_t& obj) const\
	{\
		return get_type()->kind_of(obj.get_type());\
	}\
	template<typename type_tt>\
	inline bool kind_of() const\
	{\
		return get_type()->kind_of(type_tt::type());\
	}
#define ECO_TYPE_2(type_t, parent) ECO_TYPE__(type_t)\
public:\
	inline static const eco::Type* type()\
	{\
		static eco::Type t(type_id(), #type_t, parent::type(),\
			&eco::Singleton<eco::TypeInit<type_t> >::instance());\
		return &t;\
	}
#define ECO_TYPE__(type_t)\
public:\
	virtual const eco::Type* get_type() const\
	{\
		return type();\
	}\
	inline static const uint32_t type_id()\
	{\
		return eco::TypeId<type_t>::value;\
	}\
	virtual const uint32_t get_type_id() const\
	{\
		return type_id();\
	}\
	inline static const char* type_name()\
	{\
		return #type_t;\
	}\
	virtual const char* get_type_name() const\
	{\
		return type_name();\
	}
#define ECO_TYPE(...) ECO_MACRO(ECO_TYPE_,__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
class Parameter
{
public:
	inline Parameter() {}

	/*@ name.*/
	inline void set_name(IN const char*);
	inline const char* get_name() const;
	inline Parameter& name(IN const char*);

	/*@ value.*/
	inline void set_value(IN const StringAny& val);
	inline StringAny& value();
	inline void set_value(IN const char* val);
	inline const StringAny& get_value() const;
	inline Parameter& value(IN const StringAny& val);
	inline Parameter& value(IN const char* val);

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
	std::string m_name;
	StringAny m_value;
	inline Parameter& impl() { return *this; }
	inline const Parameter& impl() const { return *this; }
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Context
{
	ECO_VALUE_API(Context);
public:
	/*@ parameter iterator.*/
	typedef eco::iterator<Parameter> iterator;
	typedef eco::iterator<const Parameter> const_iterator;
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	/*@ add parameter.*/
	Parameter& add();
	void add(IN const Parameter&);
	void push_back(IN const Parameter&);

	/*@ move & copy.*/
	void add_move(IN Context&);
	void add_copy(IN const Context&);

	/*@ remove parameter.*/
	void erase(IN int);
	iterator erase(IN iterator& it);
	void pop_back();

	/*@ clear parameter.*/
	void clear();

	/*@ get context parameter set size.*/
	size_t size() const;
	bool empty() const;
	void reserve(IN const size_t capacity);

	/*@ access parameter by item index.*/
	Parameter& at(IN const int i);
	const Parameter& at(IN const int i) const;

public:
	// whether has a key.
	bool has(IN const char* key) const;

	// get the key value.
	const StringAny& at(IN const char* key) const;

	// get the key value.
	const StringAny* find(IN const char* key) const;

	// get the key value.
	const StringAny get(IN const char* key) const;
};


////////////////////////////////////////////////////////////////////////////////
class ContextNodeSet;
class ECO_API ContextNode
{
	ECO_SHARED_API(ContextNode);
public:
	void set_name(IN const char*);
	const char* get_name() const;
	ContextNode& name(IN const char*);

	void set_value(IN const char*);
	const char* get_value() const;
	ContextNode& value(IN const char*);

	void set_property_set(IN const eco::Context&);
	eco::Context& property_set();
	const eco::Context& get_property_set() const;
	ContextNode& property_set(IN const eco::Context&);

	bool has_children() const;
	void set_children(IN eco::ContextNodeSet&);
	eco::ContextNodeSet& children();
	const eco::ContextNodeSet& get_children() const;

	// get children
	eco::ContextNode get_children(const char* key) const;

	// get children
	bool get_property_set(eco::Context& context, const char* key) const;

	// get key value.
	const StringAny& at(IN const char* key) const;
	const StringAny* find(IN const char* key) const;
	const StringAny get(IN const char* key) const;
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API ContextNodeSet
{
	ECO_SHARED_API(ContextNodeSet);
public:
	/*@ parameter iterator.*/
	typedef eco::iterator<ContextNode> iterator;
	typedef eco::iterator<const ContextNode> const_iterator;
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	/*@ move & copy.*/
	void add_move(IN ContextNodeSet&);
	void add_copy(IN const ContextNodeSet&);

	/*@ add parameter.*/
	ContextNode& add();
	void add(IN const ContextNode&);
	void push_back(IN const ContextNode&);

	/*@ remove parameter.*/
	void erase(IN int);
	iterator erase(IN iterator& it);
	void pop_back();

	/*@ clear parameter.*/
	void clear();

	/*@ get context parameter set size.*/
	size_t size() const;
	bool empty() const;
	void reserve(IN const size_t capacity);

	/*@ access parameter by item index.*/
	ContextNode& at(IN const int i);
	const ContextNode& at(IN const int i) const;

	// get property set.
	bool get_property_set(
		OUT eco::Context& result,
		IN const char* parent_key) const;

	// get context node.
	ContextNodeSet get_children(
		IN const char* parent_key) const;
};


////////////////////////////////////////////////////////////////////////////////
class Context::Impl
{
	ECO_IMPL_INIT(Context);
public:
	std::vector<Parameter> m_items;
};
class ContextNode::Impl
{
	ECO_IMPL_INIT(ContextNode);
public:
	std::string  m_name;
	std::string  m_value;
	eco::Context m_property_set;
	eco::ContextNodeSet m_children;
	Impl() : m_children(eco::null) {}
};
class ContextNodeSet::Impl
{
	ECO_IMPL_INIT(ContextNodeSet);
public:
	std::vector<ContextNode> m_items;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#include <eco/Type.inl>
#endif