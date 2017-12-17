#ifndef ECO_WEB_HTML_CLASS_FINDER_IPP
#define ECO_WEB_HTML_CLASS_FINDER_IPP
/*******************************************************************************
@ name
html class parser.

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


//##############################################################################
//##############################################################################
////////////////////////////////////////////////////////////////////////////////
class HtmlClassFinder
{
public:
	std::string m_class_name;
	std::string m_class_content;
	size_t m_parent_pos;

public:
	bool Find(IN OUT std::string& content, IN OUT size_t& find_start)
	{
		const std::string class_start = " class=\"$(";
		const std::string class_end = ")\"";

		// find class start pos.
		size_t find_next = find_start;
		size_t find_pos = static_cast<uint32_t>(
			content.find(class_start, find_next));
		while (find_pos != std::string::npos)
		{
			find_next = find_pos + class_start.size();
			// get html element start pos, which this class is in.
			size_t start_pos = 0;
			if (!GetObjectStart(start_pos, content, find_pos))
			{
				find_pos = content.find(class_start, find_next);
				continue;
			}
			// is in a html annotation.
			if (IsAnnotation(content, start_pos))
			{
				find_pos = content.find(class_start, find_next);
				continue;
			}
			// parse class name.
			size_t class_name_end_pos = content.find(class_end, find_pos);
			if (class_name_end_pos == std::string::npos)
			{
				find_pos = content.find(class_start, find_next);
				continue;
			}
			// parse class content end pos.
			size_t end_pos = 0;
			if (!GetObjectEnd(end_pos, content, start_pos))
			{
				find_pos = content.find(class_start, find_next);
				continue;
			}

			// get result.
			m_parent_pos = start_pos;
			m_class_name = content.substr(
				find_next, class_name_end_pos-find_next); 
			m_class_content = content.substr(start_pos, end_pos - start_pos);
			m_class_content.erase(find_pos - m_parent_pos, 
				class_start.size() + m_class_name.size() + class_end.size());
			// reset content and find_start.
			content.erase(start_pos, end_pos - start_pos);
			find_start = start_pos;
			return true;
		}// end while

		return false;
	}

private:
	// get html element start pos, which class is in.
	bool GetObjectStart(
		OUT size_t& object_start_pos,
		IN  const std::string& content,
		IN  const size_t pos)
	{
		bool has_start_brace = false;
		for (size_t i = pos; i != size_t(-1); --i)
		{
			if (content[i] == '<')
			{
				has_start_brace = true;
			}
			// ignore blank.
			else if (content[i] != ' ' && content[i] != '	' && has_start_brace)
			{
				object_start_pos = i + 1;
				return true;
			}
		}
		return false;
	}

	// is in a html annotation.
	bool IsAnnotation(
		IN  const std::string& content,
		IN  size_t pos)
	{
		for (; pos<content.size(); ++pos)
		{
			if (content[pos] != ' ' && content[pos] != '	')
			{
				break;
			}
		}
		const char* annotation = "<!--";
		return strncmp(&content[pos], annotation, strlen(annotation)) == 0;
	}

	// parse class content end pos.
	bool GetObjectEnd(
		OUT size_t& object_end_pos,
		IN  const std::string& content,
		IN  const size_t pos)
	{
		const std::string ann_start = "<!--";
		const std::string ann_end = "-->";
		const std::string html_element_end_flag = "/>";
		// find the end by find the html element end flag.
		// and the most important is that ignore nested html element.
		// just post and pop.
		std::string temp;
		std::list<std::string> html_elements;
		for (size_t i=pos; i<content.size(); ++i)
		{
			if (content[i] != '<')
			{
				continue;
			}

			// ignore annotation
			if (0 == strncmp(&content[i], ann_start.c_str(), ann_start.size()))
			{
				size_t ann_end_pos = content.find(ann_end, i+ann_start.size());
				if (ann_end_pos == std::string::npos)
				{
					return false;
				}
				i = ann_end_pos + ann_end.size() - 1;
				continue;
			}

			// pop html end: </tr>.
			if (!html_elements.empty() && 0 == strncmp(&content[i],
				html_elements.back().c_str(), html_elements.back().size()))
			{
				size_t ele_end_pos = content.find('>', i);
				if (ele_end_pos == std::string::npos)
				{
					return false;
				}
				html_elements.pop_back();
				if (html_elements.empty())
				{
					object_end_pos = ele_end_pos + 1;
					return true;	// find end.
				}
				i = ele_end_pos;
				continue;
			}
			
			// post html start: <tr>
			size_t end_pos = 0;
			bool has_backslash = false;
			for (size_t j=i+1; j<content.size(); ++j)
			{
				if (content[j] == ' '  || content[i] == '	' ||
					content[j] == '\r' || content[j] == '\n'  ||
					content[j] == '>')
				{
					if (end_pos == 0)
					{
						end_pos = j;
					}
				}
				else if (content[j] == '/')
				{
					has_backslash = true;
				}
				else
				{
					has_backslash = false;
				}

				if (content[j] == '>')
				{
					if (!has_backslash)
					{
						++i;
						temp  = "</";
						temp += content.substr(i, end_pos-i);
						html_elements.push_back(temp);
					}
					i = j;
					break;
				}
			}
		}

		return false;
	}
};


////////////////////////////////////////////////////////////////////////////////
}}// ns
#endif