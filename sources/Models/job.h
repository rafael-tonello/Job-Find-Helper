#ifndef __NEJOB__H__ 
#define __NEJOB__H__ 
#include <string>
#include <vector>
#include <DynamicVar.h>

using namespace std;
// template<typename T>
// class ValueRange{
// public:
//     T low;
//     T high;
//     ValueRange(){};
//     ValueRange(T low, T high): low(low), high(high){}
// };

class JobAdditionalInfo{
private:
    vector<tuple<string, DynamicVar>> data;
public:
    DynamicVar get(string name, DynamicVar defaultValue = "");
    vector<DynamicVar> getMultiple(string name, DynamicVar defaultValue = "");
    void add(string name, DynamicVar value);
    size_t size();
    DynamicVar operator[] (int index);
    DynamicVar operator[] (string name);
};

class Job { 
public: 
    enum WORKPLACE{REMOTE, INSITE, HYBRID, NOT_SPECIFIED};
    string jobId;
    string title;
    string url;
    string company;
    string company_logo_url;
    string location;
    
    // vector<string> necessarySkills;
    // vector<string> niceToHaveSkills;
    //ValueRange<double> remunerationRange;
    // string remuneration;
    JobAdditionalInfo additionalInfo;
    string pubicationDateTime;
    
    WORKPLACE workplace = NOT_SPECIFIED;
}; 

#endif 
