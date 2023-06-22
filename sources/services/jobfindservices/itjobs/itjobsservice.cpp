#include  "itjobsservice.h" 
 
ITJobsService::ITJobsService(ICacheDB *db, ILogger *logger, vector<string> searchUrls): CommonJobServiceWrapper(
    db,    
    logger->getNamedLogger("ItJobsService"),
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
    auto [error, result, remain ] = extractCuttingString(
        html, 
        "<div class=\"responsive-container\">", 
        "&nbsp;\n&nbsp;\n</div>"
    );

    if (error == Errors::NoError)
        return {true, remain, result};
    else
        return {false, "", ""};
}

Job ITJobsService::extractJobFromHTML(string jobHtml)
{
    Job job;

    auto [error, company, jobHtml1] = extractCuttingString(jobHtml, "<img alt=\"", "\"");
    job.company = company;
    jobHtml = jobHtml1;
    
    auto [error2, logo_url, jobHtml2] = extractCuttingString(jobHtml, "src=\"", "\"");
    job.company_logo_url = logo_url;
    jobHtml = jobHtml2;
    
    auto [error3, url, jobHtml3] = extractCuttingString(jobHtml, "href=\"", "\"");
    job.url = "https://www.itjobs.pt"+url;
    jobHtml = jobHtml3;
    
    auto [error4, title, jobHtml4] = extractCuttingString(jobHtml, ">", "<");
    job.title = title;
    jobHtml = jobHtml4;
    
    auto [error5, place, jobHtml5] = extractCuttingString(jobHtml, "fa fa-map-marker\"></i>&nbsp;", "&nbsp;&nbsp;");
    job.place = place;
    jobHtml = jobHtml5;
        
    auto [error6, workplace, jobHtml6] = extractCuttingString(jobHtml, "fa fa-clock-o\"></i>&nbsp;", "&nbsp;&nbsp;");
    jobHtml = jobHtml6;
    if (workplace.find("remote"))
        job.workplace = Job::WORKPLACE::REMOTE;

    auto [error7, remuneration, jobHtml7] = extractCuttingString(jobHtml, "fa fa-coins\"></i>&nbsp;", "&nbsp;&nbsp;");
    job.remuneration = remuneration;
    jobHtml = jobHtml7;

    job.jobId = "itjobs.pt."+Utils::getOnly(job.url, "0123456789");

    return job;

}; 

tuple<Error/*error*/, string /*extracted data*/, string/*remain text*/> 
ITJobsService::extractCuttingString(string text, string begin, string end, bool remove_begin_from_result, bool remove_end_from_result)
{
    string remain = text;
    if (auto posBeg = remain.find(begin); posBeg != string::npos)
    {
        remain = remain.substr(posBeg);
        if (remove_begin_from_result)
            remain = remain.substr(begin.size());

        if (auto posEnd = remain.find(end); posEnd != string::npos)
        {
            string result = remain.substr(0, posEnd);
            if (remove_end_from_result)
                result = result.substr(0, result.size()-end.size()+1);
            
            remain = remain.substr(posEnd+end.size());

            return {Errors::NoError, result, remain};
        }
        else
            return { Errors::createError("end not found"), "", text};
    }
    else
        return { Errors::createError("begin not found"), "", text};

}


string ITJobsService::downloadPage(string url)
{
    int maxTries = 5;
    while (maxTries > 0)
    {

        string cmd =    string("google-chrome ") +
                        string("--user-data-dir=/tmp/itjobpullingchromedata/ ") +
                        string("--headless ") +
                        string("--dump-dom ") +
                        string("--virtual-time-budget=5000 ") +
                        string("--timeout=5000 ") +
                        string("--run-all-compositor-stages-before-draw ") +
                        string("--disable-gpu ") +
                        string("--user-agent=\"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.93 Safari/537.36\" ") +
                        //string("--proxy-server=\"socks5://"+Utils::pickRandomProxy(true)+"\" ") +
                        string("--proxy-server=\"http://"+Utils::pickRandomProxy(true)+"\" ") +
                        string("'"+url+"' ");

        //log.debug("running command "+cmd);
        //((Logger*)(log.mainLogger))->flushCaches(); ;

        auto result = Utils::ssystem(cmd);
        
        if(result.find("<div class=\"btn btn-embossed btn-primary btn-lg pull-right\" data-target=\"#filters\" data-toggle=\"collapse\">") != string::npos)
            return result;
        
        maxTries--;
    }

    return "";
} 