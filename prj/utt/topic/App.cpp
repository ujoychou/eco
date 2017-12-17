#include "PrecHeader.h"
#include "App.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include "Test.h"


namespace eco{;
namespace topic{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
App::App()
{}


////////////////////////////////////////////////////////////////////////////////
void App::on_init()
{
	m_topic_server.start();

	// ------------------------------------------------------------------------
	// 创建主题
	eco::TopicId tid_doc_set(type_doc, doc_entry);
	m_topic_server.create_topic(tid_doc_set, DocSetTopic::make);
	// 订阅主题
	uint64_t doc_id = 1;
	m_topic_server.subscribe(&m_suber, doc_id, DocTopic::make);
	m_topic_server.subscribe(&m_suber, tid_doc_set);
	// 发布主题:1
	Document doc1;
	doc1.m_id = 1;
	doc1.m_name = "doc1";
	doc1.m_kind = "kind";
	m_topic_server.publish(doc1.m_id, doc1);
	m_topic_server.publish(tid_doc_set, doc1, DocSetTopic::make);
	// 发布主题:002
	Document doc2;
	doc1.m_id = 2;
	doc1.m_name = "doc2";
	m_topic_server.publish(doc1.m_id, doc1);
	m_topic_server.publish(tid_doc_set, doc1, DocSetTopic::make);

	// ------------------------------------------------------------------------
	// 创建主题
	eco::TopicId tid_part_set(type_part, part_entry);
	m_topic_server.create_topic(tid_part_set, PartSetTopic::make);
	// 订阅主题
	std::string part_id = "001";
	m_topic_server.subscribe(&m_suber, part_id, PartTopic::make);
	m_topic_server.subscribe(&m_suber, tid_part_set);
	// 发布主题: 001
	std::shared_ptr<Part> p1(new Part);
	p1->m_id = "001";
	p1->m_name = "cool";
	m_topic_server.publish(p1->m_id, p1);
	m_topic_server.publish(tid_part_set, p1, PartSetTopic::make);
	// 发布主题: 002
	std::shared_ptr<Part> p2(new Part);
	p2->m_id = "002";
	p2->m_name = "well";
	m_topic_server.publish(p2->m_id, p2);
	m_topic_server.publish(tid_part_set, p2, PartSetTopic::make);
	// 查找对象
	if (m_topic_server.find_content<PartSetTopic>(p1, tid_part_set, p2->m_id))
	{
		p1.reset();
	}

	// 删除主题
	m_topic_server.clear_content(tid_doc_set);
	m_topic_server.erase_topic(tid_part_set);
	m_topic_server.clear_topic();
}


////////////////////////////////////////////////////////////////////////////////
void App::on_exit()
{
	m_topic_server.stop();
}


////////////////////////////////////////////////////////////////////////////////
}}}