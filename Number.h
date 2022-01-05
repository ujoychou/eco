#ifndef ECO_NUMBER_H
#define ECO_NUMBER_H
/*******************************************************************************
@ name

@ function

@ exception


@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Prec.h>
#include <cstdint>
#include <cmath>
#include <limits>
#include <string>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
// value type: none int.
// note: uint32_t instead of enum for plant.
const uint32_t value_none = 0;


////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
inline void add(type_t& obj, uint32_t v)
{
	obj |= v;
}
template<typename type_t>
inline void del(type_t& obj, uint32_t v)
{
	obj &= ~v;
}
template<typename type_t>
inline void toggle(type_t& obj, uint32_t v)
{
	obj ^= v;
}
template<typename type_t>
inline void set(type_t& obj, uint32_t v, bool is)
{
	obj = is ? (obj | v) : (obj & ~v);
}
template<typename type_t>
inline void set_v(type_t& obj, uint32_t add_v, uint32_t del_v)
{
	eco::add(obj, add_v);
	eco::del(obj, del_v);
}
template<typename type_t>
inline bool has(const type_t& obj, uint32_t v)
{
	return (obj & v) > 0;
}


////////////////////////////////////////////////////////////////////////////////
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

inline bool equal(IN double v1, IN double v2,
	IN double p = std::numeric_limits<double>::epsilon())
{
	return std::fabs(v1 - v2) < p;
}
inline bool is_zero(IN double v)
{
	return std::fabs(v) < std::numeric_limits<double>::epsilon();
}
inline bool is_number(IN char v)
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
inline bool is_normal(IN double v)
{
	return v > -std::numeric_limits<double>::max()
		&& v < std::numeric_limits<double>::max();
}
inline bool is_positive(IN double v)
{
	return v > 0.0 && v < std::numeric_limits<double>::max();
}
inline bool is_negative(IN double v)
{
	return v < 0.0 && v > -std::numeric_limits<double>::max();
}
inline bool is_nan(IN double v)
{
	return int(std::isnan(v)) > 0;
}
inline bool is_infinity(IN double v)
{
	return v == std::numeric_limits<double>::infinity();
}
template<typename Number>
inline void set_double(OUT Number& d, IN double v, IN Number def = -1)
{
	d = (eco::is_nan(v) || eco::is_infinity(v)) ? def : v;
}
inline double get_double(IN double v, IN double def = -1)
{
	return (eco::is_nan(v) || eco::is_infinity(v)) ? def : v;
}
inline char yn(IN bool v)
{
	return v ? 'y' : 'n';
}

// float number.
inline bool equal(IN float v1, IN float v2,
	IN float p = std::numeric_limits<float>::epsilon())
{
	return std::fabs(v1 - v2) < p;
}
inline bool is_zero(IN float v)
{
	return std::fabs(v) < std::numeric_limits<float>::epsilon();
}
inline double round(double v)
{
	return (v > 0.0) ? floor(v + 0.5) : ceil(v - 0.5);
}


////////////////////////////////////////////////////////////////////////////////
// memory copy string to int
template<typename int_type>
inline int_type intcpy(IN const char* v)
{
	int_type int_v = 0;
	memcpy(&int_v, v, sizeof(int_type));
	return int_v;
}
template<typename int_type>
inline int_type intcpy(IN const std::string& v)
{
	int_type int_v = 0;
	memcpy(&int_v, v.c_str(), sizeof(int_type));
	return int_v;
}
template<typename int_type>
inline void intcpy(OUT char* v, IN int_type int_v)
{
	memcpy(&v[0], &int_v, sizeof(int_type));
}
template<typename int_type>
inline std::string intcpy(IN int_type int_v)
{
	char buf[20] = { 0 };
	memcpy(buf, &int_v, sizeof(int_type));
	return buf;
}


////////////////////////////////////////////////////////////////////////////////
// hash value function come from boost.
template<typename int_type>
inline void hash_combine(int_type& seed, int_type val)
{
	seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
template<typename int_type>
inline int_type hash_combine(int_type val)
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