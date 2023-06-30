#ifndef __OPENPROXYLISTXYZ__H__ 
#define __OPENPROXYLISTXYZ__H__ 
#include <timer.h>
#include <husymaker.h>
#include "iportaldriver.h"
 
namespace ProxyFinder{
    class OpenProxyListXyz: public IPortalDriver {
    private:
        Timer tmr;

        void downloadAndProcessList(string list, string proxysType);
    public: 
        OpenProxyListXyz(); 
        ~OpenProxyListXyz(); 
    }; 
}
 
#endif 
