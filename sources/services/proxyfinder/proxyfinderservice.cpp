#include  "proxyfinderservice.h" 
 
ProxyFinder::ProxyFinderService::ProxyFinderService() 
{
    this->tpool = new ThreadPool(true, threadPoolNumOfThreads);

    IPortalDriver * tmp = new FreeProxyListNet();
    drivers.push_back(tmp);
    drivers[0]->discoveredProxies.subscribe([&](Proxy proxy){ this->checkAndAddProxy(proxy); });

    startProxyChecker();
} 
 
ProxyFinder::ProxyFinderService::~ProxyFinderService() 
{
    delete tpool;
}

void ProxyFinder::ProxyFinderService::startProxyChecker()
{
    timerCheckProxysAlive.init(1 Timer_minutes, [&](Timer& t){
        if (lastAliveCheckIndex < 0)
            lastAliveCheckIndex = validProxies.size()-1;

        //do not use more than half of the total of threadpool threads
        for (int c = 0; c < (int)threadPoolNumOfThreads/2; c++)
        {
            //remove and readd the proxy to the list;
            if (lastAliveCheckIndex < validProxies.size())
            {
                this->checkAndAddProxy(validProxies[lastAliveCheckIndex]);
                Utils::named_lock("pf::pfs::adp", [&](){ validProxies.erase(validProxies.begin() + lastAliveCheckIndex); });
            }
            lastAliveCheckIndex--;
        }
    }, false);
}
 
void ProxyFinder::ProxyFinderService::checkAndAddProxy(Proxy proxy)
{
    if (!proxysVectorCointains(validProxies, proxy))
    {
        this->tpool->enqueue([&, proxy](){
            if (proxyIsWorking(proxy))
                Utils::named_lock("pf::pfs::adp", [&, proxy](){
                    this->validProxies.push_back(proxy);
                });
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



bool ProxyFinder::ProxyFinderService::proxyIsWorking(Proxy proxy)
{
    string tmpName = "/tmp/"+Utils::createUniqueId_customFormat("??????????");
    string command = Utils::stringReplace(
        "curl -x {proxy} --connect-timeout 10 -sS \"https://www.google.com.br\" --output {output}",
        {
            {"{proxy}", proxy.toString()},
            {"{output}", tmpName},
        }
    );
    string result = Utils::ssystem(command);
    Utils::ssystem("rm "+tmpName);
    if (result == "")
        return true;
    else
        return false;
    
}
