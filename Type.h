#ifndef ECO_TYPE_H
#define ECO_TYPE_H
/*******************************************************************************
@ name
type define.

@ function
1 value define: empty value.
2 value type: plat value.
3 value operation: +-/*, &|.

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
#include <string>
#include <unordered_map>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
// value type: none int.
// note: uint32_t instead of enum for plant.
const uint32_t value_none = 0;

// value type: empty string.
const std::string value_empty;

////////////////////////////////////////////////////////////////////////////////
// general value type
enum
{
	type_bool			= 1,			// "true/false"
	type_int32			= 2,			// "1"
	type_int64			= 3,			// "1"
	type_string			= 4,			// "abcdefg,1234"
	type_double			= 5,			// "2.35"
	type_date			= 6,			// "20051010"
	type_time			= 7,			// "10:10:10"
	type_date_time		= 8,			// "20051010 10:10:10"
	type_enum			= 9,			// "1:x;2:y;3:z"
	type_percent		= 10,			// "10%"
	type_defined		= 100,			// user defined type.
};
typedef uint16_t ValueType;




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
inline void set_v(type_t& obj, const uint32_t add_v, const uint32_t del_v)
{
	eco::add(add_v);
	eco::del(del_v);
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

inline bool equal(IN const double v1, IN const double v2,
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
// closure method.
typedef std::function<void(void)> Closure;

// closure task handler.
class Handler
{
public:
	template<typename task_t>
	inline void operator()(task_t& task) { task(); }

	template<typename task_t>
	inline void operator()(std::shared_ptr<task_t>& task) { (*task)(); }
};


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
class Parameter
{
public:
	inline Parameter() {}

	/*@ name.*/
	inline void set_name(IN const char*);
	inline const char* name() const;
	inline Parameter& name(IN const char*);

	/*@ value.*/
	inline void set_value(IN const StringAny& val);
	inline StringAny& get_value();
	inline void set_value(IN const char* val);
	inline const StringAny& value() const;
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
	const char* name() const;
	ContextNode& name(IN const char*);

	void set_value(IN const char*);
	const char* value() const;
	ContextNode& value(IN const char*);

	void set_property_set(IN const eco::Context&);
	eco::Context& get_property_set();
	const eco::Context& property_set() const;
	ContextNode& property_set(IN const eco::Context&);

	// get children
	bool has_children() const;
	void set_children(IN const eco::ContextNodeSet&);
	eco::ContextNodeSet& get_children();
	const eco::ContextNodeSet& children() const;
	eco::ContextNodeSet get_children(const char* child_key) const;

	// get child node.
	eco::ContextNode get_child(const char* child_key) const;

	// get children
	bool get_property_set(eco::Context& context, const char* key) const;

	// get key value.
	const StringAny& at(IN const char* key) const;
	const StringAny* find(IN const char* key) const;
	const StringAny  get(IN const char* key) const;

	// merge context node.
	void merge(eco::ContextNode& node);
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

	/*@ access parameter by item name.*/
	ContextNode* find(IN const char* name);
	const ContextNode* find(IN const char* name) const;
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

public:
	inline Impl() : m_children(eco::null) {}
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