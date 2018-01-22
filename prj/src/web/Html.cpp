#include "PrecHeader.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/filesystem/Operations.h>
#include <eco/filesystem/File.h>
#include <eco/web/Html.h>
#include <eco/Typex.ipp>
#include <boost/algorithm/string/replace.hpp>
#include "HtmlClassFinder.ipp"


namespace eco{;
namespace web{;
////////////////////////////////////////////////////////////////////////////////
class HtmlClass::Impl
{
	ECO_IMPL_INIT(HtmlClass);
public:
	// html class identity.
	std::string m_name;

	// html class text detail.
	std::string m_content;

	// html position in parent context.
	size_t m_position;

	// html children.
	std::list<HtmlClass> m_children;

	Impl() : m_position(0)
	{}
};


ECO_SHARED_IMPL(HtmlClass);
////////////////////////////////////////////////////////////////////////////////
const char* HtmlClass::GetName() const
{
	return impl().m_name.c_str();
}
const char* HtmlClass::GetContent() const
{
	return impl().m_content.c_str();
}
size_t HtmlClass::GetPosition() const
{
	return impl().m_position;
}
HtmlClass HtmlClass::AddChild()
{
	HtmlClass new_child;
	impl().m_children.push_back(new_child);
	return new_child;
}
HtmlClass HtmlClass::FindChild(IN const char* class_name)
{
	auto it = impl().m_children.begin();
	for (; it!=impl().m_children.end(); ++it)
	{
		if (it->impl().m_name == class_name)
		{
			return *it;
		}
	}
	return HtmlClass();
}
int HtmlClass::Compare(
	IN const HtmlClass& child_class_1,
	IN const HtmlClass& child_class_2)
{
	if (child_class_1 == child_class_2)
	{
		return 0;
	}

	auto it = impl().m_children.begin();
	for (; it!=impl().m_children.end(); ++it)
	{
		// first find the class is a previous html class.
		if (*it == child_class_1)
		{
			return -1;
		}
		if (*it == child_class_2)
		{
			return 1;
		}
	}

	throw std::logic_error("compare child class fail, can't find child class.");
	return 0;
}


//##############################################################################
//##############################################################################
////////////////////////////////////////////////////////////////////////////////
class HtmlObject::Impl
{
	ECO_IMPL_INIT(HtmlObject);
public:
	int m_pos_mode;
	HtmlClass m_class;
	std::map<std::string, std::string> m_prop_values;
	std::list<HtmlObject> m_children;

	inline Impl() : m_pos_mode(0)
	{}

public:
	void MakeText(OUT std::string& text) const
	{
		std::string cur_text = m_class.GetContent();

		// make child objects text.
		size_t last_pos = 0;
		size_t child_pos = 0;
		std::string child_text;
		for (auto itc = m_children.begin(); itc != m_children.end(); ++itc)
		{
			HtmlClass child_class = itc->GetClass();
			// first time add a new child class.
			if (last_pos == child_class.GetPosition() ||
				itc->GetPositionMode() == HtmlObject::order_pos)
			{
				itc->impl().MakeText(child_text);
				cur_text.insert(child_pos, child_text);
				child_pos += child_text.size();
			}
			// add child class item order by order.
			else
			{
				// get diff pos between last and current html class.
				// new_pos = last_pos + (cur_class_pos - last_class_pos)
				child_pos += child_class.GetPosition() - last_pos;
				last_pos  = child_class.GetPosition();
				itc->impl().MakeText(child_text);
				cur_text.insert(child_pos, child_text);
				child_pos += child_text.size();
			}
		}// end for.

		// replace property value.
		std::string key;
		auto itv = m_prop_values.begin();
		for (; itv!=m_prop_values.end(); ++itv)
		{
			key  = "$(";
			key += itv->first;
			key += ')';
			boost::algorithm::replace_all(cur_text, key, itv->second);
		}
		text.swap(cur_text);
	}
};
ECO_SHARED_IMPL(HtmlObject);
////////////////////////////////////////////////////////////////////////////////
void HtmlObject::MakeText(OUT eco::StringAny& text) const
{
	impl().MakeText(text.impl().m_value);
}
HtmlClass HtmlObject::GetClass() const
{
	return impl().m_class;
}
HtmlObject::PositionMode HtmlObject::GetPositionMode() const
{
	return impl().m_pos_mode;
}
void HtmlObject::SetValue(
	IN const char* prop_name,
	IN const char* value)
{
	impl().m_prop_values[prop_name] = value;
}
HtmlObject HtmlObject::AddChild(
	IN const char* class_name,
	IN const PositionMode pos_mode)
{
	// check parameter.
	if (pos_mode != HtmlObject::class_pos &&
		pos_mode != HtmlObject::order_pos)
	{
		throw std::logic_error(
			"html object add child fail, parameter pos is invalid.");
	}
	HtmlClass child_class = impl().m_class.FindChild(class_name);
	if (child_class.null())
	{
		EcoThrow << "find child class fail:" << class_name;
	}
	HtmlObject child;
	child.impl().m_pos_mode = pos_mode;
	child.impl().m_class = child_class;

	// add element to the class pos.
	if (pos_mode == HtmlObject::class_pos)
	{
		if (impl().m_children.empty())
		{
			impl().m_children.push_back(child);
		}
		else
		{
			// find the right insert pos.
			auto it = impl().m_children.begin();
			HtmlClass cur_class = it->GetClass();
			for (; it != impl().m_children.end(); ++it)
			{
				// if goto the next html class pos.
				if (!(it->GetClass() == cur_class) && 
					it->GetPositionMode() == it->class_pos)
				{
					cur_class = it->GetClass();
				}

				// compare child class order by position defined in html.
				if (impl().m_class.Compare(child_class, cur_class) < 0)
				{
					impl().m_children.insert(it, child);
					break;
				}
			}
			if (it == impl().m_children.end())
			{
				impl().m_children.push_back(child);
			}
		}
	}
	// add element to the child end.
	else if (pos_mode == HtmlObject::order_pos)
	{
		impl().m_children.push_back(child);
	}
	return child;
}


//##############################################################################
//##############################################################################
////////////////////////////////////////////////////////////////////////////////
class Html::Impl
{
	ECO_IMPL_INIT(Html);
public:
	std::string m_file_path;
	HtmlClass  m_class;
	HtmlObject m_object;

public:
	void LoadFile(IN const std::string& file_path)
	{
		// read file.
		eco::filesystem::ReadFile reader(file_path.c_str(), "r");
		std::string& html_text = reader.data();

		// parse class from html.
		// root class
		m_class.impl().m_name = "root";
		m_class.impl().m_content.swap(html_text);
		// root child class.
		LoadClass(m_class);

		// init object.
		m_object.impl().m_class = m_class;
		m_file_path = file_path;
	}

	void LoadClass(OUT HtmlClass& parent)
	{
		// find html class flag, and record it.
		HtmlClassFinder finder;
		size_t find_start = 0;
		while (finder.Find(parent.impl().m_content, find_start))
		{
			HtmlClass child = parent.AddChild();
			child.impl().m_position = finder.m_parent_pos;
			child.impl().m_name.swap(finder.m_class_name);
			child.impl().m_content.swap(finder.m_class_content);
			LoadClass(child);
		}
	}

	void Save()
	{
		SaveAs(m_file_path);
	}

	void SaveAs(IN const std::string& file_path)
	{
		// get html complete content.
		std::string text;
		m_object.impl().MakeText(text);

		// write text into file.
		eco::filesystem::File writer(file_path.c_str(), "w");
		writer.write(text.c_str(), strlen(text.c_str()));
	}
};
ECO_SHARED_IMPL(Html);


////////////////////////////////////////////////////////////////////////////////
Html Html::LoadFile(IN const char* file)
{
	Html html;
	html.impl().LoadFile(file);
	return html;
}
HtmlObject Html::Root()
{
	return impl().m_object;
}
void Html::Save()
{
	impl().Save();
}
void Html::SaveAs(IN const char* file)
{
	impl().SaveAs(file);
}


////////////////////////////////////////////////////////////////////////////////
}}// ns