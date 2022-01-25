#ifndef ECO_RTYPE_H
#define ECO_RTYPE_H
/*******************************************************************************
@ name
rt type.

@ function
1 runtime type. exp: mfc runtime create.
2 typeid/gettypeid/type/class/object.
3 ECO_TYPE/ECO_RTYPE.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2021-11-20


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Object.h>
#include <map>
#include <memory>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class RtClass;
class RtObject : public std::enable_shared_from_this<eco::RtObject>
{
	ECO_OBJECT(RtObject);
public:
	RtObject() {}
	virtual ~RtObject(void) {}

	// get class info of this object.
	virtual const RtClass* get_clss() const = 0;

	// get the class info of this object.
	inline static const RtClass* clss()
	{
		return nullptr;
	}

	// cast one object to current class's instance object.
	template<typename object_t>
	inline typename object_t::ptr cast()
	{
		return std::dynamic_pointer_cast<object_t>(shared_from_this());
	}

	// get class name.
	inline const char* get_class_name();

	// check is the same kind of class.
	inline bool kind_of(const RtObject& obj) const;
	template<typename object_t> inline bool kind_of() const;
};
typedef eco::RtObject::ptr (*MakeObject)(void);


////////////////////////////////////////////////////////////////////////////////
class Type
{
	ECO_NONCOPYABLE(Type);
public:
	// init.
	inline Type(uint32_t id, const char* name, const Type* parent, void*)
		: m_id(id), m_name(name), m_parent(parent) {}

	// get type id of this type.
	inline uint32_t id() const { return m_id; }

	// get type name of this type.
	inline const char* name() const { return m_name; }

	// get parent of this type.
	inline const Type* parent() const { return m_parent; }

	// is kind of runtime class.
	inline bool kind_of(IN const Type* clss_sup) const
	{
		const Type* me = this;
		for (; me != nullptr && clss_sup != me; me = me->m_parent) {}
		return (me != nullptr);
	}

private:
	const uint32_t m_id;
	const char* m_name;
	const Type* m_parent;
};
template<typename t>
struct TypeInit
{
	inline TypeInit() { t::type(); }
};
template<typename t = int>
class RtClassMap
{
	static std::map<std::string, const RtClass*> s_map;
	friend class RtClass;
};
template<typename t>
std::map<std::string, const RtClass*> RtClassMap<t>::s_map;


////////////////////////////////////////////////////////////////////////////////
class RtClass : public eco::Type
{
public:
	// make class in rtobject.
	inline RtClass(
		IN const RtClass* parent,
		IN const char* name, 
		IN const uint32_t type_id,
		IN MakeObject func,
		IN void* v)
		: eco::Type(type_id, name, parent, v), m_make(func)
	{
		RtClassMap<>::s_map[name] = this;
	}

	// get base(parent) class information which is used to hierarchy traverse.
	inline const RtClass* parent() const
	{
		return (const RtClass*)eco::Type::parent();
	}

public:
	// make object of this class.
	inline RtObject::ptr make() const
	{
		return m_make();
	}
	static inline RtObject::ptr make(const std::string& name)
	{
		auto it = RtClassMap<>::s_map.find(name);
		return (it != RtClassMap<>::s_map.end())
			? it->second->make() : RtObject::ptr();
	}
	template<typename object_t>
	static inline typename object_t::ptr make()
	{
		return std::dynamic_pointer_cast<object_t>(
			make(object_t::class_name()));
	}
	template<typename object_t>
	static inline typename object_t::ptr make(const std::string& name)
	{
		return std::dynamic_pointer_cast<object_t>(make(name));
	}

private:
	MakeObject m_make;
};


////////////////////////////////////////////////////////////////////////////////
inline const char* RtObject::get_class_name()
{
	return get_clss()->name();
}
inline bool RtObject::kind_of(const RtObject& obj) const
{
	return get_clss()->kind_of(obj.get_clss());
}
template<typename object_t>
inline bool RtObject::kind_of() const
{
	return get_clss()->kind_of(object_t::clss());
}


////////////////////////////////////////////////////////////////////////////////
template<typename t>
struct RtClassInit
{
	inline RtClassInit()
	{
		t::clss();
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename uint_t>
class GetTypeIdT
{
public:
	static inline uint32_t get_id(const char* type_name)
	{
		auto it = s_map.find(type_name);
		return it != s_map.end() ? it->second : 0;
	}

protected:
	static inline uint32_t init_id(const char* type_name)
	{
		auto it = s_map.find(type_name);
		if (it != s_map.end()) { return it->second; }
		return s_map[type_name] = ++s_next_id;
	}

	template<typename type_t>
	static inline uint32_t init_id()
	{
		return init_id(typeid(type_t).name());
	}

	static uint_t s_next_id;
	static std::map<std::string, uint_t> s_map;
};
template<typename uint_t>
uint_t GetTypeIdT<uint_t>::s_next_id = 0;
template<typename uint_t>
std::map<std::string, uint_t> GetTypeIdT<uint_t>::s_map;
typedef GetTypeIdT<uint32_t> GetTypeId;


////////////////////////////////////////////////////////////////////////////////
// c++ class type id.
template<typename type_t>
class TypeId : public GetTypeId
{
public:
	inline operator uint32_t() const
	{
		return value;
	}
	static const uint32_t value;
};
template<typename type_t>
const uint32_t TypeId<type_t>::value = GetTypeId::init_id<type_t>();
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);


////////////////////////////////////////////////////////////////////////////////
#define ECO_TYPE_NAME(type_t) #type_t
#define ECO_TYPE_1(type_t) ECO_TYPE__(type_t)\
public:\
	inline static const eco::Type* type()\
	{\
		static eco::Type t(type_id(), #type_t, nullptr,\
			&eco::Singleton<eco::TypeInit<type_t> >::get());\
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
			&eco::Singleton<eco::TypeInit<type_t> >::get());\
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
		return ECO_TYPE_NAME(type_t);\
	}\
	virtual const char* get_type_name() const\
	{\
		return type_name();\
	}
#define ECO_TYPE(...) ECO_MACRO(ECO_TYPE_,__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
// runtime object implement.
#define ECO_RTYPE__(object_t, parent_t, create_func) \
ECO_RTYPE__(object_t)\
public:\
	typedef std::weak_ptr<object_t> wptr;\
	typedef std::shared_ptr<object_t> ptr;\
	inline static const char* class_name()\
	{\
		return type_name();\
	}\
	inline static const eco::RtClass* clss()\
	{\
		static eco::RtClass s_clss(\
			parent_t::clss(), class_name(), type_id(), create_func,\
			&eco::Singleton<eco::RtClassInit<object_t> >::get());\
		return &s_clss;\
	}\
	virtual const eco::RtClass* get_clss() const override\
	{\
		return clss();\
	}\
	inline static const eco::Type* type()\
	{\
		return clss();\
	}
// runtime object who is a instance can be created.
#define ECO_RTYPE(object_t, parent_t)\
ECO_RTYPE__(object_t, parent_t, &object_t::make)\
inline static eco::RtObject::ptr make()\
{\
	return eco::RtObject::ptr(new object_t());\
}
// runtime object who is a interface cann't be created.
#define ECO_RTI(object_t, parent_t) \
ECO_RTYPE__(object_t, parent_t, nullptr)


////////////////////////////////////////////////////////////////////////////////
#endif