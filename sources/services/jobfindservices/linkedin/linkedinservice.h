#ifndef __LINKEDINSERVICE__H__ 
#define __LINKEDINSERVICE__H__ 

#include <future>
#include <tuple>
#include <string>
#include <job.h>
#include <ijobservice.h>
#include <StringUtils.h>
 
using namespace std;
class LinkedinService: public CommonJobServiceWrapper { 
private:
    string defaultUrl = "https://www.linkedin.com/jobs/search/?currentJobId=3648430440&distance=25&geoId=100364837&keywords=developer&refresh=true";
protected: 
    /* CommonJobServiceWrapper interface */
    future<Job> loadJobDetails(Job jobWithoutDetails) override;
    tuple<bool, string, string> getNextJobHtml(string html) override;
    Job extractJobFromHTML(string jobHtml) override;
protected:
    string downloadPage(string url) override;
public: 
    LinkedinService(ICacheDB *db, ILogger *logger, IProxyFinderService *proxyFinder, vector<string> searchUrls); 
    ~LinkedinService(); 
}; 
 
#endif 
