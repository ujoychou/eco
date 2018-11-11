#ifndef ECO_ANY_H
#define ECO_ANY_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Export.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class Any
{
////////////////////////////////////////////////////////////////////////////////
	class Data
	{
	public:
		inline Data() {}
		virtual ~Data() {}
		virtual void* get_value() = 0;
		virtual const uint32_t get_type_id() const = 0;
		virtual Data* copy() const = 0;
		virtual void copy(const Data* data) = 0;
	};

	template<typename T>
	class DataT : public Data
	{
	public:
		inline DataT(IN const T& v) : m_value(v)
		{}
		virtual ~DataT()
		{}
		virtual void* get_value() override
		{
			return &m_value;
		}
		virtual const uint32_t get_type_id() const override
		{
			return eco::TypeId<T>::value;
		}
		virtual Data* copy() const override
		{
			return new DataT<T>(m_value);
		}
		virtual void copy(const Data* data) override
		{
			m_value = ((DataT<T>*)data)->m_value;
		}

	private:
		T m_value;
	};


////////////////////////////////////////////////////////////////////////////////
public:
	// constructor
	inline Any() : m_data(nullptr)
	{}
	inline ~Any()
	{
		release();
	}

	template<typename T>
	inline explicit Any(IN const T& v) : m_data(new DataT<T>(v))
	{}
	inline Any(IN const Any& v) : m_data(v.null() ? v.m_data : v.m_data->copy())
	{}
	inline Any(IN Any&& v) : m_data(v.m_data)
	{
		v.m_data = nullptr;
	}

	// constructor operator =.
	template<typename T>
	inline Any& operator=(IN const T& v)
	{
		if (null())
		{
			m_data = new DataT<T>(v);
		}
		else if (eco::TypeId<T>::value == get_type_id())
		{
			m_data->copy(v.m_data);
		}
		else
		{
			release();
			if (!v.null()) m_data = new DataT<T>(v);
		}
		return *this;
	}
	inline Any& operator=(IN const Any& v)
	{
		if (null())
		{
			if (!v.null()) m_data = v.m_data->copy();
		}
		else if (get_type_id() == v.get_type_id())
		{
			m_data->copy(v.m_data);
		}
		else
		{
			release();
			if (!v.null()) m_data = v.m_data->copy();
		}
		return *this;
	}
	inline Any& operator=(IN Any&& v)
	{
		m_data = v.m_data;
		v.m_data = nullptr;
		return *this;
	}

public:
	inline void swap(IN Any& v)
	{
		auto temp = m_data;
		m_data = v.m_data;
		v.m_data = temp;
	}

	inline void release()
	{
		delete m_data;
		m_data = nullptr;
	}

	inline bool null() const
	{
		return m_data == nullptr;
	}

	inline operator bool() const
	{
		return m_data == nullptr;
	}

	// content type.
	inline const uint32_t get_type_id() const
	{
		return m_data->get_type_id();
	}
	inline bool same_type_id(const uint32_t type_id) const
	{
		return !null() && type_id == get_type_id();
	}

public:
	// static get content object.
	template<typename value_t>
	inline static value_t* cast(Any* any)
	{
		return (value_t*)cast<value_t>((const Any*)any);
	}
	template<typename value_t>
	inline static const value_t* cast(const Any* any)
	{
		return any->same_type_id(eco::TypeId<value_t>())
			? (const value_t*)(any->m_data->get_value()) : nullptr;
	}
	template<typename value_t>
	inline static value_t& cast(Any& any)
	{
		return *cast<value_t>(&any);
	}
	template<typename value_t>
	inline static const value_t& cast(const Any& any)
	{
		return *cast<value_t>(&any);
	}

	// get content object.
	template<typename value_t>
	inline value_t* cast_ptr()
	{
		return eco::Any::cast<value_t>(this);
	}
	template<typename value_t>
	inline const value_t* cast_ptr() const
	{
		return eco::Any::cast<value_t>(this);
	}
	template<typename value_t>
	inline value_t& cast()
	{
		return eco::Any::cast<value_t>(*this);
	}
	template<typename value_t>
	inline const value_t& cast() const
	{
		return eco::Any::cast<value_t>(*this);
	}

private:
	Data* m_data;
};


////////////////////////////////////////////////////////////////////////////////
}
#endif