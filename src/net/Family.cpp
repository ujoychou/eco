#include "Pch.h"
#include <eco/net/Family.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Error.h>
#include <eco/rx/RxImpl.h>
#include <unordered_map>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class Family::Impl
{
	ECO_IMPL_INIT(Family);
public:
	typedef std::shared_ptr<Protocol> ProtocolPtr;
	typedef std::shared_ptr<ProtocolTcp> ProtocolTcpPtr;
	inline Impl() : protocol_latest(0) {}

	// tcp protocol
	ProtocolTcpPtr protocol_tcp;

	// protocol set of the tcp protocol
	Protocol* protocol_latest;
	std::unordered_map<int, ProtocolPtr> protocol_map;
};
ECO_OBJECT_IMPL(Family);


////////////////////////////////////////////////////////////////////////////////
void Family::protocol_tcp(ProtocolTcp* heap)
{
	impl->protocol_tcp.reset(heap);
}
ProtocolTcp* Family::protocol_tcp()
{
	return impl->protocol_tcp.get();
}


////////////////////////////////////////////////////////////////////////////////
void Family::protocol_add(Protocol* heap)
{
	// set protocol body of latest version.
	if (impl->protocol_latest == nullptr ||
		impl->protocol_latest->version <= heap->version)
	{
		impl->protocol_latest = heap;
	}
	impl->protocol_map[heap->version] = Impl::ProtocolPtr(heap);
}
Protocol* Family::protocol(int version)
{
	auto it = impl->protocol_map.find(version);
	if (it != impl->protocol_map.end())
	{
		return it->second.get();
	}
	return nullptr;
}
Protocol* Family::protocol_latest()
{
	return impl->protocol_latest;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);