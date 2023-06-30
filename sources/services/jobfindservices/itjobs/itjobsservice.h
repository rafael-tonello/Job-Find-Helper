#ifndef __ITJOBSSERVICE__H__ 
#define __ITJOBSSERVICE__H__ 
#include <ijobservice.h>
#include <timer.h>
class ITJobsService: public CommonJobServiceWrapper { 
private:
    /**
     * @brief Find a text between 'begin' and 'and' and return this text and the remain text (texto after the match)
     * 
     * @param text source text
     * @param begin the initial delimiter
     * @param end the final delimiter
     * @param remove_begin_from_result remove or not the initial delimiter from the found text
     * @param remove_end_from_result remove or not the final delimiter from the found text
     * @return tuple<Error, string extracted_data, string remain_text> 
     */
    tuple<Error, string, string> extractCuttingString(string text, string begin, string end, bool remove_begin_from_result = true, bool remove_end_from_result = true);

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
