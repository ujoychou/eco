#ifndef ECO_XML_IMPL_H
#define ECO_XML_IMPL_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2019-08-19.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2017 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/xml/Reader.h>
#include <eco/Implement.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(xml);
////////////////////////////////////////////////////////////////////////////////
class Reader::Impl
{
	ECO_IMPL_INIT(Reader);
public:
	bool read_node(
		OUT ContextNode& node,
		IN  const std::string& node_name,
		IN  const boost::property_tree::ptree& xml_node_set);
	bool get_node(
		OUT ContextNode& node,
		IN  const std::string& node_name,
		IN  boost::property_tree::ptree::const_iterator& xml_it);
	void read_node(
		OUT Context& vals,
		IN  const std::string& xml_node_name,
		IN  const boost::property_tree::ptree& xml_node);

public:
	inline Impl() : m_root_node(0)
	{}

	inline bool is_root_import(const ContextNode* node) const
	{
		return !m_xml_file.empty() && (node == m_root_node);
	}

	std::string  m_xml_file;
	ContextNode* m_root_node;
	std::vector<std::string> m_imports;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(xml);
ECO_NS_END(eco);
#endif