#ifndef __IPORTALDRIVER__H__ 
#define __IPORTALDRIVER__H__ 

#include <stream.h>
#include "../proxy.h"

namespace ProxyFinder{
    class IPortalDriver{ 
    public: 
        Common::Stream<Proxy> discoveredProxies;
    }; 
}
 
#endif 
