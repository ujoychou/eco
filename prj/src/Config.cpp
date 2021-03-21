#include "PrecHeader.h"
#include <eco/Config.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/thread/Map.h>
#include "./xml/Impl.h"


namespace eco{;
//##############################################################################
//##############################################################################
class Config::Impl
{
public:
	void init(Config&) {}

	// config nodes.
	ContextNode m_root;

	// config import files.
	std::vector<std::string> m_imports;

	// index of config nodes.
	eco::HashMap<std::string, StringAny> m_data;

public:
	////////////////////////////////////////////////////////////////////////////
	inline void init(IN const char* file, IN const uint32_t text_size = 0)
	{
		try
		{
			xml::Reader reader;
			if (text_size == 0)
			{
				reader.read(m_root, file);
			}
			else
			{
				const char* text = file;
				reader.read(m_root, text, text_size);
			}
			m_imports = reader.impl().m_imports;

			// setup index for config key.
			m_root.set_name("");
			setup_index(m_root, "");
		}
		catch (const std::exception& e)
		{
			ECO_THROW("init config fail:") < e.what();
		}
	}


	////////////////////////////////////////////////////////////////////////////
	inline void get_property_set(
		OUT eco::Context& result,
		IN  const char* node_key) const
	{
		std_lock_guard lock(m_data.mutex());
		m_root.get_property_set(result, node_key);
	}


	////////////////////////////////////////////////////////////////////////////
	inline eco::ContextNodeSet get_children(IN  const char* parent_key) const
	{
		std_lock_guard lock(m_data.mutex());
		auto node = get_node_raw(parent_key);
		return !node.null() ? node.get_children() : eco::null;
	}


	////////////////////////////////////////////////////////////////////////////
	inline eco::ContextNode get_node(IN  const char* node_key) const
	{
		std_lock_guard lock(m_data.mutex());
		return get_node_raw(node_key);
	}
	inline eco::ContextNode get_node_raw(IN  const char* node_key) const
	{
		if (m_root.has_children() && node_key != nullptr)
		{
			return m_root.get_child(node_key);
		}
		return m_root;
	}

	////////////////////////////////////////////////////////////////////////////
	inline void setup_index(
		IN const eco::ContextNode& node,
		IN const std::string& parent_name)
	{
		// node name.
		std::string node_name(parent_name);
		eco::xml::sub_path(node_name, node.name());
		if (!node_name.empty())
		{
			m_data.set(node_name, node.value());
		}
	
		// node attribute.
		auto att = node.property_set().begin();
		for (; att != node.property_set().end(); ++att)
		{
			std::string attr_name(node_name);
			eco::xml::sub_path(attr_name, att->name());
			m_data.set(attr_name, att->value());
		}

		// node children
		if (node.has_children())
		{
			auto it = node.children().begin();
			for (; it != node.children().end(); ++it)
			{
				setup_index(*it, node_name);
			}
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(Config);
void Config::init(IN const char* file)
{
	impl().init(file);
}
void Config::init(IN const char* text, IN const uint32_t size)
{
	assert(size != 0);
	impl().init(text, size);
}
bool Config::find(OUT eco::StringAny& v, IN const char* key) const
{
	return impl().m_data.find(v, key);
}
const eco::StringAny Config::at(IN const char* key) const
{
	eco::StringAny v;
	if (!impl().m_data.find(v, key))
	{
		ECO_THROW("config can't find key:") < key;
	}
	return v;
}
const eco::StringAny Config::get(IN const char* key) const
{
	eco::StringAny v;
	impl().m_data.find(v, key);
	return v;
}
void Config::add(IN const char* key, IN const char* value)
{
	StringAny v;
	impl().m_data.set(key, (v = value));
}
uint32_t Config::import_file_size() const
{
	return (uint32_t)impl().m_imports.size();
}
const char* Config::import_file(IN uint32_t index) const
{
	return impl().m_imports[index].c_str();
}


////////////////////////////////////////////////////////////////////////////////
eco::ContextNodeSet Config::find_children(IN const char* parent_key) const
{
	return impl().get_children(parent_key);
}
eco::ContextNodeSet Config::get_children(IN const char* parent_key) const
{
	eco::ContextNodeSet result = impl().get_children(parent_key);
	if (result.null())
	{
		if (parent_key == nullptr) parent_key = "root";
		ECO_THROW("get config node children fail ") < parent_key;
	}
	return result;
}


////////////////////////////////////////////////////////////////////////////////
void Config::get_property_set(
	OUT eco::Context& context_set,
	IN  const char* node_key) const
{
	impl().get_property_set(context_set, node_key);
}


////////////////////////////////////////////////////////////////////////////////
eco::ContextNode Config::get_node(IN const char* node_key) const
{
	eco::ContextNode result = impl().get_node(node_key);
	if (result.null())
	{
		if (node_key == nullptr) node_key = "root";
		ECO_THROW("get config node child fail ") < node_key;
	}
	return result;
}
eco::ContextNode Config::find_node(IN const char* node_key) const
{
	return impl().get_node(node_key);
}
eco::ContextNode Config::find_node(
	IN const uint32_t index, IN const char* parent_key) const
{
	eco::ContextNodeSet set = impl().get_children(parent_key);
	if (!set.null() && set.size() > index)
	{
		return set.at(index);
	}
	return eco::null;
}


////////////////////////////////////////////////////////////////////////////////
}