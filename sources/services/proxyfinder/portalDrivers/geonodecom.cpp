#include  "geonodecom.h" 
 
ProxyFinder::GeoNodeCom::GeoNodeCom() 
{ 
    this->tmr.init(10 Timer_seconds, [&](Timer& tmrref){
        currPage++;
        if (currPage > 10)
        {
            tmrref.changeInterval(5 Timer_minutes);
            currPage = 1;
        }

        downloadAndProcessList("https://proxylist.geonode.com/api/proxy-list?limit=500&page="+to_string(currPage)+"&sort_by=lastChecked&sort_type=desc");
        
    });
} 
 
ProxyFinder::GeoNodeCom::~GeoNodeCom() 
{ 
     
} 
 
void ProxyFinder::GeoNodeCom::downloadAndProcessList(string list)
{
    string proxysJSON = Utils::ssystem("curl -sS \""+list+"\"");

    auto rootNode = JsonIO::parseJson(proxysJSON);

    rootNode["data"].foreachChilds([&](string childName, TreeNode &child)
    {
        string ip = child["ip"].value;
        uint port = child["port"].value;

        child["protocols"].foreachChilds([&](string cName, TreeNode &protocol){
            string proxyInfo = protocol.value.getString() + "://"+ip + ":" + to_string(port);
            this->discoveredProxies.stream(Proxy(proxyInfo));
        });
    });


    // {
    //     "_id": "6314cfffde95dae521d06819",
    //     "ip": "186.251.255.249",
    //     "anonymityLevel": "elite",
    //     "asn": "AS267513",
    //     "city": "Coqueiros do Sul",
    //     "country": "BR",
    //     "created_at": "2022-09-04T16:19:11.569Z",
    //     "google": false,
    //     "isp": "Seanet Telecom Carazinho Eireli",
    //     "lastChecked": 1687660436,
    //     "latency": 292.327,
    //     "org": "Seanet Telecom Carazinho Eireli",
    //     "port": "31337",
    //     "protocols": [
    //         "socks4"
    //     ],
    //     "region": null,
    //     "responseTime": 4707,
    //     "speed": 1,
    //     "updated_at": "2023-06-25T02:33:56.837Z",
    //     "workingPercent": null,
    //     "upTime": 98.63013698630137,
    //     "upTimeSuccessCount": 2448,
    //     "upTimeTryCount": 2482
    // },
}