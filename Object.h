#ifndef ECO_OBJECT_H
#define ECO_OBJECT_H
/*******************************************************************************
@ name
basic language define.

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.

@ ujoy modifyed on 2021-11-20
1.refactory responsibility of base frame.
2.change Export.h -> Prec.h


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Prec.h>
#include <memory>
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
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
	type_char			= 11,			// 'a'
	type_defined		= 100,			// user defined type.
};
typedef uint16_t ValueType;


////////////////////////////////////////////////////////////////////////////////
template<typename value_t>
class Value
{
public:
	typedef value_t object;
	typedef value_t value;
};


////////////////////////////////////////////////////////////////////////////////
template<typename object_t>
class Object
{
public:
	typedef object_t object;
	typedef std::shared_ptr<object_t> value;
	typedef std::shared_ptr<object_t> ptr;
	typedef std::weak_ptr<object_t> wptr;

protected:
	Object() {};
	~Object() {};

private:
	Object(const Object&);
	const Object& operator=(const Object& );
};


////////////////////////////////////////////////////////////////////////////////
template<typename Object>
class Movable
{
public:
	inline Movable() {}
	inline Movable(IN Object& v) : m_object(std::move(v)) {}
	inline Movable(IN Movable&& v) : m_object(std::move(v.m_object)) {}
	inline Movable(IN const Movable& v) : m_object(std::move(v.m_object)) {}
	inline operator Object& () { return m_object; }
	inline operator const Object& () const { return m_object; }
	inline Movable& operator=(IN const Movable& v)
	{
		m_object = std::move(v.m_object);
		return *this;
	}

private:
	mutable Object m_object;
};


////////////////////////////////////////////////////////////////////////////////
template<typename Object>
inline eco::Movable<Object> move(IN Object& obj)
{
	return eco::Movable<Object>(obj);
}
template<typename type_t>
inline type_t& lv(type_t&& value)
{
	return value;
}

// compile object: noncopyable.
#define ECO_NONCOPYABLE(type_t) \
private:\
	type_t(IN const type_t& );\
	type_t& operator=(IN const type_t& );

// compile object: single object delare.
#define ECO_OBJECT_PTR(object_t) \
public:\
	typedef object_t object;\
	typedef std::shared_ptr<object_t> value;\
	typedef std::shared_ptr<object_t> ptr;\
	typedef std::weak_ptr<object_t> wptr;
#define ECO_OBJECT(object_t) \
ECO_OBJECT_PTR(object_t) ECO_NONCOPYABLE(object_t);
#define ECO_NEW(object_t) std::make_shared<object_t>()


////////////////////////////////////////////////////////////////////////////////
// singleton proxy object that instantiate the object.
template<typename object_t>
class Singleton
{
	ECO_OBJECT(Singleton);
public:
	inline static object_t& get()
	{
		return s_object;
	}
private:
	static object_t s_object;
};
template<typename object_t>
object_t Singleton<object_t>::s_object;

// singleton: single object declare.
#define ECO_SINGLETON_1(object_t)\
	ECO_NONCOPYABLE(object_t);\
public:\
	inline static object_t& get() {\
		return eco::Singleton<object_t>::get();\
	}\
private:\
	friend class eco::Singleton<object_t>;\
	inline object_t() {}

#define ECO_SINGLETON_2(object_t, init)\
	ECO_NONCOPYABLE(object_t);\
public:\
	inline static object_t& get() {\
		return eco::Singleton<object_t>::get();\
	}\
private:\
	friend class eco::Singleton<object_t>;\
	inline object_t() { init(); }
#define ECO_SINGLETON(...) ECO_MACRO(ECO_SINGLETON_,__VA_ARGS__)

// singleton: single func.
#define ECO_SINGLETON_NAME(object_t, method)\
inline object_t& method()\
{\
	return eco::Singleton<object_t>::get();\
}

// singleton: single lazy func.
#define ECO_SINGLETON_LAZY(object_t, method)\
inline object_t& method()\
{\
	static object_t s_object;\
	return s_object;\
}


////////////////////////////////////////////////////////////////////////////////
// get object type.
template<typename T>
inline T& get_object() { return *(T*)nullptr; }
template<typename T>
inline T& get_object(T& obj) { return obj; }
template<typename T>
inline T& get_object(std::shared_ptr<T>& ptr) { return *ptr; }
template<typename T>
inline const T& object(const T& obj) { return obj; }
template<typename T>
inline const T& object(const std::shared_ptr<T>& ptr) { return *ptr; }

// get T when type is shared_ptr<T>, else return T when type is T.
template<typename T>
class Raw
{
public:
	typedef decltype(&eco::get_object(eco::get_object<T>())) ptr;
	typedef decltype(&eco::get_object(eco::get_object<T>())) pointer;
	typedef decltype(eco::get_object(eco::get_object<T>()))  reference;
	typedef decltype(&eco::object(eco::get_object<T>())) const_ptr;
	typedef decltype(&eco::object(eco::get_object<T>())) const_pointer;
	typedef decltype(eco::object(eco::get_object<T>()))  const_reference;
};


////////////////////////////////////////////////////////////////////////////////
// make object.
template<typename T>
inline void* make(std::unique_ptr<T>& obj)
{
	obj.reset(new T());
	return obj.get();
}
template<typename T>
inline void* make(std::shared_ptr<T>& obj)
{
	obj = std::make_shared<T>();
	return obj.get();
}
template<typename T>
inline void* make(T& obj)
{
	return &obj;
}
template<typename T>
inline void* make(T* obj)
{
	return new T();
}

////////////////////////////////////////////////////////////////////////////////
// task: closure method.
typedef std::function<void(void)> Task;

// task handler.
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
template<typename set_t>
struct SetValueCompare
{
	typedef typename set_t::value_type value_t;
	typedef std::function<bool(const value_t& a, const value_t& b)> Func;
};
template<typename set_t>
inline void get_insert_set(
	OUT set_t& insert_set,
	IN  const set_t& old_set,
	IN  const set_t& new_set,
	IN  typename SetValueCompare<set_t>::Func&& equal)
{
	for (auto& new_it : new_set)
	{
		auto it_find = std::find_if(old_set.begin(), old_set.end(),
			[&](const typename set_t::value_type& old)->bool {
			return equal(old, new_it);
		});
		if (it_find == old_set.end())
		{
			insert_set.push_back(new_it);
		}
	}
}
template<typename set_t>
inline void get_compare_set(
	OUT set_t& insert_set,
	OUT set_t& delele_set,
	IN  const set_t& old_set,
	IN  const set_t& new_set,
	IN  typename SetValueCompare<set_t>::Func equal)
{
	get_insert_set(insert_set, old_set, new_set, equal);
	get_insert_set(delele_set, new_set, old_set, equal);
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
		for (uint8_t i = 0; i < m_turn_line; ++i)
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

// when app exit with exception, use _getch to show a console windows.
// and show the exception error.
inline char getch_exit()
{
	char ch = 0;
	std::cout << "please input a char to end." << std::endl;
	std::cin >> ch;
	return ch;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif