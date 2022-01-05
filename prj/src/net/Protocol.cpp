#include "Pch.h"
#include <eco/net/protocol/ProtocolFamily.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Implement.h>
#include <unordered_map>


ECO_NS_BEGIN(eco);
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class Protocol::Impl
{
	ECO_IMPL_INIT(Protocol);
public:
	uint8_t m_version;
	uint32_t m_max_size;

	inline Impl()
	{
		m_version = 0;
		m_max_size = -1;
	}
};
ECO_OBJECT_IMPL(Protocol);
void Protocol::set_version(uint8_t ver)
{
	impl().m_version = ver;
}
uint8_t Protocol::version() const
{
	return impl().m_version;
}
void Protocol::set_max_size(uint32_t siz)
{
	impl().m_max_size = siz;
}
uint32_t Protocol::max_size() const
{
	return impl().m_max_size;
}


////////////////////////////////////////////////////////////////////////////////
class ProtocolFamily::Impl
{
	ECO_IMPL_INIT(ProtocolFamily);
public:
	inline Impl() : m_protocol_latest(0) {}

	typedef std::auto_ptr<Protocol> ProtocolPtr;
	Protocol* m_protocol_latest;
	std::unordered_map<int, ProtocolPtr> m_protocol_map;
};
ECO_OBJECT_IMPL(ProtocolFamily);
void ProtocolFamily::add_protocol(IN Protocol* heap)
{
	// check protocol family by version size.
	if (!impl().m_protocol_map.empty())
	{
		Protocol& prot = *impl().m_protocol_map.begin()->second;
		if (prot.version_size() != heap->version_size())
		{
			ECO_THROW(e_protocol_error) << typeid(*heap).name()
				<< " can't add to protocol family, "
				<< "because of unequal version size.";
		}
	}

	// set protocol body of latest version.
	if (impl().m_protocol_latest == nullptr ||
		impl().m_protocol_latest->version() <= heap->version())
	{
		impl().m_protocol_latest = heap;
	}
	impl().m_protocol_map[heap->version()] = Impl::ProtocolPtr(heap);
}
Protocol* ProtocolFamily::protocol(int version) const
{
	auto it = impl().m_protocol_map.find(version);
	if (it != impl().m_protocol_map.end())
	{
		return it->second.get();
	}
	return nullptr;
}
Protocol* ProtocolFamily::protocol_latest() const
{
	return impl().m_protocol_latest;
}


////////////////////////////////////////////////////////////////////////////////
}}