#ifndef __ICACHEDB__H__ 
#define __ICACHEDB__H__ 
#include <map>
#include <string>
#include <DynamicVar.h>
#include <atomic>
#include <utils.h>
#include <sstream>

using namespace std;

class ICacheDB { 
public: 
    virtual DynamicVar get(string name, DynamicVar defaultValue = "") = 0;
    virtual void set(string name, DynamicVar value) = 0;
}; 
 
#endif 
