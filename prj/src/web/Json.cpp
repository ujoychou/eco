#include "PrecHeader.h"
#include <eco/web/Json.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/Typex.ipp>
#include <eco/filesystem/Operations.h>
#include <boost/algorithm/string/replace.hpp>
#include "JsonClassFinder.ipp"


namespace eco{;
namespace web{;
//##############################################################################
//##############################################################################
class JsonClass::Impl
{
	ECO_IMPL_INIT(JsonClass);
public:
	// json class identity.
	std::string m_name;

	// json class text detail.
	std::string m_content;

	// json parent position.
	size_t m_parent_pos;

	// json children.
	std::list<JsonClass> m_children;

	Impl() : m_parent_pos(0)
	{}
};
ECO_SHARED_IMPL(JsonClass);
////////////////////////////////////////////////////////////////////////////////
const char* JsonClass::GetName() const
{
	return impl().m_name.c_str();
}
const char* JsonClass::GetContent() const
{
	return impl().m_content.c_str();
}
size_t JsonClass::GetParentPosition() const
{
	return impl().m_parent_pos;
}
JsonClass JsonClass::AddChild()
{
	JsonClass new_child;
	impl().m_children.push_back(new_child);
	return new_child;
}
JsonClass JsonClass::FindChild(IN const char* class_name)
{
	auto it = impl().m_children.begin();
	for (; it!=impl().m_children.end(); ++it)
	{
		if (it->impl().m_name == class_name)
		{
			return *it;
		}
	}
	return eco::null;
}


//##############################################################################
//##############################################################################
////////////////////////////////////////////////////////////////////////////////
class JsonObject::Impl
{
	ECO_IMPL_INIT(JsonObject);
public:
	void MakeText(OUT std::string& text) const
	{
		std::string cur_text = (m_class.GetContent());

		// make child objects text.
		size_t last_pos = 0;
		size_t child_pos = 0;
		std::string child_text;
		auto itc = m_children.begin();
		for ( ; itc!=m_children.end(); ++itc)
		{
			JsonClass child_class = itc->GetClass();
			if (last_pos == child_class.GetParentPosition())
			{
				itc->impl().MakeText(child_text);
				child_text = "," + child_text;
				cur_text.insert(child_pos, child_text);
				child_pos += child_text.size();
			}
			else
			{
				child_pos += child_class.GetParentPosition() - last_pos;
				last_pos  = child_class.GetParentPosition();
				itc->impl().MakeText(child_text);
				cur_text.insert(child_pos, child_text);
				child_pos += child_text.size();
			}
		}// end for.

		// replace property value.
		// "property1": "value1",
		// "property2": "value2",
		// "property3": "value3"
		std::string key;
		auto itv = m_prop_values.begin();
		for (; itv!=m_prop_values.end(); ++itv)
		{
			// "property1"
			key  = '"';
			key += itv->first; 
			key += '"';
			size_t key_pos = cur_text.find(key);
			if (key_pos == std::string::npos)
			{
				throw std::logic_error("json property can't be find1.");
			}
			// : "value1"
			// get the second '"'.
			size_t first_pos = cur_text.find('"', key_pos + key.size());
			if (first_pos == std::string::npos)
			{
				throw std::logic_error("json property can't be find2.");
			}
			size_t second_pos = cur_text.find('"', first_pos + 1);
			if (second_pos == std::string::npos)
			{
				throw std::logic_error("json property can't be find3.");
			}
			size_t len = second_pos - first_pos + 1;
			cur_text.replace(first_pos, len, itv->second);
		}

		// replace object name.
		auto itn = m_obj_names.begin();
		for (; itn!=m_obj_names.end(); ++itn)
		{
			boost::algorithm::replace_all(cur_text, itn->first, itn->second);
		}

		text.swap(cur_text);
	}

public:
	JsonClass m_class;
	std::map<std::string, std::string> m_obj_names;
	std::map<std::string, std::string> m_prop_values;
	std::list<JsonObject> m_children;
};
ECO_SHARED_IMPL(JsonObject);
////////////////////////////////////////////////////////////////////////////////
void JsonObject::MakeText(OUT eco::StringAny& text) const
{
	impl().MakeText(text.impl().m_value);
}
JsonClass JsonObject::GetClass() const
{
	return impl().m_class;
}
void JsonObject::SetObjectName(
	IN const char* name,
	IN const char* value)
{
	impl().m_obj_names[name] = value;
}
void JsonObject::SetValue(
	IN const char* prop_name,
	IN const char* value)
{
	std::string v("\"");
	v += value;
	v += "\"";
	impl().m_prop_values[prop_name] = v;
}
void JsonObject::SetValue(
	IN const char* prop_name,
	IN const double value)
{
	std::string v = eco::cast<std::string>(value);
	impl().m_prop_values[prop_name] = v;
}
void JsonObject::SetValue(
	IN const char* prop_name,
	IN const int value)
{
	std::string v = eco::cast<std::string>(value);
	impl().m_prop_values[prop_name] = v;
}
JsonObject JsonObject::AddChild(
	IN const char* class_name)
{
	JsonClass child_class = impl().m_class.FindChild(class_name);
	if (child_class.null())
	{
		EcoThrow << "find child class fail:" << class_name;
	}
	JsonObject child;
	child.impl().m_class = child_class;
	impl().m_children.push_back(child);
	return child;
}


//##############################################################################
//##############################################################################
////////////////////////////////////////////////////////////////////////////////
class Json::Impl
{
	ECO_IMPL_INIT(Json);
public:
	std::string m_file_path;
	JsonClass  m_class;
	JsonObject m_object;

public:
	void LoadFile(IN const char* file_path)
	{
		// read file.
		eco::filesystem::ReadFile reader(file_path, "r");
		std::string& json_text = reader.data();

		// parse class from json.
		// root class
		m_class.impl().m_name = "root";
		m_class.impl().m_content.swap(json_text);
		// root child class.
		LoadClass(m_class);

		// init object.
		m_object.impl().m_class = m_class;
		m_file_path = file_path;
	}

	void LoadClass(OUT JsonClass& parent)
	{
		// find json class flag, and record it.
		// 1.single object, just replace value.
		// 2.array object, just using class.
		JsonClassFinder finder;
		size_t find_start = 0;
		while (finder.Find(parent.impl().m_content, find_start))
		{
			JsonClass child = parent.AddChild();
			child.impl().m_parent_pos = finder.m_parent_pos;
			child.impl().m_name.swap(finder.m_class_name);
			child.impl().m_content.swap(finder.m_class_content);
			LoadClass(child);
		}
	}

	void Save()
	{
		SaveAs(m_file_path.c_str());
	}

	void SaveAs(IN const char* file_path)
	{
		// get json complete content.
		std::string text;
		m_object.impl().MakeText(text);

		// write text into file.
		eco::filesystem::File writer(file_path, "w");
		writer.write(text.c_str(), text.size());
	}
};
ECO_SHARED_IMPL(Json);
////////////////////////////////////////////////////////////////////////////////
Json Json::LoadFile(IN const char* file)
{
	Json json;
	json.impl().LoadFile(file);
	return json;
}
JsonObject Json::Root()
{
	return impl().m_object;
}
void Json::Save()
{
	impl().Save();
}
void Json::SaveAs(IN const char* file)
{
	impl().SaveAs(file);
}


////////////////////////////////////////////////////////////////////////////////
}}// ns