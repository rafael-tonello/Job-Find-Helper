#ifndef __CACHEDB__H__ 
#define __CACHEDB__H__ 
#include <map>
#include <string>
#include <DynamicVar.h>
#include <atomic>
#include <utils.h>
#include <sstream>
#include <icachedb.h>

using namespace std;

class TextFileDB: public ICacheDB { 
private:
    void dump();
    void load();
    map<string, DynamicVar> vars;
    atomic<bool> autoDumpRunning = true;
    mutex autoDumpWait;
    string fileName;
public: 
    TextFileDB(string fileName); 
    ~TextFileDB();

    DynamicVar get(string name, DynamicVar defaultValue = "");
    void set(string name, DynamicVar value);

}; 
 
#endif 
