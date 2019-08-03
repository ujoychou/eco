#ifndef ECO_RT_OBJECT_H
#define ECO_RT_OBJECT_H
/*******************************************************************************
@ name
	strategy design patten, runtime object and class.

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Object.h>
#include <map>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
#define ECO_RTX(object_t, parent_t) ECO_TYPE__(object_t)\
public:\
	typedef std::weak_ptr<object_t> wptr;\
	typedef std::shared_ptr<object_t> ptr;\
	inline static const char* class_name()\
	{\
		return type_name();\
	}\
	inline static eco::RtObject::ptr create()\
	{\
		return eco::RtObject::ptr(new object_t());\
	}\
	inline static const eco::RtClass* clss()\
	{\
		static eco::RtClass s_clss(\
			parent_t::clss(), class_name(), type_id(), &object_t::create,\
			&eco::Singleton<eco::RtClassInit<object_t> >::instance());\
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
typedef eco::RtObject::ptr (*CreateRtObject)(void);


////////////////////////////////////////////////////////////////////////////////
template<typename t>
struct RtClassInit
{
	inline RtClassInit()
	{
		t::clss();
	}
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
	typedef RtObject::ptr (*CreateObject)(void);

	// create class in rtobject.
	inline RtClass(
		IN const RtClass* parent,
		IN const char* name, 
		IN const uint32_t type_id,
		IN CreateObject func,
		IN void* v)
		: eco::Type(type_id, name, parent, v), m_create(func)
	{
		RtClassMap<>::s_map[name] = this;
	}

	// get base(parent) class information which is used to hierarchy traverse.
	inline const RtClass* parent() const
	{
		return (const RtClass*)eco::Type::parent();
	}

public:
	// create object of this class.
	inline RtObject::ptr create() const
	{
		return m_create();
	}
	static inline RtObject::ptr create(const std::string& name)
	{
		auto it = RtClassMap<>::s_map.find(name);
		return (it != RtClassMap<>::s_map.end())
			? it->second->create() : RtObject::ptr();
	}
	template<typename object_t>
	static inline typename object_t::ptr create()
	{
		return std::dynamic_pointer_cast<object_t>(
			create(object_t::class_name()));
	}
	template<typename object_t>
	static inline typename object_t::ptr create(const std::string& name)
	{
		return std::dynamic_pointer_cast<object_t>(create(name));
	}

private:
	CreateObject m_create;
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
ECO_NS_END(eco);
#endif