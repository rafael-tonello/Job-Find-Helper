#ifndef __PrefixTreeDB__H__ 
#define __PrefixTreeDB__H__ 
#include <string>
#include <icachedb.h>
#include <patricia.h>
 

#define LIST_VALUE_IDENTIFICATION "_.-lis_value-._"
class PrefixTreeDB: public ICacheDB { 
private:
    const int MAX_VALUE_SIZE = 128;
    string validKeyChars = string("abcdefghijklmnopqrstuvxywz./:0123456789_-");
    string fileName;
    Trees::PatriciaFile<string> *db;
    string ensureKeyChars(string key);
public: 
    PrefixTreeDB(string fileName); 
    ~PrefixTreeDB(); 

    DynamicVar get(string name, DynamicVar defaultValue = "");
    void set(string name, DynamicVar value);
}; 
 
#endif 
