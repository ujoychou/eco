#ifndef ECO_ATOMIC_H
#define ECO_ATOMIC_H
/*******************************************************************************
@ name
atomic sync.

@ function

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/thread/AtomicImpl.h>



namespace eco{;


////////////////////////////////////////////////////////////////////////////////
template<typename integral>
class Atomic
{
public:
	inline Atomic() : m_data(0)
	{}

	inline explicit Atomic(IN integral v) : m_data(v)
	{}

	inline integral load(IN integral v)
	{
		atomic::exchange_add(&m_data, v - m_data);
		return m_data;
	}
	inline integral get_value() const
	{
		return m_data;
	}
	inline operator integral() const
	{
		return m_data;
	}
	inline Atomic& operator=(IN integral v)
	{
		load(v);
		return *this;
	}

public:
	inline integral operator++()
	{
		atomic::exchange_add(&m_data, 1);
		return m_data;
	}
	inline integral operator++(int)
	{
		return atomic::exchange_add(&m_data, 1);
	}
	inline integral operator--()
	{
		atomic::exchange_add(&m_data, -1);
		return m_data;
	}
	inline integral operator--(int)
	{
		return atomic::exchange_add(&m_data, -1);
	}
	inline integral operator+=(IN integral v)
	{
		atomic::exchange_add(&m_data, v);
		return m_data;
	}
	inline integral operator-=(IN integral v)
	{
		atomic::exchange_add(&m_data, v * -1);
		return m_data;
	}
	inline integral operator*=(IN integral v)
	{
		return load(m_data * v);
	}
	inline integral operator/=(IN integral v)
	{
		return load(m_data / v);
	}
	inline integral operator&=(IN integral v)
	{
		return load(m_data & v);
	}
	inline integral operator|=(IN integral v)
	{
		return load(m_data | v);
	}

private:
	volatile integral m_data;
};


////////////////////////////////////////////////////////////////////////////////
}
#endif