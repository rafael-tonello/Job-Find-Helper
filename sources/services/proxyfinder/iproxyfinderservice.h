#ifndef __IPROXYFINDERSERVICE__H__ 
#define __IPROXYFINDERSERVICE__H__ 

#include <future>
#include "proxy.h"

using namespace std;

//namespace ProxyFinder{
    class IProxyFinderService {
    public: 
        /**
         * @brief return the amount of found and ready to use proxys
         * 
         * @return size_t 
         */
        virtual size_t size() = 0;

        /**
         * @brief get a proxy using a index. Index must be >= 0 and < size()
         * 
         * @param pos 
         * @return Proxy 
         */
        virtual ProxyFinder::Proxy operator[](size_t pos) = 0;

        /**
         * @brief Returns a random proxy
         * 
         * @return Proxy 
         */
        virtual future<ProxyFinder::Proxy> pickRandomProxy() = 0;



    }; 
//}
 
#endif 
