#ifndef __IJOBSERVICE__H__ 
#define __IJOBSERVICE__H__ 

#include <stream.h>
#include <job.h>
#include <errors.h>
#include <logger.h>
#include <timer.h>
#include <icachedb.h>
 
class IJobService { 
public: 
    Common::Stream<Job> jobsStream;

    virtual Error start() = 0;
    virtual void stop() = 0;
}; 


/**
 * @brief This class contains the basic log for job search. It work in a lot of portals and prevent you
 * from have to implement IJobService and rewrite the same code for download the portals search pages every time.
 * If you need to do little changes here, you can override the functions downloadPage (to add a login, for example), findLastPublishedJobs and extractJobs. 
 */
class CommonJobServiceWrapper: public IJobService{
protected:
    NLogger log;
    ICacheDB *db;
    Timer workTimer;
    vector<string> searchUrls;
    int consecutiveNoJobsFoundCount = 0;


    void work();
    Error processJob(Job currJob);
    string unescapeHTML(const string& html);
    virtual string downloadPage(string url);
    CommonJobServiceWrapper(ICacheDB *db, NLogger log, vector<string> searchUrls);
    virtual tuple<Error, vector<Job>> findLastPublishedJobs();
    virtual tuple<Error, vector<Job> /*jobs*/> extractJobs(string html);

protected: /* Interface for child classes*/
    virtual future<Job> loadJobDetails(Job jobWithoutDetails) = 0;
    virtual tuple<bool, string, string> getNextJobHtml(string html) = 0;
    virtual Job extractJobFromHTML(string jobHtml) = 0;
public:
    Error start();
    void stop();
};
 
#endif 
