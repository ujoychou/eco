#ifndef ECO_TEST_TRACE_H
#define ECO_TEST_TRACE_H

/*******************************************************************************
@ name
exception.

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-01-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Project.h>


namespace eco{;
namespace test{;
////////////////////////////////////////////////////////////////////////////////
class Trace
{
////////////////////////////////////////////////////////////////////////////////
public:
	inline Trace() : m_file_line(-1)
	{}

	inline explicit Trace(IN const std::string& trace_msg) :
	m_file_line(-1)
	{}

	inline Trace(
		IN const std::string& file_path,
		IN const long file_line,
		IN const std::string& func_name)
		: m_file_path(file_path)
		, m_file_line(file_line)
		, m_func_name(func_name)
	{}

	inline Trace(IN const Trace& tracer)
		: m_file_path(tracer.m_file_path)
		, m_file_line(tracer.m_file_line)
		, m_func_name(tracer.m_func_name)
	{}

	~Trace()
	{}

	// get file name where exception throwed.
	inline const std::string& GetFilePath() const
	{
		return m_file_path;
	}

	// get file line where exception throwed.
	inline const long GetFileLine() const
	{
		return m_file_line;
	}

	// get function name where exception throwed.
	inline const std::string& GetFuncName() const
	{
		return m_func_name;
	}

////////////////////////////////////////////////////////////////////////////////
public:
	// get trace format string.
	inline std::string Format() const
	{
		eco::FixStream fmt;
		fmt << m_file_path << ", " << m_func_name << ", " << m_file_line;
		return fmt.c_str();
	}

////////////////////////////////////////////////////////////////////////////////
private:
	// file name where exception throwed.
	std::string m_file_path;

	// file line where exception throwed.
	long m_file_line;

	// function where exception throwed.
	std::string m_func_name;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif