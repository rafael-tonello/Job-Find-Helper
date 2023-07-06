#ifndef __NEJOB__H__ 
#define __NEJOB__H__ 
#include <string>
#include <vector>
#include <DynamicVar.h>
#include <husymaker.h>

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
    vector<string> getNames();
    vector<tuple<string, DynamicVar>>& getRawData();
    void forEach(function<void(string name, DynamicVar value)> f);
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


    /**
     * @brief serialize the job data to a JSON string
     * 
     * @return a JSON string 
     */
    string serialize(bool format = true){
        TreeNode tn;
        tn["jobId"] = this-> jobId;
        tn["title"] = this-> title;
        tn["url"] = this-> url;
        tn["company"] = this-> company;
        tn["company_logo_url"] = this-> company_logo_url;
        tn["location"] = this-> location;
        tn["pubicationDateTime"] = this-> pubicationDateTime;

        int index = 0;
        additionalInfo.forEach([&](string key, DynamicVar value){
            tn["aditionalInfo"][index]["key"] = key;
            tn["aditionalInfo"][index]["value"] = value;
        });

        switch( this->workplace)
        {
            case REMOTE:
                tn["workPlace"] = "REMOTE";
            break;
            case INSITE:
                tn["workPlace"] = "INSITE";
            break;
            case HYBRID:
                tn["workPlace"] = "HYBRID";
            break;
            default:
                tn["workPlace"] = "NOT_SPECIFIED";
            break;
        }

        return JsonIO::exportJson(tn, format);
    }
}; 

#endif 
