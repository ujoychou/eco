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
	inline eco::meta::Timestamp& timestamp();

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
}// ns
#endif