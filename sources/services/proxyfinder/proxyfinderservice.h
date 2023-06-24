#ifndef __PROXYFINDERSERVICE__H__ 
#define __PROXYFINDERSERVICE__H__ 

#include <string>
#include <utils.h>
#include "proxy.h"
#include <iportaldriver.h>
#include <freeproxylistnet.h>
#include <ThreadPool.h>

using namespace std;

namespace ProxyFinder{

    class ProxyFinderService { 
    private:
        const int threadPoolNumOfThreads = 10;
        vector<Proxy> validProxies;
        ThreadPool *tpool;
        vector<IPortalDriver*> drivers;

        void checkAndAddProxy(Proxy proxy);
        bool proxysVectorCointains(vector<Proxy> &proxiesVector, Proxy proxy);
        bool proxyIsWorking(Proxy proxy);
        void checkProxyStillAlive(int index);
        void startProxyChecker();

        int lastAliveCheckIndex = -1;
        Timer timerCheckProxysAlive;
    public: 
        
        ProxyFinderService();
        ~ProxyFinderService(); 

        size_t size();
        Proxy operator[](size_t pos);
        Proxy pickRandomProxy();



    }; 
}
 
#endif 
