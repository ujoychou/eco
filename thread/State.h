#ifndef ECO_ATOMIC_STATE_H
#define ECO_ATOMIC_STATE_H
/*******************************************************************************
@ name
thread safe object state.

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/thread/Atomic.h>



namespace eco{;
namespace atomic{;


////////////////////////////////////////////////////////////////////////////////
class State
{
	ECO_NONCOPYABLE(State);
public:
	// state flag.
	enum 
	{
		_a  = 0x0001,
		_b  = 0x0002,
		_c  = 0x0004,
		_d  = 0x0008,
		_e  = 0x0010,
		_f  = 0x0020,
		_g  = 0x0040,
		_h  = 0x0080,
		_i  = 0x0100,
		_j  = 0x0200,
		_k  = 0x0400,
		_l  = 0x0800,
		_m  = 0x1000,
		_n  = 0x2000,
		_o  = 0x4000,
		_p  = 0x8000,
		_aa = 0x00010000,
		_bb = 0x00020000,
		_cc = 0x00040000,
		_dd = 0x00080000,
		_ee = 0x00100000,
		_ff = 0x00200000,
		_gg = 0x00400000,
		_hh = 0x00800000,
		_ii = 0x01000000,
		_jj = 0x02000000,
		_kk = 0x04000000,
		_ll = 0x08000000,
		_mm = 0x10000000,
		_nn = 0x20000000,
		_oo = 0x40000000,
		_pp = 0x80000000,

		// ok flag.
		_no = 0x0000,
		_ok = 0x1000,
	};

public:
	inline State();

	inline State(IN const uint32_t v);

	inline void none();

	inline void ok();

	inline bool is_none() const;

	inline bool is_ok() const;

public:
	// add state.
	inline void add(IN const uint32_t v);

	// delete state.
	inline void del(IN const uint32_t v);

	// asign value to state.
	inline void set(IN bool is, IN const uint32_t v);

	// delete state.
	inline void set_v(
		IN const uint32_t add_v,
		IN const uint32_t del_v);

	// asign value to state.
	inline void operator=(IN const uint32_t v);

	// whether has dedicated state.
	inline bool has(IN const uint32_t v) const;

	// whether two state is equal.
	inline bool operator==(IN const State& state) const;

	// whether this state is equal to the value.
	inline bool operator==(IN const int state) const;
	inline bool operator==(IN const uint32_t state) const;

	// get uint32_t value.
	inline uint32_t value() const;

	// get uint32_t value.
	inline operator uint32_t() const;

private:
	eco::Atomic<uint32_t> m_value;
};


////////////////////////////////////////////////////////////////////////////////
State::State() : m_value(_no)
{}
State::State(IN const uint32_t v) : m_value(v)
{}
void State::none()
{
	m_value = _no;
}
bool State::is_none() const
{
	return (m_value == _no);
}
void State::ok()
{
	add(_ok);
}
bool State::is_ok() const
{
	return has(_ok);
}
void State::add(IN const uint32_t v)
{
	m_value |= v;
}
void State::del(IN const uint32_t v)
{
	m_value &= ~v;
}
void State::set_v(IN const uint32_t add_v, IN const uint32_t del_v)
{
	add(add_v);
	del(del_v);
}
void State::operator=(IN const uint32_t v)
{
	m_value = v;
}
void State::set(bool is, IN const uint32_t v)
{
	return is ? add(v) : del(v);
}
bool State::has(IN const uint32_t v) const
{
	return (m_value & v) > 0;
}
bool State::operator==(IN const State& state) const
{
	return m_value == state.m_value;
}
bool State::operator==(IN const uint32_t v) const
{
	return m_value == v;
}
bool State::operator==(IN const int v) const
{
	return m_value == v;
}
uint32_t State::value() const
{
	return m_value;
}
State::operator uint32_t() const
{
	return m_value;
}


////////////////////////////////////////////////////////////////////////////////
}}// ns::eco.atomic
#endif