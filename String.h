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
#include <eco/Object.h>
#include <eco/Stream.h>
#include <eco/Number.h>
#include <eco/rx/RxHeap.h>
#include <cstring>
#include <cassert>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
#ifdef ECO_VC100
#	define __func__ eco::func(__FUNCTION__)
#endif


////////////////////////////////////////////////////////////////////////////////
// value type: empty string.
const std::string value_empty;
using detail::is_lower;
using detail::upper;
using detail::iequal;
using detail::first;
inline bool is_upper(IN char v)
{
	return v >= 'A' && v <= 'Z';
}
inline bool is_char(IN char v)
{
	return is_upper(v) || is_lower(v);
}
inline bool is_char_number(IN char v)
{
	return is_number(v) || is_char(v);
}
inline bool empty(IN const char* v)
{
	return v[0] == 0;
}
inline bool equal(IN const char* s1, IN const char* s2)
{
	return strcmp(s1, s2) == 0;
}
inline char lower(IN char v)
{
	return is_upper(v) ? (v + 'a' - 'A') : v;
}

/*@ strncpy copy like strncpy but return the length of copyed string instead
of return the dest string. 
1.strncpy is more effective than strncpy, because strncpy will set left 
memory to '\0' when dest len is max than src.
2.strncpy will auto add '\0' to dest when src len is equal or more than dest.
but strncpy not, and it will make crash sometimes.
*/
inline size_t strncpy(OUT char* dest, IN const char* src, IN size_t len)
{
	assert(dest != nullptr && src != nullptr && len != 0);
	// copy string like strncpy.
	size_t cpy = 0;
	char* temp = dest;
	while (cpy++ < len && (*temp++ = *src++) != '\0') {}
	// auto add '\0' to dest.
	if (0 != len) dest[len - 1] = '\0';
	return len - 1;
}

inline size_t strncat(OUT char* dest, IN const char* src, IN size_t len)
{
	size_t dest_end = strlen(dest);
	return strncpy(&dest[dest_end], src, len - dest_end);
}


////////////////////////////////////////////////////////////////////////////////
inline void copy(OUT char& dest, OUT uint32_t& pos, IN  char sour)
{
	dest = sour;
	pos += 1;
}
inline void copy(char* dest, uint32_t& pos, const char* sour, uint32_t size)
{
	memcpy(dest, sour, size);
	pos += size;
}
#define eco_copy_dest(dest, pos, sour) \
{\
	memcpy(dest, sour, sizeof(dest));\
	pos += sizeof(dest);\
}
#define eco_copy_sour(dest, pos, sour) \
{\
	memcpy(dest, sour, sizeof(sour));\
	pos += sizeof(sour);\
}
#define eco_cpyc(dest, sour) eco::strncpy(dest, sour, sizeof(dest))
#define eco_cpys(dest, sour) eco::strncpy(dest, (sour).c_str(), sizeof(dest))
#define eco_catc(dest, sour) eco::strncat(dest, sour, sizeof(dest))
#define eco_cats(dest, sour) eco::strncat(dest, (sour).c_str(), sizeof(dest))


////////////////////////////////////////////////////////////////////////////////
inline uint32_t find_first(IN const char* key, IN char flag)
{
	uint32_t pos = 0;
	for (; *key != 0 && *key != flag; ++key, ++pos) {}
	return (*key == 0) ? -1 : pos;
}
inline uint32_t find_last(IN const char* key, IN uint32_t end, IN char flag)
{
	const char* it = key + end - 1;
	for (; *it != flag && it >= key; --it) {}
	return static_cast<uint32_t>(it - key);
}
inline uint32_t find_last(IN const char* key, IN char flag)
{
	// key format: "logging/file_link/roll_size".
	uint32_t len = static_cast<uint32_t>(strlen(key));
	return find_last(key, len, flag);
}
inline uint32_t find_nth(const char* key, char flag, uint32_t nth)
{
	uint32_t pos = 0;
	uint32_t cur_seq = 0;
	for (; *key != 0; ++key, ++pos)
	{
		if (*key == flag && ++cur_seq == nth) { break; }
	}
	return (*key == 0) ? -1 : pos;
}
inline bool find_cmp(IN const char* dest, IN const char* v)
{
	for (; *dest != '\0' && *v != '\0' && *dest == *v; ++dest, ++v) {}
	return *v == '\0';
}
inline const char* find(IN const char* dest, IN const char* v)
{
	for (; *dest != '\0'; ++dest)
	{
		if (find_cmp(dest, v)) { return dest; }
	}
	return nullptr;
}


////////////////////////////////////////////////////////////////////////////////
/*@find sub string from string.
@return: the index of string array.
*/
inline const char* find(
	IN const char* data, IN size_t size,
	IN const char* sub, IN size_t sub_size = 0)
{
	if (sub_size == 0) sub_size = strlen(sub);
	const char* end = &data[size - sub_size];
	for (; data <= end; ++data)
	{
		if (find_cmp(data, sub)) return data;
	}
	return nullptr;
}

/*@reverse find sub string from string.
@return: the index of string array.
exp:
String str("there is a desk.");
str.find_reverse("is", 10, 6) == 6;
str.find_reverse("desk") == 11;
*/
inline uint32_t find_reverse(
	IN const char* data, IN uint32_t size,
	IN const char* sub, IN uint32_t rstart = -1, IN uint32_t rend = -1)
{
	// size isn't enough.
	uint32_t size_sub = (uint32_t)strlen(sub);
	if (size_sub > size) return uint32_t(-1);

	uint32_t pos = size - size_sub;
	if (rstart == uint32_t(-1) || pos < rstart) rstart = pos;
	rend += 1;
	if (rend > rstart) return uint32_t(-1);

	// reverse find with "--".
	const char* end = &data[rend];
	const char* start = &data[rstart];
	// "--end" because of "rend += 1"
	for (--end; start != end; --start)
	{
		if (eco::find_cmp(start, sub))
			return uint32_t(start - data);
	}
	return -1;
}


////////////////////////////////////////////////////////////////////////////////
inline void insert(OUT char* dest, IN uint32_t pos, IN uint32_t num, char c)
{
	if (num == 0) { return; }

	size_t size = strlen(dest);
	char* dest_move = &dest[size];
	char* dest_move_end = &dest[pos];
	for (; dest_move >= dest_move_end; --dest_move)
	{
		*(dest_move + num) = *dest_move;
	}
	for (; num > 0; --num)
	{
		*dest_move_end++ = c;
	}
}
inline void replace(OUT char* path, IN char c, IN char r)
{
	for (char* ch = path; *ch != 0; ++ch)
	{
		if (*ch == c) { *ch = r; }
	}
}
inline void remove(IN std::string& v, IN char ch)
{
	v.erase(std::remove(v.begin(), v.end(), ch), v.end());
}
inline void remove(IN char* v, IN char ch)
{
	char* p = v;
	for (; *v != 0; ++v)
	{
		if (*v == ch) { continue; }
		*p = *v; ++p;
	}
	*p = 0;
}
// clear the string data.
inline void clear(OUT char* v)
{
	v[0] = 0;
}
// get string end size.
inline size_t fit_size(IN const char* str, IN size_t size)
{
	size_t x = 0;
	size_t i = size - 1;
	for (; i != size_t(-1) && str[i] == 0; --i) ++x;
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
template<typename array_t>
inline void split(OUT array_t& set, IN const char* str, IN char token)
{
	const char* v = str;
	uint32_t pos = eco::find_first(v, token);
	while (pos != -1)
	{
		set.push_back(std::string(v, pos));
		v += pos + 1;
		pos = eco::find_first(v, token);
	}
	if (!eco::empty(v))
	{
		set.push_back(v);
	}
}
template<typename type_t, typename array_t>
inline void split(OUT array_t& set, IN const char* str, IN char token)
{
	const char* v = str;
	uint32_t pos = eco::find_first(v, token);
	while (pos != -1)
	{
		set.push_back(eco::cast<type_t>(std::string(v, pos)));
		v += pos + 1;
		pos = eco::find_first(v, token);
	}
	if (!eco::empty(v))
	{
		set.push_back(eco::cast<type_t>(v));
	}
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
		if (m_size == uint32_t(-1) && data != nullptr)
			m_size = static_cast<uint32_t>(strlen(data));
	}

	inline const char* c_str() const
	{
		return m_data;
	}

	inline uint32_t size() const
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
	if (end == uint32_t(-1)) end = len;
	uint32_t start = eco::find_last(full_func_name, end, ':');
	start = (start == uint32_t(-1)) ? 0 : start + 1;
	return eco::Bytes(full_func_name + start, end - start);
}
inline const char* clss(IN const char* full_clss_name)
{
	uint32_t end = (uint32_t)strlen(full_clss_name);
	uint32_t start = eco::find_last(full_clss_name, end, ':');
	start = (start == uint32_t(-1)) ? 0 : start + 1;
	return (full_clss_name + start);
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

	explicit inline String(IN uint32_t size, IN bool reserved = false)
		: m_data(nullptr), m_size(0), m_capacity(0)
	{
		if (reserved)
			reserve(size);
		else
			resize(size);
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
			uint32_t size = m_size - pos;
			if (count < size) size = count;
			count = m_size - pos - size;
			if (count > 0) memcpy(&m_data[pos], &m_data[pos + size], count);
			m_size -= size;
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
			if (c < 32) { c = 32; }
			uint32_t old_size = m_size;
			uint32_t new_size = m_capacity * 2;
			if (new_size < c) { new_size = c; }

			// keep old value.
			char* new_data = (char*)RxHeap::
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
			RxHeap::operator delete[](m_data);
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
template<uint32_t size_v = 160>
class Buffer
{
	ECO_STREAM_OPERATOR(Buffer, (*this), ' ');
public:
	inline Buffer()
	{
		assert(size_v > 0);
		clear();
	}

	inline explicit Buffer(const char* msg)
	{
		assert(size_v > 0);
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

	inline char* buffer(IN uint32_t pos = 0)
	{
		return &m_data[pos];
	}

	inline uint32_t size() const
	{
		return m_size;
	}

	inline static uint32_t capacity()
	{
		return size_v - 1;	// last char = '\0'
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
	inline uint32_t append(IN uint32_t size, IN char c)
	{
		if (size == 1) return append(c);
		if (avail() >= size)
		{
			memset(&m_data[m_size], c, size);
			m_size += size;
			m_data[m_size] = 0;
			return size;
		}
		return 0;
	}

	// force append string.
	inline uint32_t force_append(IN const char* buf, IN uint32_t size)
	{
		force(size);
		return append(buf, size);
	}

	// append char.
	inline uint32_t force_append(IN uint32_t size, IN char c)
	{
		force(size);
		return append(size, c);
	}

	// erase a space for force content using.
	inline void force(IN uint32_t size)
	{
		int32_t pop_size = static_cast<int32_t>(size - avail());
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
	char m_data[size_v];
	uint32_t  m_size;
};


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
	inline Format& operator%(IN T v)
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
	inline bool operator!=(IN const string_t& v) const
	{
		return !(operator==(v));
	}

public:
	inline StringAny();
	inline StringAny(IN bool v);
	inline StringAny(IN char v);
	inline StringAny(IN unsigned char v);
	inline StringAny(IN int32_t v);
	inline StringAny(IN uint32_t v);
	inline StringAny(IN int64_t v);
	inline StringAny(IN uint64_t v);
	inline StringAny(IN double v, IN int precision = -1);

public:
	inline operator const char*() const;
	inline operator char() const;
	inline operator unsigned char() const;
	inline operator int16_t() const;
	inline operator uint16_t() const;
	inline operator int32_t() const;
	inline operator uint32_t() const;
	inline operator int64_t() const;
	inline operator uint64_t() const;
	inline operator float() const;
	inline operator double() const;
	inline operator bool() const;

private:
	eco::ValueType m_vtype;
	std::string m_value;
};


////////////////////////////////////////////////////////////////////////////////
StringAny::StringAny() : m_vtype(0) {}
StringAny::StringAny(IN StringAny&& v)
	: m_vtype(v.m_vtype), m_value(std::move(v.m_value)) { v.m_vtype = 0; }
StringAny::StringAny(IN const StringAny& v)
	: m_vtype(v.m_vtype), m_value(v.m_value)  {}
StringAny::StringAny(IN const char* v)
	: m_vtype(type_string), m_value(v) {}
StringAny::StringAny(IN bool v)
	: m_vtype(type_bool), m_value(eco::cast(v)) {}
StringAny::StringAny(IN char v)
	: m_vtype(type_char), m_value(1, v) {}
StringAny::StringAny(IN unsigned char v)
	: m_vtype(type_char), m_value(1, v) {}
StringAny::StringAny(IN int32_t v)
	: m_vtype(type_int32), m_value(eco::Integer<int32_t>(v).c_str()) {}
StringAny::StringAny(IN uint32_t v)
	: m_vtype(type_int32), m_value(eco::Integer<uint32_t>(v).c_str()) {}
StringAny::StringAny(IN int64_t v)
	: m_vtype(type_int64), m_value(eco::Integer<int64_t>(v).c_str()) {}
StringAny::StringAny(IN uint64_t v)
	: m_vtype(type_int64), m_value(eco::Integer<uint64_t>(v).c_str()) {}
StringAny::StringAny(IN double v, IN int precision)
	: m_vtype(type_double), m_value(eco::Double(v, precision).c_str()) {}

StringAny& StringAny::operator=(IN StringAny&& v)
{
	m_value = std::move(v.m_value);
	m_vtype = v.m_vtype;
	v.m_vtype = 0;
	return *this;
}
StringAny& StringAny::operator=(IN const StringAny& v)
{
	m_value = v.m_value;
	m_vtype = v.m_vtype;
	return *this;
}
StringAny& StringAny::operator=(IN const char* v)
{
	m_vtype = type_string;
	m_value = v;
	return *this;
}
bool StringAny::operator==(IN const char* v) const
{
	return m_value == v;
}
bool StringAny::operator==(IN const StringAny& obj) const
{
	return m_value == obj.m_value && m_vtype == obj.m_vtype;
}
bool StringAny::empty() const
{
	return m_value.empty();
}
const char* StringAny::c_str() const
{
	return m_value.c_str();
}
std::string& StringAny::str()
{
	return m_value;
}
StringAny::operator const char*() const
{
	return m_value.c_str();
}
StringAny::operator char() const
{
	return eco::cast<char>(m_value);
}
StringAny::operator unsigned char() const
{
	return eco::cast<unsigned char>(m_value);
}
StringAny::operator int16_t() const
{
	return eco::cast<int16_t>(m_value);
}
StringAny::operator uint16_t() const
{
	return eco::cast<uint16_t>(m_value);
}
StringAny::operator int32_t() const
{
	return eco::cast<int32_t>(m_value);
}
StringAny::operator uint32_t() const
{
	return eco::cast<uint32_t>(m_value);
}
StringAny::operator int64_t() const
{
	return eco::cast<int64_t>(m_value);
}
StringAny::operator uint64_t() const
{
	return eco::cast<uint64_t>(m_value);
}
StringAny::operator double() const
{
	return eco::cast<double>(m_value);
}
StringAny::operator float() const
{
	return eco::cast<float>(m_value);
}
StringAny::operator bool() const
{
	return eco::cast<bool>(m_value);
}
eco::ValueType StringAny::type() const
{
	return m_vtype;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif