#ifndef __PROXYFINDER_PROXY__H__
#define __PROXYFINDER_PROXY__H__

#include <string>
#include <utils.h>
using namespace std;
namespace ProxyFinder{
    class Proxy{
    public:
        Utils_CreateGetSetProp(host, string, "NOCHANGE");
        Utils_CreateGetSetProp(port, int, -1);
        Utils_CreateGetSetProp2(protocol, string, "NOCHANGE", this->_protocol = Utils::strToUpper(this->_protocol));

        /**
         * @brief Construct a new Proxy object
         * 
         * @param host
         * @param port 
         * @param protocol 
         */
        Proxy(string host, int port, string protocol = "HTTP"){
            this->host(host);
            this->port(port);
            this->protocol(protocol);
        }

        /**
         * @brief Construct a new Proxy object using a string in the format "protocol://host:port"
         * 
         * @param protocol_host_port_string 
         */
        Proxy(string protocol_host_port_string){
            auto parts = Utils::splitString(protocol_host_port_string, ":");
            if (parts.size() == 3)
            {
                this->protocol(Utils::getOnly(parts[0], "SOCKSHTTPSsockshttps45"));
                if (parts[1].size() > 2 && parts[1].find("//") == 0) parts[1] = parts[1].substr(2);
                this->host(parts[1]);
                this->port(stoi(Utils::getOnly(parts[2], "0123456789")));
            }
            else
                throw invalid_argument("Invalid protocol_host_port format. Valid format is protocol://host:port (Example: SOCKS5://127.0.0.1:1234).");
            
        }

        Proxy(){};

        string toString(){
            return this->protocol() + "://" + this->host() + ":" +to_string(this->port());
        }

        operator string(){
            return toString();
        }
    };
}
 
#endif
