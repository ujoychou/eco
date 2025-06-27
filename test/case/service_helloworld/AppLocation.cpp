#include <eco/app.hpp>
////////////////////////////////////////////////////////////////////////////////
#include "ServiceIds.hpp"
#include <memory>
#include <map>
#include <list>
#include <string>


////////////////////////////////////////////////////////////////////////////////
class AppLocation : eco::app
{
public:
	inline AppLocation()
	{
	}

	virtual void on_init()
	{
		eco::app::service();
	}

	virtual void on_load();

	virtual void on_exit();

public:

	void test()
	{

	}
	
};


ECO_APP(AppLocation, get_app);
////////////////////////////////////////////////////////////////////////////////
class change
{
public:
	typedef std::shared_ptr<change> ptr;
};

class reader
{
public:
	typedef std::shared_ptr<reader> ptr;
};

class whc_nokey
{
	std::map<uint32_t, reader::ptr> readers_reliable;

	// whc
	std::cycle_buffer<change::ptr> samples;


	void on_reader_ack(uint32_t reader, uint32_t seq)
	{
		// lock
		auto r = get_readers(reader);
		auto r_size = get_readers_size();
		auto range = r->on_ack(seq);
		if (range.empty()) { return; }
		
		uint32_t ack_seq = 0;
		auto it = get_samples(range.min);
		for (uint32_t i = range.min; i < range.max; i++)
		{
			if (s->on_ack() == r_size)
			{
				ack_seq = i;
			}
		}
	}

	void on_reader_remove(uint32_t reader)
	{
		auto r = get_reader(reader);
		auto seq = r->max_seq();
		// reduce ack count in samples.
		auto it = samples.begin();
		for (; it != samples.end() && it->seq <= seq; ++it)
		{
			it->ack_down();
		}
	}
};


struct sample
{
	uint32_t seq;
	sample*  next;
	sample*  next_inst;
	std::string key;
};


struct instance
{
	uint32_t size;
	sample[] samples;
};


class whc_withkey
{
	uint32_t size_samples;
	uint32_t size_instances;
	
	std::map<uint32_t, instance*> inst_maps;
	std::cycle_buffer<sample> samples;

	void add_sample(uint32_t seq, void* data, std::string& key)
	{
		if (size_samples == max)
		{
			return ;
		}
	}

	void add_sample(sample* sample)
	{
		instance* inst = get_instance(sample.key);
		inst->samples->next_inst = sample;
		samples.push_back(sample); 
		
	}
};

class whc_keep_last_1
{
	uint32_t size_samples;
	uint32_t size_instances;
	
	std::map<uint32_t, instance*> inst_maps;
	std::cycle_buffer<sample> samples;

	void add_sample(uint32_t seq, void* data, std::string& key)
	{
		if (size_samples == max)
		{
			return ;
		}
	}

	void add_sample(sample* sample)
	{
		instance* inst = get_instance(sample.key);
		inst->samples->next_inst = sample;
		samples.push_back(sample); 
		
	}
};
