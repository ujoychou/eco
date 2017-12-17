#include "PrecHeader.h"
#include "Test.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/test/Timing.h>
#include "App.h"


namespace eco{;
namespace topic{;
namespace test{;


class TestA
{
public:
	int func_a(int i) const
	{
		return i + 1;
	}
};
void Test()
{
	eco::test::Timing timer;
	// 1.bind new.
	std::shared_ptr<TestA> ta(new TestA);
	// 2.bind func & shared_ptr.copy
	std::function<int(int)> func = std::bind(
		&TestA::func_a, ta, std::placeholders::_1);
	timer.start();
	int c = 0;
	for (auto i = 0; i < 100000000; ++i)
	{
		// 3.bind call.
		c += func(2);
	}
	timer.timeup();
	EcoInfo << timer.milliseconds();
}


////////////////////////////////////////////////////////////////////////////////
void Subscriber::on_publish(
	IN const uint64_t topic_id,
	IN eco::Content& content)
{
	if (content.get_type() == Document::type())
	{
		Document* objp = content.cast_object<Document>();
		Document  obj = *(Document*)content.get_value();
	}
}
void Subscriber::on_erase_topic(IN const uint64_t topic_id)
{
	auto id = topic_id;
	id = 0;
}


////////////////////////////////////////////////////////////////////////////////
void Subscriber::on_publish(
	IN const std::string& topic_id,
	IN eco::Content& content)
{
	if (content.get_type() == Part::type())
	{
		Part* objp = content.cast_object<Part>();
		auto  obj = *(std::shared_ptr<Part>*)content.get_value();
	}
}
void Subscriber::on_erase_topic(IN const std::string& topic_id)
{
	auto id = topic_id;
	id.clear();
}


////////////////////////////////////////////////////////////////////////////////
void Subscriber::on_publish(
	IN const eco::TopicId& topic_id,
	IN eco::Content& content)
{
	if (topic_id.m_object_type == type_doc)
	{
		if (topic_id.m_aspect_id == doc_entry)
		{
			Document* objp = content.cast_object<Document>();
			Document  obj = *(Document*)content.get_value();
		}
	}
	else if (topic_id.m_object_type == type_part)
	{
		if (topic_id.m_aspect_id == part_entry)
		{
			Part* objp = content.cast_object<Part>();
			auto  obj = *(std::shared_ptr<Part>*)content.get_value();
		}
	}
}
void Subscriber::on_erase_topic(IN const eco::TopicId& topic_id)
{
	int flag = 0;
	if (topic_id.m_object_type == type_doc)
	{
		flag = 1;
	}
	else if (topic_id.m_object_type == type_part)
	{
		flag = 2;
	}
}
void Subscriber::on_clear_content(IN const eco::TopicId& topic_id)
{
	int flag = 0;
	if (topic_id.m_object_type == type_doc)
	{
		flag = 1;
	}
	else if (topic_id.m_object_type == type_part)
	{
		flag = 2;
	}
}


////////////////////////////////////////////////////////////////////////////////
}}}