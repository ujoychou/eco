#ifndef ECO_FILESYSTEM_SOURCE_FILE_H
#define ECO_FILESYSTEM_SOURCE_FILE_H
/*******************************************************************************
@ name
source file.

@ function
1.get source file name.

@ exception

@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <cstring>


namespace eco{;
namespace filesystem{;
////////////////////////////////////////////////////////////////////////////////
class ECO_API SourceFile
{
public:
	explicit SourceFile(IN const char* filename, IN int line)
		: m_full_name(filename)
		, m_line(line)
	{
		if (nullptr != (m_name = strrchr(filename, '/')) ||
			nullptr != (m_name = strrchr(filename, '\\')))
		{
			m_name = m_name + 1;
		}
		else
		{
			m_name = filename;
		}
	}
	inline const char* get_name() const
	{
		return m_name;
	}
	inline const char* get_full_name() const
	{
		return m_full_name;
	}
	inline int get_line() const
	{
		return m_line;
	}

private:
	int m_line;
	const char* m_name;
	const char* m_full_name;
};

}}
#endif