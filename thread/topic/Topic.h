#ifndef ECO_TOPIC_H
#define ECO_TOPIC_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/thread/topic/Role.h>
#include <map>
#include <deque>
#include <unordered_map>


ECO_NS_BEGIN(eco);
//##############################################################################
//##############################################################################
class OneTopic : public Topic
{
	ECO_TOPIC(OneTopic, Topic);
protected:
	virtual void do_snap(IN eco::Subscription& node) override
	{
		if (m_snap != nullptr)
		{
			auto* suber = (Subscriber*)node.m_subscriber;
			Snap type = snap_head | snap_last;
			m_snap->stamp() = eco::meta::stamp_insert;
			suber->on_publish(*this, Content(m_snap, nullptr, type));
		}
	}

	virtual bool do_move(
		OUT eco::ContentData::ptr& newc,
		OUT eco::ContentData::ptr& old) override
	{
		old = m_snap;
		m_snap = newc;		// update snap with new data.
		return true;
	}

	virtual void do_clear() override
	{
		m_snap.reset();
	}

	inline ContentData::ptr snap() const
	{
		std_lock_guard lock(mutex());
		return m_snap;
	}

	// latest data.
	eco::ContentData::ptr m_snap;
};


//##############################################################################
//##############################################################################
class SeqTopic : public Topic
{
	ECO_TOPIC(SeqTopic, Topic);
public:
	/* exp"market kline", last content can be updated as follow:
	1) get seq topic last content, check it's id equal with new item?
	2) if they are equal, merge new item into last content.
	3) publish last content.
	*/
	inline ContentData::ptr back() const
	{
		std_lock_guard lock(mutex());
		return !m_snap_set.empty() ? m_snap_set.back() : nullptr;
	}
	inline ContentData::ptr front() const
	{
		std_lock_guard lock(mutex());
		return !m_snap_set.empty() ? m_snap_set.front() : nullptr;
	}

	/*@ pop back item from deque.
	*/
	inline ContentData::ptr pop_back()
	{
		std_lock_guard lock(mutex());
		if (!m_snap_set.empty())
		{
			auto c = m_snap_set.back(); 
			m_snap_set.pop_back();
			return std::move(c);
		}
		return nullptr;
	}

	/*@ pop front item from deque.
	*/
	inline ContentData::ptr pop_front()
	{
		std_lock_guard lock(mutex());
		if (!m_snap_set.empty())
		{
			auto c = m_snap_set.front();
			m_snap_set.pop_front();
			return std::move(c);
		}
		return nullptr;
	}

	/*@ erase front item from deque.
	*/
	inline void pop_front(uint32_t left)
	{
		std_lock_guard lock(mutex());
		if (m_snap_set.size() > left)
		{
			auto it_erase_end = m_snap_set.end() - left;
			m_snap_set.erase(m_snap_set.begin(), it_erase_end);
		}
	}

	inline const uint32_t size() const
	{
		std_lock_guard lock(mutex());
		return (uint32_t)m_snap_set.size();
	}
	inline const std::deque<eco::ContentData::ptr>& snap_set_raw() const
	{
		return m_snap_set;
	}
	template<typename object_t>
	inline object_t at(IN uint32_t i)
	{
		std_lock_guard lock(mutex());
		return m_snap_set[i]->cast<object_t>();
	}

	// add "object set/object/shared_object" to topic by "load".
	template<typename object_set_t>
	inline void push_front_set(IN const object_set_t& set)
	{
		std_lock_guard lock(mutex());
		for (auto it = set.rbegin(); it != set.rend(); ++it)
		{
			push_front_raw(*it);
		}
	}
	template<typename object_t>
	inline void push_front(IN const object_t& obj)
	{
		std_lock_guard lock(mutex());
		push_front_raw(obj);
	}
	template<typename object_t>
	inline void push_front(IN const std::shared_ptr<object_t>& obj)
	{
		std_lock_guard lock(mutex());
		push_front_raw(obj);
	}
	template<typename object_t>
	inline void push_front_raw(IN const object_t& obj)
	{
		ContentData::ptr newc(new ContentDataT<
			object_t, object_t>(obj, eco::meta::stamp_insert));
		m_snap_set.push_front(newc);
	}
	template<typename object_t>
	inline void push_front_raw(IN const std::shared_ptr<object_t>& obj)
	{
		typedef std::shared_ptr<object_t> value_t;
		ContentData::ptr newc(new ContentDataT<
			object_t, value_t>(obj, eco::meta::stamp_insert));
		m_snap_set.push_front(newc);
	}

protected:
	inline void on_publish(
		eco::Subscriber* suber, size_t seq, eco::Snap snap)
	{
		auto& cur = m_snap_set[seq];
		cur->stamp() = eco::meta::stamp_insert;
		eco::ContentData* old = seq > 0 ? m_snap_set[seq - 1].get() : nullptr;
		suber->on_publish(*this, Content(cur, old, snap));
	}

	// get snap seq size to publish snap.
	virtual size_t get_snap_seq(IN void* data)
	{
		return 0;	// publish all snap.
	}

	virtual void do_snap(IN eco::Subscription& node) override
	{
		if (m_snap_set.empty()) return;

		// get start seq.
		size_t seq = !node.data() ? 0 : get_snap_seq(node.data());
		if (seq >= m_snap_set.size())
		{
			seq = 0;
			ECO_ERROR << "seq topic hasn't enough content to publish snap "
				<< "seq >= size()" << seq <= m_snap_set.size();
		}

		// *node subscriber may be destruct in "on_publish";
		// publish the snap, and set a content type.
		size_t siz = m_snap_set.size() - 1;
		auto* suber = (eco::Subscriber*)node.m_subscriber;
		for (size_t i = seq; i <= siz && node.m_working; ++i)
		{
			Snap v = (i == seq) ? eco::snap_head : 0;
			if (i == siz) v |= eco::snap_last;
			on_publish(suber, i, v);
		}
	}

	virtual bool do_move(
		OUT eco::ContentData::ptr& newc,
		OUT eco::ContentData::ptr& old) override
	{
		// update last content.
		if (eco::meta::is_update(newc->get_stamp()) && m_snap_set.size() > 0)
		{
			old = m_snap_set.back();
			m_snap_set.back() = newc;
			return true;
		}
		m_snap_set.push_back(newc);
		return true;
	}

	virtual void do_clear() override
	{
		m_snap_set.clear();
	}

	// history data.
	std::deque<eco::ContentData::ptr> m_snap_set;
};


//##############################################################################
//##############################################################################
struct GetId
{
	template<typename object_id_t, typename object_t>
	inline void get_id(object_id_t& id, const object_t& obj)
	{
		id = obj.id();
	}

#ifndef ECO_NO_PROTOBUF
	inline void get_id(uint64_t& id, const eco::proto::Property& obj)
	{
		id = obj.object();
	}
#endif
};

template<typename object_id_t, typename object_t, typename id_adapter_t = GetId,
	typename object_map = std::unordered_map<object_id_t, ContentData::ptr>>
class SetTopic : public Topic
{
	ECO_TOPIC(SetTopic, Topic);
public:
	typedef object_t object;
	typedef object_id_t object_id;
	typedef std::shared_ptr<object_t> object_ptr;

	// find object by identity.
	template<typename value_t>
	inline bool find(OUT value_t& v, IN  const object_id_t& id) const
	{
		ContentData::ptr c = find(id);
		if (c != nullptr) v = *(value_t*)c->get_value();
		return c != nullptr;
	}

	// find content by identity.
	inline ContentData::ptr find(IN const object_id_t& id) const
	{
		std_lock_guard lock(mutex());
		auto it = m_snap_map.find(id);
		return (it != m_snap_map.end()) ? it->second : nullptr;
	}

	// get objects map.
	inline const object_map& get_object_map() const
	{
		return m_snap_map;
	}

	// get objects map size.
	inline size_t size() const
	{
		std_lock_guard lock(mutex());
		return m_snap_map.size();
	}

protected:
	virtual void do_snap(IN eco::Subscription& node) override
	{
		if (m_snap_map.empty()) return;

		// *node subscriber may be destruct in "on_publish";
		// publish the snap, and set a content type.
		uint32_t index = 0;
		auto* suber = (Subscriber*)node.m_subscriber;
		Snap snap = snap_head;
		auto it = m_snap_map.begin();
		for (; it != m_snap_map.end() && node.m_working; ++it, ++index)
		{
			if (index == 1)
				eco::del(snap, snap_head);
			if (index == m_snap_map.size() - 1)
				eco::add(snap, snap_last);
			it->second->stamp() = eco::meta::stamp_insert;
			suber->on_publish(*this, Content(it->second, nullptr, snap));
		}
	}

	virtual bool do_move(
		OUT eco::ContentData::ptr& newc,
		OUT eco::ContentData::ptr& old) override
	{
		object_id_t id;
		if (newc->get_id())
			id = *(object_id_t*)newc->get_id();
		else
			id_adapter_t().get_id(id, *(object_t*)newc->get_object());

		// remove, insert, update object.
		if (eco::meta::is_remove(newc->get_stamp()))
		{
			auto it = m_snap_map.find(id);
			if (it != m_snap_map.end())
			{
				newc = old = it->second;
				newc->stamp() = meta::stamp_delete;
				m_snap_map.erase(it);
			}
		}
		else
		{
			auto& oldc = m_snap_map[id];
			newc->stamp() = oldc ? meta::stamp_update : meta::stamp_insert;
			old = oldc;	oldc = newc;
		}
		return true;
	}

	virtual void do_clear() override
	{
		m_snap_map.clear();
	}

	object_map m_snap_map;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif