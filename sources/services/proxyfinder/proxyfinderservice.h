#ifndef __PROXYFINDERSERVICE__H__ 
#define __PROXYFINDERSERVICE__H__ 

#include <string>
#include <utils.h>
#include "proxy.h"
#include "portalDrivers/iportaldriver.h"
#include "portalDrivers/freeproxylistnet.h"
#include "portalDrivers/openproxylistxyz.h"
#include "portalDrivers/geonodecom.h"
#include "iproxyfinderservice.h"
#include <ThreadPool.h>

using namespace std;

namespace ProxyFinder{

    class ProxyFinderService: public IProxyFinderService { 
    private:
        const int threadPoolNumOfThreads = 50;
        vector<Proxy> validProxies;
        ThreadPool *tpool;
        vector<IPortalDriver*> drivers;

        void checkAndAddProxy(Proxy proxy);
        bool proxysVectorCointains(vector<Proxy> &proxiesVector, Proxy proxy);
        bool proxyIsWorking(Proxy proxy, int tries = 3);
        void checkProxyStillAlive(int index);
        void startProxyChecker();

        int lastAliveCheckIndex = -1;
        Timer timerCheckProxysAlive;


        struct Statistics{
            uint foundProxies;
            uint checkedProxies;
            uint validProxies;
            uint invalidProxies;
        }; Statistics statistics = {0, 0, 0, 0};
    public: 
        
        ProxyFinderService();
        ~ProxyFinderService(); 

        
        size_t size() override;
        Proxy operator[](size_t pos) override;
        future<ProxyFinder::Proxy> pickRandomProxy() override;


    }; 
}
 
#endif 
