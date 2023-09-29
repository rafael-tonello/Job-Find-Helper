#ifndef __NESERVICE__H__ 
#define __NESERVICE__H__ 
#include <stream.h>
#include <job.h>
#include <errors.h>
#include <logger.h>
#include <utils.h>
#include <icachedb.h>
#include <ijobservice.h>
#include <iproxyfinderservice.h>
#include <dependencyInjectionManager.h>

class NetEmpregosService: public CommonJobServiceWrapper { 
protected:
    string defaultUrl = "https://www.net-empregos.com/emprego-informatica-programacao.asp" ;

    future<Job> loadJobDetails(Job jobWithoutDetails);
    //returns job_was_found, remain_html, found_job_htlm
    tuple<bool, string, string> getNextJobHtml(string html);
    Job extractJobFromHTML(string jobHtml);
    
public: 

    NetEmpregosService(vector<string> searchUrls = {});
    ~NetEmpregosService();
};
#endif 
