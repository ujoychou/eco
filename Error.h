#ifndef ECO_ERROR_H
#define ECO_ERROR_H
/*******************************************************************************
@ name
error and error thrower.

@ function
1.define eco error include id and message.
2.thrower support io stream.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Type.h>
#include <exception>
#include <iostream>



namespace eco{;

////////////////////////////////////////////////////////////////////////////////
// get default Error log format.
class Error
{
public:
	inline explicit Error(IN const int eid = 0) : m_id(eid)
	{}

	inline explicit Error(
		IN const int eid,
		IN const std::string& msg)
		: m_id(eid)
	{
		m_msg << msg;
	}

	inline explicit Error(
		IN const std::string& msg,
		IN const int eid)
		: m_id(eid)
	{
		m_msg << msg;
	}

	virtual ~Error() throw()
	{}

	inline void set_error(
		IN const int eid,
		IN const std::string& msg)
	{
		m_id = eid;
		m_msg.buffer().clear();
		m_msg << msg;
	}

	// error code.
	inline Error& id(IN const int eid)
	{
		m_id = eid;
		return *this;
	}
	inline int id() const
	{
		return m_id;
	}

	// error message.
	template<typename value_type>
	inline Error& operator<<(IN const value_type& v)
	{
		m_msg << v;
		return (*this);
	}
	inline char* buffer()
	{
		return m_msg.buffer().buffer(0);
	}
	inline const char* message() const
	{
		return m_msg.get_buffer().c_str();
	}
	virtual const char* what() const throw()
	{
		return m_msg.get_buffer().c_str();
	}

	inline operator bool() const
	{
		return m_id != 0;
	}

private:
	int m_id;
	FixStream<256> m_msg;
};
#define EcoFmt(err) (err).message() << " #" << (err).id()


////////////////////////////////////////////////////////////////////////////////
class ErrorRecord
{
public:
	template<typename value_type>
	inline ErrorRecord& operator<<(IN const value_type& v)
	{
		m_msg << v;
		return (*this);
	}

	inline ErrorRecord(IN int eid = -1) : m_eid(eid)
	{}

private:
	friend class Thrower;
	friend class LogicThrower;
	int m_eid;
	FixStream<256> m_msg;
};


////////////////////////////////////////////////////////////////////////////////
class Thrower
{
public:
	inline Thrower() {}
	inline void operator=(IN const ErrorRecord& that)
	{
		throw eco::Error(that.m_eid, that.m_msg.get_buffer().c_str());
	}
};
// throw error with stream input way.
#define EcoThrow(eid) eco::Thrower() = eco::ErrorRecord(eid)
#define EcoThrowError eco::Thrower() = eco::ErrorRecord(eco::error)



//##############################################################################
//##############################################################################
class Cout
{
public:
	template<typename value_t>
	inline Cout& operator<<(IN const value_t& val)
	{
		std::cout << val;
		return (*this);
	}
	
	inline ~Cout()
	{
		for (uint8_t i=0; i<m_turn_line; ++i)
		{
			std::cout << std::endl;
		}
	}

	inline explicit Cout(IN const uint8_t turn_line) : m_turn_line(turn_line)
	{}

private:
	uint8_t m_turn_line;
};


////////////////////////////////////////////////////////////////////////////////
// print stream message to std out auto append std::endl.
inline Cout cout(IN uint8_t turn_line = 1)
{
	return Cout(turn_line);
}
#define EcoCout eco::cout()


}//ns.eco
#endif