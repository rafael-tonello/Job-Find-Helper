#ifndef __IJOBSERVICE__H__ 
#define __IJOBSERVICE__H__ 

#include <stream.h>
#include <job.h>
#include <errors.h>
#include <logger.h>
#include <timer.h>
#include <icachedb.h>
#include <iproxyfinderservice.h>
#include <dependencyInjectionManager.h>
 
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
    DimDfPropP (ICacheDB, db);
    DimDfPropP (IProxyFinderService, proxyFinder);

    NLogger log;
    Timer workTimer;
    vector<string> searchUrls;
    int consecutiveNoJobsFoundCount = 0;


    void work();
    Error processJob(Job currJob);
    string unescapeHTML(const string& html);
    virtual string downloadPage(string url);
    CommonJobServiceWrapper(string logName, vector<string> searchUrls);
    virtual tuple<Error, vector<Job>> findLastPublishedJobs();
    virtual tuple<Error, vector<Job> /*jobs*/> extractJobs(string html);

    /**
     * @brief Find a text between 'begin' and 'and' and return this text and the remain text (texto after the match)
     * 
     * @param text source text
     * @param begin the initial delimiter
     * @param end the final delimiter
     * @param remove_begin_from_result remove or not the initial delimiter from the found text
     * @param remove_end_from_result remove or not the final delimiter from the found text
     * @returns Error The Error::NoError or an Error describign the error;
     * @returns extracted_data the found data;
     * @returns remain_text the remain text (after the extracted data);
     */
    tuple<Error, string, string> helper_extractCuttingString(string text, string begin, string end, bool remove_begin_from_result = true, bool remove_end_from_result = true);
    string helper_downloadUsingChrome(string url, function<bool(string html)> f_toValidadeDownloadedData, int maxTries = 5);

    Timer chromiumKiller;

protected: /* Interface for child classes*/
    /**
     * @brief Load aditional Job Details. Can be used to download and parse extra pages
     * 
     * @param jobWithoutDetails a Job object with basic data (normally returned by the extracJobFromHtml function)
     * @return future<Job> 
     */
    virtual future<Job> loadJobDetails(Job jobWithoutDetails) = 0;
    /**
     * @brief Get the Next Job Html object
     * 
     * @param html 
     * @return tuple<bool, string, string> 
     * @return bool sucess status indicating if the job was found
     * @return string with the found job html
     * @return string with the remain html (after the found job)
     */
    virtual tuple<bool, string, string> getNextJobHtml(string html) = 0;

    /**
     * @brief Extracth the job inforation from a jobHtml (jobHtml is one of the returned ones from getNextJobHtml)
     * 
     * @param jobHtml 
     * @return Job object with the job data.
     */
    virtual Job extractJobFromHTML(string jobHtml) = 0;
public:
    Error start();
    void stop();
};
 
#endif 
