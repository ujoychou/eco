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
#include <memory>




namespace eco{;


////////////////////////////////////////////////////////////////////////////////
#define ECO_OBJECT(object_t) \
public:\
	typedef object_t object;\
	typedef std::shared_ptr<object_t> value;\
	typedef std::shared_ptr<object_t> ptr;\
	typedef std::weak_ptr<object_t> wptr;\
	ECO_NONCOPYABLE(object_t);

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
// make object.
template<typename T>
inline static void make(T&) {}
template<typename T>
inline static void make(std::shared_ptr<T>& ptr) { ptr.reset(new T()); }

// get object.
template<typename T>
inline T& object(T& obj) { return obj; }
template<typename T>
inline T& object(std::shared_ptr<T>& ptr) {	return *ptr; }
template<typename T>
inline const T& get_object(const T& obj) { return obj; }
template<typename T>
inline const T& get_object(const std::shared_ptr<T>& ptr) { return *ptr; }

// get decltype type.
template<typename T>
inline T& object_v() {	static T* t = nullptr;	return *t; }
template<typename T>
inline T* object_t(T& obj) { return &obj; }
template<typename T>
inline T* object_t(std::shared_ptr<T>& ptr) { return ptr.get(); }


////////////////////////////////////////////////////////////////////////////////
// singleton proxy object that instantiate the object.
template<typename ObjectType>
class Singleton
{
	ECO_OBJECT(Singleton);
public:
	inline static ObjectType& instance()
	{
		return s_object;
	}
private:
	static ObjectType s_object;
};
template<typename ObjectType>
ObjectType Singleton<ObjectType>::s_object;


/*@ singleton instance to access singleton object. */
#define ECO_SINGLETON(ObjectType)\
	ECO_NONCOPYABLE(ObjectType);\
public:\
	inline ~ObjectType(){}\
private:\
	friend class eco::Singleton<ObjectType>;\
	inline ObjectType(){}

#define ECO_SINGLETON_UNINIT(ObjectType)\
	ECO_NONCOPYABLE(ObjectType);\
public:\
	friend class eco::Singleton<ObjectType>;\
	~ObjectType();\
private:\
	ObjectType();


/*@ singleton get function to access singleton object.*/
#define ECO_SINGLETON_NAME(ObjectType, method)\
inline ObjectType& method()\
{\
	return eco::Singleton<ObjectType>::instance();\
}


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif