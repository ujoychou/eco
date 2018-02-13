#ifndef ECO_META_H
#define ECO_META_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Export.h>
#include <eco/meta/Timestamp.h>
#include <string>


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
template<typename object_t>
class Meta
{
public:
	// set value.
	inline void set_value(
		IN const char* prop,
		IN const char* value,
		IN const char* view = nullptr);

	// get value.
	inline std::string get_value(
		IN const char* prop,
		IN const char* view = nullptr);

	// get timestamp.
	inline eco::meta::Timestamp& timestamp()
	{
		return eco::meta::timestamp();
	}

	// create new object: value\raw_ptr\shared_ptr.
	inline void* create()
	{
		return nullptr;
	}

public:
	// attach object
	inline void attach(object_t& obj)
	{
		m_object = (object_t*)&obj;
	}
	inline void attach(const object_t& obj)
	{
		m_object = (object_t*)&obj;
	}
	inline void attach(void* obj)
	{
		m_object = (object_t*)obj;
	}
	inline void attach(object_t* obj)
	{
		m_object = obj;
	}
	inline void attach(const object_t* obj)
	{
		m_object = (object_t*)obj;
	}
	inline void attach(std::shared_ptr<object_t>& obj)
	{
		m_object = (object_t*)obj.get();
	}
	inline void attach(const std::shared_ptr<object_t>& obj)
	{
		m_object = (object_t*)obj.get();
	}

	// reset object
	inline void reset()
	{
		m_object = nullptr;
	}

	// get object_t.
	inline object_t& object()
	{
		return *m_object;
	}
	inline const object_t& get_object()
	{
		return *m_object;
	}

protected:
	object_t* m_object;
};


////////////////////////////////////////////////////////////////////////////////
template<typename ObjectId>
class PropertyValue
{
public:
	ObjectId	m_id;				// object id.
	std::string m_property;			// property name
	std::string m_value;			// Property value.

	inline PropertyValue(
		IN const ObjectId id,
		IN const char* p,
		IN const char* v)
		: m_id(id), m_property(p), m_value(v)
	{}

	inline void set_id(IN const ObjectId id)
	{
		m_id = id;
	}

	inline void set_value(IN const char* p, IN const char* v)
	{
		m_property = p;
		m_value = v;
	}
};


////////////////////////////////////////////////////////////////////////////////
class BindV
{
public:
	inline BindV(IN std::string& val) : m_value(std::move(val))
	{}

	inline operator const char*() const
	{
		return m_value.c_str();
	}

	std::string m_value;
};


////////////////////////////////////////////////////////////////////////////////
}// ns
#endif