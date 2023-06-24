#include  "freeproxylistnet.h" 
 
ProxyFinder::FreeProxyListNet::FreeProxyListNet() 
{ 
     this->tmr.init(1 Timer_hours, [&](Timer& tmrref){
        
        string proxysHtml = Utils::ssystem("curl -sS \"https://free-proxy-list.net/\"");

        if (auto pos = proxysHtml.find("Updated at "); pos != string::npos)
        {
            proxysHtml = proxysHtml.substr(pos);
            if (auto pos2 = proxysHtml.find("\n\n"); pos != string::npos)
            {
                pos2 += 2;
                proxysHtml = proxysHtml.substr(pos2);
                while (true)
                {
                    if (auto cutPos = proxysHtml.find("\n"); cutPos != string::npos)
                    {
                        string currProxy = proxysHtml.substr(0, cutPos);
                        proxysHtml = proxysHtml.substr(cutPos+1);

                        if (currProxy.find("div") == string::npos)
                            this->discoveredProxies.stream(Proxy("http://"+currProxy));
                        else
                            break;
                    }
                    else
                        break;
                }
            }
            else
                cerr << "Error initing proxy list. Html parse error";
        }
        else
            cerr << "Error initing proxy list. Html parse error";
    });
} 
 
ProxyFinder::FreeProxyListNet::~FreeProxyListNet() 
{

}
