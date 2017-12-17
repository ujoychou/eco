#ifndef ECO_WEB_JSON_CLASS_FINDER_IPP
#define ECO_WEB_JSON_CLASS_FINDER_IPP
/*******************************************************************************
@ name

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
namespace web{;
////////////////////////////////////////////////////////////////////////////////
class JsonClassFinder
{
public:
	std::string m_class_name;
	std::string m_class_content;
	size_t m_parent_pos;

public:
	inline bool Find(IN OUT std::string& content, IN OUT size_t& find_start)
	{
		const char array_flag = '[';
		// find class start pos.
		size_t find_next = find_start;
		size_t find_pos = content.find(array_flag, find_next);
		while (find_pos != std::string::npos)
		{
			find_next = find_pos + 1;
			// get html element start pos, which this class is in.
			size_t start_pos = 0;
			if (!GetObjectStart(start_pos, content, find_pos))
			{
				find_pos = content.find(array_flag, find_next);
				continue;
			}
			// parse class content end pos.
			size_t end_pos = 0;
			if (!GetObjectEnd(end_pos, content, start_pos))
			{
				find_pos = content.find(array_flag, find_next);
				continue;
			}
			// parse class name.
			if (!GetClassName(m_class_name, content, start_pos))
			{
				find_pos = content.find(array_flag, find_next);
				continue;
			}

			// get result.
			m_parent_pos = start_pos;
			m_class_content = content.substr(start_pos, end_pos - start_pos);
			// reset content and find_start.
			content.erase(start_pos, end_pos - start_pos);
			find_start = start_pos;
			return true;
		}// end while

		return false;
	}

private:
	inline bool GetObjectStart(
		OUT size_t& object_start_pos,
		IN  const std::string& content,
		IN  const size_t pos)
	{
		size_t find_pos = content.find('{', pos);
		if (find_pos == std::string::npos)
		{
			return false;
		}
		// ignore blank space.
		for (size_t i = find_pos - 1; i != size_t(-1); --i)
		{
			if (content[i] != ' ' && content[i] != '	')
			{
				object_start_pos = i + 1;
				return true;
			}
		}
		return true;
	}

	// parse class content end pos.
	inline bool GetObjectEnd(
		OUT size_t& object_end_pos,
		IN  const std::string& content,
		IN  const size_t pos)
	{
		// find the end by find the html element end flag.
		// and the most important is that ignore nested html element.
		// just post and pop.
		int  brace_start_count = 0;
		for (size_t i=pos; i<content.size(); ++i)
		{
			if (content[i] == '{')
			{
				++brace_start_count;
			}
			else if (content[i] == '}')
			{
				--brace_start_count;
				if (brace_start_count == 0)
				{
					object_end_pos = i + 1;
					return true;	// find end.
				}
			}// end if.
		}// end for.

		return false;
	}

	// get class name.
	inline bool GetClassName(
		OUT std::string& class_name,
		IN  const std::string& content,
		IN  const size_t pos)
	{
		// json format:
		// 1. "var variant_name = ["
		// 2. ""variant_name" : ["
		bool var_flag = false;
		size_t name_end_pos = std::string::npos;
		size_t name_start_pos = std::string::npos;
		for (size_t i = pos; i != size_t(-1); --i)
		{
			if (content[i] == ':' || content[i] == '=')
			{
				var_flag = true;
				continue;
			}
			if (var_flag && name_end_pos == std::string::npos)
			{
				if (content[i]!=' ' && content[i]!='	' &&
					content[i]!='\r' && content[i]!='\n')
				{
					name_end_pos = i + 1;
					continue;
				}
			}
			if (var_flag && name_end_pos != std::string::npos)
			{
				if (i == 0)
				{
					name_start_pos = i;
					break;
				}
				if (content[i]==' ' || content[i]=='	' || 
					content[i]=='\r' || content[i]=='\n')
				{
					name_start_pos = i + 1;
					break;
				}
			}// end if.
		}

		if (!var_flag || 
			name_end_pos == std::string::npos ||
			name_start_pos == std::string::npos)
		{
			return false;
		}
		class_name = content.substr(
			name_start_pos, name_end_pos-name_start_pos);
		// remove '"'.
		if (class_name.size() > 0 && class_name.back() == '"')
		{
			class_name.erase(class_name.size()-1, 1);
		}
		if (class_name.size() > 0 && class_name.front() == '"')
		{
			class_name.erase(0, 1);
		}
		return true;
	}
};


////////////////////////////////////////////////////////////////////////////////
}}// ns
#endif