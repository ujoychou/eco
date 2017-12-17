#ifndef ECO_CMD_TEST_H
#define ECO_CMD_TEST_H
/*******************************************************************************
@ 名称

@ 功能

@ 异常

@ 备注

--------------------------------------------------------------------------------
@ 历史记录 @
@ ujoy modifyed on 2016-10-29

--------------------------------------------------------------------------------
* 版权所有(c) 2015 - 2017, ujoychou, 保留所有权利。

*******************************************************************************/
#include <eco/thread/topic/Topic.h>


namespace eco{;
namespace topic{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
enum Type
{
	type_doc,
	type_part,
};
enum Aspect
{
	doc_entry,
	part_entry,
};


////////////////////////////////////////////////////////////////////////////////
class Part
{
	ECO_TYPE(Part);
public:
	inline const std::string& get_id() const
	{
		return m_id;
	}

	std::string m_id;
	std::string m_name;
	std::string m_spec;
	std::string m_kind;
	std::string m_source;
};
typedef eco::OneTopic<std::string> PartTopic;
typedef eco::SeqTopic<std::string> PartSeqTopic;
typedef eco::SetTopic<Part, std::string> PartSetTopic;


////////////////////////////////////////////////////////////////////////////////
class Document
{
	ECO_TYPE(Document);
public:
	inline uint64_t get_id() const
	{
		return m_id;
	}

	uint64_t m_id;
	std::string m_name;
	std::string m_spec;
	std::string m_kind;
	std::string m_text;
};
typedef eco::OneTopic<uint64_t> DocTopic;
typedef eco::SeqTopic<uint64_t> DocSeqTopic;
typedef eco::SetTopic<Document, uint64_t> DocSetTopic;
typedef eco::OneTopic<eco::TopicId> DocumentTopic;
typedef eco::SeqTopic<eco::TopicId> DocumentSeqTopic;
typedef eco::SetTopic<Document, eco::TopicId> DocumentSetTopic;


////////////////////////////////////////////////////////////////////////////////
class Subscriber : public eco::Subscriber
{
public:
	virtual void on_publish(
		IN const uint64_t topic_id,
		IN eco::Content& content) override;
	virtual void on_erase_topic(
		IN const uint64_t topic_id) override;

	virtual void on_publish(
		IN const std::string& topic_id,
		IN eco::Content& content) override;
	virtual void on_erase_topic(
		IN const std::string& topic_id) override;

	virtual void on_publish(
		IN const eco::TopicId& topic_id,
		IN eco::Content& content) override;
	virtual void on_erase_topic(
		IN const eco::TopicId& topic_id) override;
	virtual void on_clear_content(
		IN const eco::TopicId& topic_id) override;
};

////////////////////////////////////////////////////////////////////////////////
}}}
#endif