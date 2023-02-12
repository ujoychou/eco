#include "Pch.h"
#include "Impl.h"
////////////////////////////////////////////////////////////////////////////////
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>


ECO_NS_BEGIN(eco);
namespace xml{;
////////////////////////////////////////////////////////////////////////////////
/*@ read xml node key value.*/
void Reader::Impl::read_node(
	OUT Context& vals,
	IN  const std::string& xml_node_name,
	IN  const boost::property_tree::ptree& xml_node)
{
	std::string temp;
	// traverse child
	for (auto it=xml_node.begin(); it!=xml_node.end(); ++it)
	{
		if (it->first == "<xmlattr>")		// <key value="xxx"/>
		{
			for (auto itc=it->second.begin(); itc!=itc->second.end(); ++itc)
			{
				if (itc->first == "value")
				{
					vals.add().name(xml_node_name.c_str())
						.value(itc->second.data().c_str());
					break;
				}
			}
		}
		else if (it->second.size() == 0)	// leaf node.
		{
			temp = xml_node_name;
			sub_path(temp, it->first.c_str());
			auto& para = vals.add();
			para.set_name(temp.c_str());
			para.set_value(it->second.data().c_str());
		}
		else								// parent node.
		{
			temp = xml_node_name;
			sub_path(temp, it->first.c_str());
			read_node(vals, temp, it->second);
		}
	}// end for.
}


////////////////////////////////////////////////////////////////////////////////
bool Reader::Impl::get_node(
	OUT ContextNode& node,
	IN  const std::string& node_name,
	IN  boost::property_tree::ptree::const_iterator& xml_it)
{
	if (xml_it->first == "<xmlattr>")		// attribute children
	{
		auto itc = xml_it->second.begin();
		for (; itc != xml_it->second.end(); ++itc)
		{
			// <key value="xxx"/>
			if (itc->first == "value")
			{
				if (eco::empty(node.get_name()))
					node.set_name(node_name.c_str());
				node.set_value(itc->second.data().c_str());
			}
			// <key node="xxx"/>
			else if (itc->first == "node")
			{
				node.set_name(itc->second.data().c_str());
			}
			// <key enable="true"/>
			else if (itc->first == "enable")
			{
				if (!StringAny(itc->second.data().c_str()))
				{
					return false;
				}
			}
			else if (itc->first != "note")
			{
				auto& param = node.property_set().add();
				param.set_name(itc->first.c_str());
				param.set_value(itc->second.data().c_str());
			}
		}
	}
	// it->first = key. (node name)
	else if (xml_it->second.size() == 0)	// leaf children.
	{
		node.property_set().add().name(xml_it->first.c_str()).
			value(xml_it->second.data().c_str());
	}
	else									// node children.
	{
		ContextNode child;
		if (read_node(child, xml_it->first, xml_it->second))
		{
			// child node only contain a value node: "<key value="xxx"/>"
			// change this value node as a parent's property.
			if (!child.has_children() && child.get_property_set().empty())
			{
				auto& param = node.property_set().add();
				param.set_name(child.get_name());
				param.set_value(child.get_value());
			}
			else
			{
				node.children().add(child);
			}
		}
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
/*@ read xml node key value.*/
bool Reader::Impl::read_node(
	OUT ContextNode& node,
	IN  const std::string& node_name,
	IN  const boost::property_tree::ptree& xml_node_data)
{
	node.set_name(node_name.c_str());
	for (auto it = xml_node_data.begin(); it != xml_node_data.end(); ++it)
	{
		// import node.
		if (is_root_import(&node) && it->first == "import")
		{
			m_imports.push_back(it->second.data().c_str());
		}
		else if (!get_node(node, node_name, it))
		{
			return false;
		}
	}

	// import xml.
	if (is_root_import(&node) && !m_imports.empty())
	{
		boost::filesystem::path xmlf(m_xml_file);
		std::string dir = xmlf.parent_path().string() + '\\';
		for (auto it = m_imports.begin(); it != m_imports.end(); )
		{
			if (it->empty())
			{
				it = m_imports.erase(it);
				continue;
			}
			
			Reader reader;
			ContextNode node_import;
			*it = dir + *it;
			if (boost::filesystem::exists(*it))
			{
				reader.read(node_import, it->c_str());
				node.merge(node_import);
			}
			++it;
		}
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(Reader);
void Reader::read(OUT eco::Context& vals, IN const char* file)
{
	using namespace boost::property_tree;
	using namespace boost::property_tree::xml_parser;
	boost::property_tree::ptree doc;
	impl().m_xml_file = file;
	impl().m_root_node = nullptr;
	xml_parser::read_xml(impl().m_xml_file, doc, trim_whitespace | no_comments);
	impl().read_node(vals, "", doc.get_child("root"));
}


////////////////////////////////////////////////////////////////////////////////
void Reader::read(OUT eco::ContextNode& node, IN const char* file)
{
	using namespace boost::property_tree;
	using namespace boost::property_tree::xml_parser;
	boost::property_tree::ptree doc;
	impl().m_xml_file = file;
	impl().m_root_node = &node;
	xml_parser::read_xml(impl().m_xml_file, doc, trim_whitespace | no_comments);
	impl().read_node(node, "root", doc.get_child("root"));
}


////////////////////////////////////////////////////////////////////////////////
void Reader::read(
	OUT eco::ContextNode& node,
	IN const char* xml_text,
	IN const uint32_t xml_size)
{
	using namespace boost::property_tree;
	using namespace boost::property_tree::xml_parser;
	boost::property_tree::ptree doc;
	std::stringstream in;
	in.write(xml_text, xml_size);
	impl().m_root_node = &node;
	xml_parser::read_xml(in, doc, trim_whitespace | no_comments);
	impl().read_node(node, "root", doc.get_child("root"));
}


////////////////////////////////////////////////////////////////////////////////
}}