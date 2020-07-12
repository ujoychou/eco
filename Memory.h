#ifndef ECO_MEMORY_H
#define ECO_MEMORY_H
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-12-03.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Export.h>
#include <algorithm>
#include <stdarg.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
inline bool is_upper(IN const char v)
{
	return v >= 'A' && v <= 'Z';
}
inline bool is_lower(IN const char v)
{
	return v >= 'a' && v <= 'z';
}
inline bool is_char(IN const char v)
{
	return is_upper(v) || is_lower(v);
}
inline char upper(IN const char v)
{
	return is_lower(v) ? v + 'A' - 'a' : v;
}
inline char lower(IN const char v)
{
	return is_upper(v) ? v + 'a' - 'A' : v;
}


////////////////////////////////////////////////////////////////////////////////
#ifdef ECO_WIN32
inline int snprintf(OUT char* buf, IN size_t size, IN const char* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	int result = _vsnprintf(buf, size, format, arglist);
	va_end(arglist);
	return result;
}
#endif


////////////////////////////////////////////////////////////////////////////////
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
	size_t cpy_len = 0;
	char* temp = dest;
	while (cpy_len++ < len && (*temp++ = *src++) != '\0') {}
	// auto add '\0' to dest.
	if (--cpy_len == len) dest[len] = '\0';
	return cpy_len;
}

inline size_t strncat(OUT char* dest, IN const char* src, IN size_t len)
{
	size_t dest_end = strlen(dest);
	return strncpy(&dest[dest_end], src, len - dest_end);
}


////////////////////////////////////////////////////////////////////////////////
inline void copy(OUT char& dest, OUT uint32_t& pos, IN  const char sour)
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


////////////////////////////////////////////////////////////////////////////////
#define eco_cpyc(dest, sour) eco::strncpy(dest, sour, sizeof(dest) - 1)
#define eco_cpys(dest, sour) eco::strncpy(dest, (sour).c_str(), sizeof(dest) - 1)
#define eco_catc(dest, sour) eco::strncat(dest, sour, sizeof(dest) - 1)
#define eco_cats(dest, sour) eco::strncat(dest, (sour).c_str(), sizeof(dest) - 1)


////////////////////////////////////////////////////////////////////////////////
inline bool find_cmp(IN const char* dest, IN const char* v)
{
	for (; *dest != '\0' && *v != '\0' && *dest == *v; ++dest, ++v) {}
	return *v == '\0';
}

inline const char* find(IN const char* dest, IN const char* v)
{
	for (; *dest != '\0'; ++dest)
	{
		if (find_cmp(dest, v))
			return dest;
	}
	return nullptr;
}

/*@find sub string from string.
@return: the index of string array.
*/
inline const char* find(
	IN const char* data, 
	IN const size_t size,
	IN const char* sub,
	IN size_t sub_size = 0)
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
	IN const char* data,
	IN uint32_t size,
	IN const char* sub,
	IN uint32_t rstart = -1,
	IN uint32_t rend = -1)
{
	// size isn't enough.
	uint32_t size_sub = (uint32_t)strlen(sub);
	if (size_sub > size) return -1;

	uint32_t pos = size - size_sub;
	if (rstart == -1 || pos < rstart) rstart = pos;
	rend += 1;
	if (rend > rstart) return -1;

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
inline uint32_t find_first(IN const char* key, IN const char flag)
{
	uint32_t pos = 0;
	for (; *key != 0 && *key != flag; ++key, ++pos) {}
	return (*key == 0) ? -1 : pos;
}
inline uint32_t find_last(IN const char* key, IN const uint32_t end,
	IN const char flag)
{
	const char* it = key + end - 1;
	for (; *it != flag && it >= key; --it) {}
	return static_cast<uint32_t>(it - key);
}
inline uint32_t find_last(IN const char* key, IN const char flag)
{
	// key format: "logging/file_link/roll_size".
	uint32_t len = static_cast<uint32_t>(strlen(key));
	return find_last(key, len, flag);
}
inline uint32_t find_nth(const char* key, const char flag, const uint32_t nth)
{
	uint32_t pos = 0;
	uint32_t cur_seq = 0;
	for (; *key != 0; ++key, ++pos)
	{
		if (*key == flag && ++cur_seq == nth)
			break;
	}
	return (*key == 0) ? -1 : pos;
}


////////////////////////////////////////////////////////////////////////////////
inline void insert(OUT char* dest, IN uint32_t pos, IN uint32_t num, char c)
{
	if (num == 0)
	{
		return;
	}

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


////////////////////////////////////////////////////////////////////////////////
inline void replace(OUT char* path, IN const char c, IN const char r)
{
	for (char* ch = path; *ch != 0; ++ch)
	{
		if (*ch == c) *ch = r;
	}
}
inline bool equal(IN const char* s1, IN const char* s2)
{
	return std::strcmp(s1, s2) == 0;
}
inline bool iequal(IN const char* s1, IN const char* s2)
{
	char c1 = 0, c2 = 0;
	for (size_t i = 0; true; ++i)
	{
		c1 = upper(s1[i]);
		c2 = upper(s2[i]);
		if (c1 != c2 || c1 == 0 || c2 == 0) break;
	}
	return c1 == 0 && c2 == 0;
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
		if (*v == ch) continue;
		*p = *v; ++p;
	}
	*p = 0;
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


////////////////////////////////////////////////////////////////////////////////
// memory copy string to int
template<typename int_type>
inline int_type intcpy(IN const char* v)
{
	int_type int_v = 0;
	eco::strncpy((char*)&int_v, v, sizeof(int_type));
	return int_v;
}
template<typename int_type>
inline int_type intcpy(IN const std::string& v)
{
	int_type int_v = 0;
	eco::strncpy((char*)&int_v, v.c_str(), sizeof(int_type));
	return int_v;
}
template<typename int_type>
inline void intcpy(OUT char* v, IN int_type int_v)
{
	eco::strncpy(&v[0], (const char*)&int_v, sizeof(int_type));
}
template<typename int_type>
inline std::string intcpy(IN int_type int_v)
{
	char buf[20] = { 0 };
	eco::strncpy(buf, (const char*)&int_v, sizeof(int_type));
	return buf;
}


////////////////////////////////////////////////////////////////////////////////
#define ECO_PTR_MEMBER(data_t, m_data) \
public:\
	inline data_t* operator->()\
	{\
		return m_data;\
	}\
	inline const data_t* operator->() const\
	{\
		return m_data;\
	}\
	inline data_t& operator*()\
	{\
		return *m_data;\
	}\
	inline const data_t& operator*() const\
	{\
		return *m_data;\
	}\
	inline data_t* get()\
	{\
		return m_data;\
	}\
	inline const data_t* get() const\
	{\
		return m_data;\
	}\
	inline bool null() const\
	{\
		return m_data == nullptr;\
	}

////////////////////////////////////////////////////////////////////////////////
/* auto delete heap ptr and manage object memory.*/
template<typename Object>
class AutoDelete
{
	ECO_PTR_MEMBER(Object, m_ptr);
public:
	inline AutoDelete(
		IN Object* ptr = nullptr,
		IN bool auto_delete = true)
		: m_ptr(ptr), m_auto_delete(auto_delete)
	{}

	inline ~AutoDelete()
	{
		if (m_auto_delete)
		{
			delete m_ptr;
		}
	}

	inline void reset(Object* ptr = nullptr, bool auto_delete = true)
	{
		if (m_ptr == ptr)
		{
			return;
		}

		if (m_auto_delete)
		{
			delete m_ptr;
		}

		m_ptr = ptr;
		m_auto_delete = auto_delete;
	}

private:
	Object* m_ptr;
	bool m_auto_delete;
};


////////////////////////////////////////////////////////////////////////////////
template<typename object_t, typename container>
inline void release(IN container& c)
{
	std::for_each(c.begin(), c.end(), [&](IN object_t* pitem) {
		delete pitem;
	});
	c.clear();
}


////////////////////////////////////////////////////////////////////////////////
// hash value function come from boost.
template<typename int_type>
inline void hash_combine(int_type& seed, const int_type val)
{
	seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
template<typename int_type>
inline int_type hash_combine(const int_type val)
{
	int_type seed = 0;
	seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	return seed;
}
template<typename T>
inline void hash_address(uint64_t& seed, const T& val)
{
	hash_combine(seed, reinterpret_cast<uint64_t>(&val));
}
template<typename T>
inline void hash_value(uint64_t& seed, const T& val)
{
	hash_address(seed, val);
}

// must support c++11 variadic template.
#ifndef ECO_NO_VARIADIC_TEMPLATE
template<typename T, typename... Types>
inline void hash_value(uint64_t& seed, const T& val, const Types&... args)
{
	hash_address(seed, val);
	hash_value(seed, args...);
}
template<typename... Types>
inline uint64_t hash_value(const Types&... args)
{
	uint64_t seed = 0;
	hash_value(seed, args...);
	return seed;
}
#endif
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif
