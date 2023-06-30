#include  "openproxylistxyz.h" 
 
ProxyFinder::OpenProxyListXyz::OpenProxyListXyz() 
{ 
    this->tmr.init(5 Timer_minutes, [&](Timer& tmrref){
        downloadAndProcessList("https://openproxylist.xyz/http.txt", "HTTP");
        downloadAndProcessList("https://openproxylist.xyz/socks5.txt", "SOCKS5");
        downloadAndProcessList("https://openproxylist.xyz/socks4.txt", "SOCKS4");
    });
} 
 
ProxyFinder::OpenProxyListXyz::~OpenProxyListXyz() 
{ 
     
} 
 
void ProxyFinder::OpenProxyListXyz::downloadAndProcessList(string list, string proxysType)
{
    string proxysJSON = Utils::ssystem("curl -sS \""+list+"\"");

    auto proxies = Utils::splitString(proxysJSON, "\n");

    for (auto &currProxie: proxies)
    {
        this->discoveredProxies.stream(proxysType + "://"+currProxie);
        if (proxysType == "HTTP")
            this->discoveredProxies.stream(proxysType + "s://"+currProxie);
    }
}