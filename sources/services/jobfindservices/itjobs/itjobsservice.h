#ifndef __ITJOBSSERVICE__H__ 
#define __ITJOBSSERVICE__H__ 
#include <ijobservice.h>
#include <timer.h>
class ITJobsService: public CommonJobServiceWrapper { 
private:
    string defaultUrl = "https://www.itjobs.pt/emprego?location=&q=developer&sort=date";
protected:
    future<Job> loadJobDetails(Job jobWithoutDetails);
    //returns job_was_found, remain_html, found_job_htlm
    tuple<bool, string, string> getNextJobHtml(string html);
    Job extractJobFromHTML(string jobHtml);

    string downloadPage(string url) override;
public: 
    ITJobsService(ICacheDB *db, ILogger *logger, IProxyFinderService *proxyFinder, vector<string> searchUrls);
}; 
 
#endif 
