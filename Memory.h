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
#include <memory>
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
#ifdef ECO_WIN32
#	define snprintf _snprintf
#endif


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
/*@ auto_strncpy copy like strncpy but return the length of copyed string instead
of return the dest string. 
1.auto_strncpy is more effective than strncpy, because strncpy will set left 
memory to '\0' when dest len is max than src.
2.auto_strncpy will auto add '\0' to dest when src len is equal or more than dest.
but strncpy not, and it will make crash sometimes.
*/
inline size_t auto_strncpy(OUT char* dest, IN const char* src, IN size_t len)
{
	assert(dest != nullptr && src != nullptr);
	// copy string like strncpy.
	size_t cpy_len = 0;
	char* temp = dest;
	while (cpy_len++ < len && (*temp++ = *src++) != '\0')
	{}

	// auto add '\0' to dest.
	if (--cpy_len == len)
	{
		dest[--cpy_len] = '\0';
	}
	return cpy_len;
}

inline size_t auto_strncat(OUT char* dest, IN const char* src, IN size_t len)
{
	size_t dest_end = strlen(dest);
	return auto_strncpy(&dest[dest_end], src, len - dest_end);
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
		if (find_cmp(dest, v))
			return dest;
	}
	return nullptr;
}

inline const char* find(
	IN const char* dest,
	IN const size_t size,
	IN const char* v)
{
	const char* end = &dest[size - strlen(v)];
	for (; dest <= end; ++dest)
	{
		if (find_cmp(dest, v))
			return dest;
	}
	return nullptr;
}



////////////////////////////////////////////////////////////////////////////////
inline uint32_t find_first(IN const char* key, IN const char flag)
{
	uint32_t pos = 0;
	for (; *key != 0 && *key != flag; ++key, ++pos) {}
	return (*key == 0) ? - 1 : pos;
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
	for (char* ch = path; *ch != 0; ++ch){
		if (*ch == c) *ch = r;
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void cpy_pos(OUT char& dest, OUT uint32_t& pos, IN  const char sour)
{
	dest = sour;
	pos += 1;
}
inline void cpy_pos(OUT char* dest, OUT uint32_t& pos,
	IN  const char* sour, IN uint32_t size)
{
	memcpy(dest, sour, size);
	pos += size;
}
#define eco_cpy_pos_dest(dest, pos, sour) \
{\
	memcpy(dest, sour, sizeof(dest));\
	pos += sizeof(dest);\
}
#define eco_cpy_pos_sour(dest, pos, sour) \
{\
	memcpy(dest, sour, sizeof(sour));\
	pos += sizeof(sour);\
}


////////////////////////////////////////////////////////////////////////////////
#define eco_cpyc(dest, sour) eco::auto_strncpy(dest, sour, sizeof(dest))
#define eco_cpys(dest, sour) eco::auto_strncpy(dest, sour.c_str(), sizeof(dest));
#define eco_catc(dest, sour) eco::auto_strncat(dest, sour, sizeof(dest))
#define eco_cats(dest, sour) eco::auto_strncat(dest, sour.c_str(), sizeof(dest));


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
	ECO_PTR_MEMBER(AutoDelete, m_ptr);
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
template<typename container>
inline void release(IN container& c)
{
	std::for_each(c.begin(), c.end(), [&](IN JoinMeta* pitem) {
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
}
#endif
