#ifndef __PATRICIADB__H__ 
#define __PATRICIADB__H__ 
#include <string>
#include <icachedb.h>
#include <patricia.h>
 
class PatriciaDB: public ICacheDB { 
private:
    string validKeyChars = string("abcdefghijklmnopqrstuvxywz./:0123456789_-");
    string fileName;
    Trees::PatriciaFile<string> *db;
    string ensureKeyChars(string key);
public: 
    PatriciaDB(string fileName); 
    ~PatriciaDB(); 

    DynamicVar get(string name, DynamicVar defaultValue = "");
    void set(string name, DynamicVar value);
}; 
 
#endif 
