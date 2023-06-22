#ifndef __NEJOB__H__ 
#define __NEJOB__H__ 
#include <string>
#include <vector>

using namespace std;
template<typename T>
class ValueRange{
public:
    T low;
    T high;
    ValueRange(){};
    ValueRange(T low, T high): low(low), high(high){}
};

class Job { 
public: 
    enum WORKPLACE{REMOTE, INSITE, HYBRID, NOT_SPECIFIED};
    string jobId; 
    string title;
    string url;
    string company;
    string company_logo_url;
    string place;
    string category;
    vector<string> necessarySkills;
    vector<string> niceToHaveSkills;
    //ValueRange<double> remunerationRange;
    string remuneration;
    string additionalInfo;
    string pubicationDateTim;
    WORKPLACE workplace = NOT_SPECIFIED;
}; 

#endif 
