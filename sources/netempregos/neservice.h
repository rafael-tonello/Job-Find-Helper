#ifndef __NESERVICE__H__ 
#define __NESERVICE__H__ 
#include <stream.h>
#include <nejob.h>
#include <errors.h>
#include <logger.h>
#include <utils.h>
#include <icachedb.h>

namespace NetEmpregos{
    #define milisseconds *1000
    #define seconds milisseconds*60
    class NEService { 
    protected:
        vector<string> searchUrls = { "https://www.net-empregos.com/emprego-informatica-programacao.asp" };
        NLogger log;
        void work();
        tuple<Error, vector<NEJob>> getLastPublishedNetEmpregosJobs();
        string downloadNetEmpregosPage(string url);
        tuple<Error, vector<NEJob> /*jobs*/> extractJobs(string html);
        Error processJob(NEJob currJob);

        //returns job_was_found, remain_html, found_job_htlm
        tuple<bool, string, string> getNextJobHtml(string html);
        NEJob extractJobFromHTML(string jobHtml);

        atomic<bool> running;
        mutex workThreadWait;
        std::string unescapeHTML(const std::string& html);

        ICacheDB *db;

        int consecutiveNoJobsFoundCount = 0;
    public: 
        Common::Stream<NEJob> jobsStream;

        Error start();
        void stop();

        NEService(ICacheDB *db, ILogger *logger, vector<string> searchUrls = {});

        ~NEService();
    }; 
}
#endif 
