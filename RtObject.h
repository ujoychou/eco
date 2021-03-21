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
1.make and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Object.h>
#include <map>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
// runtime object implement.
#define ECO_RTX__(object_t, parent_t, create_func) \
ECO_TYPE__(object_t)\
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
#define ECO_RTX(object_t, parent_t)\
ECO_RTX__(object_t, parent_t, &object_t::make)\
inline static eco::RtObject::ptr make()\
{\
	return eco::RtObject::ptr(new object_t());\
}
// runtime object who is a interface cann't be created.
#define ECO_RTI(object_t, parent_t) \
ECO_RTX__(object_t, parent_t, nullptr)


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
typedef eco::RtObject::ptr (*MakeRtObject)(void);


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
	typedef RtObject::ptr (*MakeObject)(void);

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
ECO_NS_END(eco);
#endif