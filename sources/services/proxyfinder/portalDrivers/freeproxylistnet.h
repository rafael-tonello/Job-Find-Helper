#ifndef __FREEPROXYLISTNET__H__ 
#define __FREEPROXYLISTNET__H__ 

#include "iportaldriver.h"
#include <timer.h>
namespace ProxyFinder{
    class FreeProxyListNet: public IPortalDriver{ 
    private:
        Timer tmr;
    public: 
        FreeProxyListNet(); 
        ~FreeProxyListNet();
    };
} 
 
#endif 
