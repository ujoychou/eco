#ifndef ECO_OBJECT_H
#define ECO_OBJECT_H
/*******************************************************************************
@ name
object.

@ function
1.define the base object type.
2.define the base value type.



--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Export.h>


ECO_NS_BEGIN(eco)
////////////////////////////////////////////////////////////////////////////////
// make object.
template<typename T>
inline static void make(T&) {}
template<typename T>
inline static void make(std::shared_ptr<T>& ptr) { ptr.reset(new T()); }

// get object type.
template<typename T>
inline T& object() { return *(T*)nullptr; }
template<typename T>
inline T& object(T& obj) { return obj; }
template<typename T>
inline T& object(std::shared_ptr<T>& ptr) { return *ptr; }
template<typename T>
inline const T& get_object(const T& obj) { return obj; }
template<typename T>
inline const T& get_object(const std::shared_ptr<T>& ptr) { return *ptr; }

// get T when type is shared_ptr<T>, else return T when type is T.
template<typename T>
class Raw
{
public:
	typedef decltype(&eco::object(eco::object<T>())) ptr;
	typedef decltype(&eco::object(eco::object<T>())) pointer;
	typedef decltype(eco::object(eco::object<T>()))  reference;
};


////////////////////////////////////////////////////////////////////////////////
#define ECO_OBJECT_PTR(object_t) \
public:\
	typedef object_t object;\
	typedef std::shared_ptr<object_t> value;\
	typedef std::shared_ptr<object_t> ptr;\
	typedef std::weak_ptr<object_t> wptr;
#define ECO_OBJECT(object_t) \
ECO_OBJECT_PTR(object_t) ECO_NONCOPYABLE(object_t);
#define ECO_NEW(object_t) object_t::ptr(new object_t)

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
template<typename value_t>
class Value
{
public:
	typedef value_t object;
	typedef value_t value;
};


////////////////////////////////////////////////////////////////////////////////
// singleton proxy object that instantiate the object.
template<typename object_t>
class Singleton
{
	ECO_OBJECT(Singleton);
public:
	inline static object_t& instance()
	{
		return s_object;
	}
private:
	static object_t s_object;
};
template<typename object_t>
object_t Singleton<object_t>::s_object;


/*@ singleton instance to access singleton object. */
#define ECO_SINGLETON_1(object_t)\
	ECO_NONCOPYABLE(object_t);\
public:\
	inline static object_t& instance() {\
		return eco::Singleton<object_t>::instance();\
	}\
private:\
	friend class eco::Singleton<object_t>;\
	inline object_t() {}

#define ECO_SINGLETON_2(object_t, init)\
	ECO_NONCOPYABLE(object_t);\
public:\
	inline static object_t& instance() {\
		return eco::Singleton<object_t>::instance();\
	}\
private:\
	friend class eco::Singleton<object_t>;\
	inline object_t() { init(); }
#define ECO_SINGLETON(...) ECO_MACRO(ECO_SINGLETON_,__VA_ARGS__)


#define ECO_SINGLETON_API(object_t)\
    ECO_IMPL_API();\
    ECO_NONCOPYABLE(object_t);\
public:\
	inline static object_t& instance() {\
		return eco::Singleton<object_t>::instance();\
	}\
	~object_t();\
private:\
	friend class eco::Singleton<object_t>;\
	object_t();\

/*@ singleton get function to access singleton object.*/
#define ECO_SINGLETON_NAME(object_t, method)\
inline object_t& method()\
{\
	return object_t::instance();\
}
#define ECO_NAME(object_t, method)\
inline object_t& method()\
{\
	static object_t s_object;\
	return s_object;\
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif