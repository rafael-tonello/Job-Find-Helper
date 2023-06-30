#ifndef __GEONODECOM__H__ 
#define __GEONODECOM__H__ 
#include <timer.h>
#include <husymaker.h>
#include "iportaldriver.h"
 
namespace ProxyFinder{
    class GeoNodeCom: public IPortalDriver {
    private:
        Timer tmr;

        void downloadAndProcessList(string list);
        int currPage = 0;
    public: 
        GeoNodeCom(); 
        ~GeoNodeCom(); 
    }; 
}
 
#endif 
