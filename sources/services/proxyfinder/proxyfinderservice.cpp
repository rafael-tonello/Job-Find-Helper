#include  "proxyfinderservice.h" 
 
ProxyFinder::ProxyFinderService::ProxyFinderService() 
{
    srand((unsigned)time(0)); 

    this->tpool = new ThreadPool(true, threadPoolNumOfThreads);

    IPortalDriver * drv1 = new FreeProxyListNet();
    drivers.push_back(drv1);
    drv1->discoveredProxies.subscribe([&](Proxy proxy){ this->checkAndAddProxy(proxy); });

    IPortalDriver * drv2 = new OpenProxyListXyz();
    drivers.push_back(drv2);
    drv2->discoveredProxies.subscribe([&](Proxy proxy){ this->checkAndAddProxy(proxy); });

    IPortalDriver * drv3 = new GeoNodeCom();
    drivers.push_back(drv3);
    drv3->discoveredProxies.subscribe([&](Proxy proxy){ this->checkAndAddProxy(proxy); });

    //startProxyChecker();
} 

ProxyFinder::Proxy ProxyFinder::ProxyFinderService::operator [](size_t pos)
{
    return this->validProxies[pos];
}

size_t ProxyFinder::ProxyFinderService::size()
{
    return this->validProxies.size();
}
 
ProxyFinder::ProxyFinderService::~ProxyFinderService() 
{
    delete tpool;
    for (auto &c: drivers)
        delete c;
}

void ProxyFinder::ProxyFinderService::startProxyChecker()
{
    timerCheckProxysAlive.init(1 Timer_seconds, [&](Timer& t){
        if (lastAliveCheckIndex < 0)
            lastAliveCheckIndex = validProxies.size()-1;

        if (lastAliveCheckIndex < validProxies.size() && lastAliveCheckIndex >= 0)
        {
            if (!proxyIsWorking(validProxies[lastAliveCheckIndex]))
            {
                Utils::named_lock("pf::pfs::adp", [&](){ 
                    validProxies.erase(validProxies.begin() + lastAliveCheckIndex); 
                });
            }

                lastAliveCheckIndex--;
        }
    }, false);
}
 
void ProxyFinder::ProxyFinderService::checkAndAddProxy(Proxy proxy)
{
    if (!proxysVectorCointains(validProxies, proxy))
    {
        Utils::named_lock("pf::pfs::sts", [&](){ this->statistics.foundProxies++; });
        
        this->tpool->enqueue([&, proxy](){
            Utils::named_lock("pf::pfs::sts", [&](){ this->statistics.checkedProxies++; });
            if (proxyIsWorking(proxy))
            {
                Utils::named_lock("pf::pfs::adp", [&, proxy](){
                    this->validProxies.push_back(proxy);
                });
                Utils::named_lock("pf::pfs::sts", [&](){ this->statistics.validProxies++; });
            }
            else
                Utils::named_lock("pf::pfs::sts", [&](){ this->statistics.invalidProxies++; });

        });
    }
}

bool ProxyFinder::ProxyFinderService::proxysVectorCointains(vector<Proxy> &proxiesVector, Proxy proxy)
{
    auto it = std::find_if(proxiesVector.begin(), proxiesVector.end(), [&](Proxy curr){
        return proxy.toString() == curr.toString();
    });

    return it != proxiesVector.end();

}



bool ProxyFinder::ProxyFinderService::proxyIsWorking(Proxy proxy, int tries)
{
    while (tries > 0)
    {
    
        string tmpName = "/tmp/proxyCheckResult_"+Utils::createUniqueId_customFormat("??????????");
        string command = Utils::stringReplace(
            //"curl -x  "+proxy+"  --connect-timeout 15 -sS \"https://www.google.com\" --output \"/tmp/proxyCheckResult\"");
            "curl -x \"{proxy}\" --connect-timeout 15 -sS \"https://www.wikipedia.org\" --output \"{output}\"",
            {
                {"{proxy}", proxy.toString()},
                {"{output}", tmpName},
            }
        );
        string result = Utils::ssystem(command);
        Utils::ssystem("rm "+tmpName);
        if (result == "")
            return true;

        tries --;
    }

    return false;

    
}

future<ProxyFinder::Proxy> ProxyFinder::ProxyFinderService::pickRandomProxy()
{
    return std::async(std::launch::async, [&](){
        while (validProxies.size() == 0)
            usleep(1000);
        
        auto proxyIndex = rand() % validProxies.size();
        return validProxies[proxyIndex];
    });
}