#ifndef __NEJOB__H__ 
#define __NEJOB__H__ 
#include <string>
namespace NetEmpregos{
    using namespace std;
    class NEJob { 
    public: 
        string title;
        string url;
        string company;
        string company_logo_url;
        string place;
        string category;
    }; 
}

#endif 
