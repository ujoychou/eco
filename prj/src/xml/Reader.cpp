#include "PrecHeader.h"
#include <eco/xml/Reader.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <eco/Typex.h>



namespace eco{;
namespace xml{;


////////////////////////////////////////////////////////////////////////////////
class Reader::Impl
{
public:
	void init(Reader& wrap){}
};


////////////////////////////////////////////////////////////////////////////////
/*@ read xml node key value.*/
void read_node(
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
					vals.add().key(xml_node_name.c_str())
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
			para.set_key(temp.c_str());
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
ECO_SHARED_IMPL(Reader);
void Reader::read(OUT eco::Context& vals, IN const char* file)
{
	using namespace boost::property_tree;
	using namespace boost::property_tree::xml_parser;
	boost::property_tree::ptree doc;
	xml_parser::read_xml(file, doc, trim_whitespace | no_comments);
	read_node(vals, "", doc.get_child("root"));
}


////////////////////////////////////////////////////////////////////////////////
bool read_node(
	OUT ContextNode& node,
	IN  const std::string& node_name,
	IN  const boost::property_tree::ptree& xml_node_set);
bool get_node(
	OUT ContextNode& node,
	IN  const std::string& node_name,
	IN  boost::property_tree::ptree::const_iterator& xml_it);


////////////////////////////////////////////////////////////////////////////////
bool get_node(
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
				node.set_name(node_name.c_str());
				node.set_value(itc->second.data().c_str());
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
				param.set_key(itc->first.c_str());
				param.set_value(itc->second.data().c_str());
			}
		}
	}
	// it->first = key. (node name)
	else if (xml_it->second.size() == 0)	// leaf children.
	{
		node.property_set().add().key(xml_it->first.c_str()).
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
				param.set_key(child.get_name());
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
bool read_node(
	OUT ContextNode& node,
	IN  const std::string& node_name,
	IN  const boost::property_tree::ptree& xml_node_data)
{
	node.set_name(node_name.c_str());
	for (auto it = xml_node_data.begin(); it != xml_node_data.end(); ++it)
	{
		if (!get_node(node, node_name, it))
		{
			return false;
		}
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
void Reader::read(OUT eco::ContextNode& node, IN const char* file)
{
	using namespace boost::property_tree;
	using namespace boost::property_tree::xml_parser;
	boost::property_tree::ptree doc;
	xml_parser::read_xml(file, doc, trim_whitespace | no_comments);
	read_node(node, "root", doc.get_child("root"));
}


////////////////////////////////////////////////////////////////////////////////
}}