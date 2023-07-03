#include  "itjobsservice.h" 
 
ITJobsService::ITJobsService(ICacheDB *db, ILogger *logger, IProxyFinderService *proxyFinder, vector<string> searchUrls): CommonJobServiceWrapper(
    db,    
    logger->getNamedLogger("ItJobsService"),
    proxyFinder,
    searchUrls
){
    log.info("Initing Itjobs.pt pulling service.");
    if (searchUrls.size() == 0)
    {
        log.info("No url provided. Using default URL ("+defaultUrl+").");
        this->searchUrls = { defaultUrl };
    }
}

future<Job> ITJobsService::loadJobDetails(Job jobWithoutDetails)
{
    return std::async(std::launch::async, [jobWithoutDetails](){
        return jobWithoutDetails;
    });
}

//returns job_was_found, remain_html, found_job_htlm
tuple<bool, string, string> ITJobsService::getNextJobHtml(string html)
{
    auto [error, result, remain ] = helper_extractCuttingString(
        html, 
        "<div class=\"responsive-container\">", 
        "&nbsp;\n&nbsp;\n</div>"
    );

    if (error == Errors::NoError)
        return {true, result, remain};
    else
        return {false, "", ""};
}

Job ITJobsService::extractJobFromHTML(string jobHtml)
{
    Job job;

    auto [error, company, jobHtml1] = helper_extractCuttingString(jobHtml, "<img alt=\"", "\"");
    job.company = company;
    jobHtml = jobHtml1;
    
    auto [error2, logo_url, jobHtml2] = helper_extractCuttingString(jobHtml, "src=\"", "\"");
    job.company_logo_url = logo_url;
    jobHtml = jobHtml2;
    
    auto [error3, url, jobHtml3] = helper_extractCuttingString(jobHtml, "href=\"", "\"");
    job.url = "https://www.itjobs.pt"+url;
    jobHtml = jobHtml3;
    
    auto [error4, title, jobHtml4] = helper_extractCuttingString(jobHtml, ">", "<");
    job.title = title;
    jobHtml = jobHtml4;
    
    auto [error5, place, jobHtml5] = helper_extractCuttingString(jobHtml, "fa fa-map-marker\"></i>&nbsp;", "&nbsp;&nbsp;");
    job.location = place;
    jobHtml = jobHtml5;
        
    auto [error6, workplace, jobHtml6] = helper_extractCuttingString(jobHtml, "fa fa-clock-o\"></i>&nbsp;", "&nbsp;&nbsp;");
    jobHtml = jobHtml6;
    if (workplace.find("remote"))
        job.workplace = Job::WORKPLACE::REMOTE;

    auto [error7, remuneration, jobHtml7] = helper_extractCuttingString(jobHtml, "fa fa-coins\"></i>&nbsp;", "&nbsp;&nbsp;");
    job.additionalInfo.add("remuneration", remuneration);
    jobHtml = jobHtml7;

    job.jobId = "itjobs.pt."+Utils::getOnly(job.url, "0123456789");

    return job;

}; 

string ITJobsService::downloadPage(string url)
{
    return this->helper_downloadUsingChrome(url, [&](string dataToBeValidated){
        return dataToBeValidated.find("<div class=\"btn btn-embossed btn-primary btn-lg pull-right\" data-target=\"#filters\" data-toggle=\"collapse\">") != string::npos;
    });
} 