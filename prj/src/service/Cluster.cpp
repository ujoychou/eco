#include "Pch.h"
#include <eco/service/Cluster.h>
////////////////////////////////////////////////////////////////////////////////


namespace eco { ;
namespace service { ;
namespace ops { ;


////////////////////////////////////////////////////////////////////////////////
Cluster& get_cluster()
{
	return eco::Singleton<Cluster>::get();
}


////////////////////////////////////////////////////////////////////////////////
}}}