#ifndef ECO_ANY_H
#define ECO_ANY_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Type.h>


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

	inline Any(eco::Null) : m_data(nullptr)
	{}
	inline Any& operator=(eco::Null)
	{
		release();
	}

	template<typename T>
	inline Any(IN const T& v) : m_data(new DataT<T>(v))
	{}

	inline Any(IN const char* v) : m_data(new DataT<std::string>(v))
	{}

	inline Any(IN bool v) : m_data(new DataT<uint32_t>(v))
	{}

	inline Any(IN char v) : m_data(new DataT<uint32_t>(v))
	{}

	inline Any(IN unsigned char v) : m_data(new DataT<uint32_t>(v))
	{}

	inline Any(IN short v) : m_data(new DataT<uint32_t>(v))
	{}

	inline Any(IN unsigned short v) : m_data(new DataT<uint32_t>(v))
	{}

	inline Any(IN int v) : m_data(new DataT<uint32_t>(v))
	{}

	inline Any(IN unsigned int v) : m_data(new DataT<uint32_t>(v))
	{}

	inline Any(IN long v) : m_data(new DataT<uint32_t>(v))
	{}

	inline Any(IN unsigned long v) : m_data(new DataT<uint32_t>(v))
	{}

	inline Any(IN int64_t v) : m_data(new DataT<uint64_t>(v))
	{}

	inline Any(IN uint64_t v) : m_data(new DataT<uint64_t>(v))
	{}

	inline Any(IN float v) : m_data(new DataT<float>(v))
	{}

	inline Any(IN double v) : m_data(new DataT<double>(v))
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
		return (value_t&)cast<value_t>((const Any&)any);
	}
	template<typename value_t>
	inline static const value_t& cast(const Any& any)
	{
		const value_t* v = cast<value_t>(&any);
		if (v == nullptr)
		{
			eco::FixFormat fmt("cast eco::any fail, % to %");
			fmt % any.get_type_id() % typeid(value_t).name();
			throw std::logic_error(fmt);
		}
		return *v;
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

public:
	inline operator const char*() const
	{
		return eco::Any::cast<std::string>(*this).c_str();
	}
	inline operator const bool() const
	{
		return eco::Any::cast<uint32_t>(*this) != 0;
	}
	inline operator const char() const
	{
		return eco::Any::cast<uint32_t>(*this);
	}
	inline operator const unsigned char() const
	{
		return eco::Any::cast<uint32_t>(*this);
	}
	inline operator const short() const
	{
		return eco::Any::cast<uint32_t>(*this);
	}
	inline operator const unsigned short() const
	{
		return eco::Any::cast<uint32_t>(*this);
	}
	inline operator const int() const
	{
		return eco::Any::cast<uint32_t>(*this);
	}
	inline operator const unsigned int() const
	{
		return eco::Any::cast<uint32_t>(*this);
	}
	inline operator const long() const
	{
		return eco::Any::cast<uint32_t>(*this);
	}
	inline operator const unsigned long() const
	{
		return eco::Any::cast<uint32_t>(*this);
	}
	inline operator const int64_t() const
	{
		return eco::Any::cast<uint64_t>(*this);
	}
	inline operator const uint64_t() const
	{
		return eco::Any::cast<uint64_t>(*this);
	}
	inline operator const float() const
	{
		return eco::Any::cast<float>(*this);
	}
	inline operator const double() const
	{
		return eco::Any::cast<double>(*this);
	}

public:
	inline std::string format(int prec = 0) const
	{
		auto d = eco::Any::cast<double>(this);
		if (d != nullptr) return eco::Double(*d, prec).c_str();
		auto sz = eco::Any::cast<std::string>(this);
		if (sz != nullptr) return *sz;
		auto i32 = eco::Any::cast<uint32_t>(this);
		if (i32 != nullptr) return eco::Integer<uint32_t>(*i32).c_str();
		auto i64 = eco::Any::cast<uint64_t>(this);
		if (i64 != nullptr) return eco::Integer<uint64_t>(*i64).c_str();
		auto f = eco::Any::cast<float>(this);
		if (f != nullptr) return eco::Double(*f, prec).c_str();
		return eco::empty_str;
	}

private:
	Data* m_data;
};
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif