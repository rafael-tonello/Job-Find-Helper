#include  "netempregosservice.h" 

NetEmpregosService::NetEmpregosService(ICacheDB *db, ILogger *logger, IProxyFinderService *proxyFinder, vector<string> searchUrls): CommonJobServiceWrapper(
    db,    
    logger->getNamedLogger("NetEmpregosService"),
    proxyFinder,
    searchUrls
){
    log.info("Initing Net-empregos.com pulling service.");
    if (searchUrls.size() == 0)
    {
        log.info("No url provided. Using default URL ("+defaultUrl+").");
        this->searchUrls = { defaultUrl };
    }
}
 
NetEmpregosService::~NetEmpregosService() 
{ 
    this->stop();
}

future<Job> NetEmpregosService::loadJobDetails(Job jobWithoutDetails)
{
    return std::async(std::launch::async, [&, jobWithoutDetails](){

        return jobWithoutDetails;
    });
}

//returns job_was_found, remain_html, found_job_htlm
tuple<bool, string, string> NetEmpregosService::getNextJobHtml(string html)
{
    string begString = "<div class=\"media-body align-self-center\" style=\"padding-left: 5px;\">";
    string endString = "style=\"position: absolute;top: 10px;right: 10px; font-size: 20px; font-weight: bold; color: green;\"";
    string result;
    if (auto begPos = html.find(begString); begPos != string::npos)
    {
        html = html.substr(begPos);
        if (auto endPos = html.find(endString); endPos != string::npos)
        {
            result = html.substr(0, endPos+endString.size());
            html = html.substr(endPos + endString.size());
        }
        else
        {
            result = html;
            html = "";
        }

        return {true, html, result};
    }
    else 
        return {false, "", ""};
}

Job NetEmpregosService::extractJobFromHTML(string jobHtml)
{
    Job result;

    string f1 = "<h2 style=\"font-size:20px;margin: 0px -5x; padding-right: 25px;\"><a class=\"oferta-link\" style=\"font-weight:bold\" href=";
    string f2 = "/>";
    string f3 = "</a></h2>";
    string f4 = "<img alt=\"";
    string f5 = "src=\"";
    string f6 = "<li><i class=\"flaticon-pin\" style=\"color: #d81e1e;\"></i> ";
    string f7 = "<li><i class=\"fa fa-tags\" style=\"color: #4da95d;\"></i> ";
    if (auto f1Pos = jobHtml.find(f1); f1Pos != string::npos)
    {
        jobHtml = jobHtml.substr(f1Pos + f1.size());
        if (auto f2Pos = jobHtml.find(f2); f2Pos != string::npos)
        {
            result.url = "https://www.net-empregos.com"+jobHtml.substr(0, f2Pos);
            result.jobId = "net-empregos.com."+Utils::getOnly(result.url, "0123456789");
            jobHtml = jobHtml.substr(f2Pos + f2.size());
            if (auto f3Pos = jobHtml.find(f3); f3Pos != string::npos)
            {
                result.title = jobHtml.substr(0, f3Pos);
                jobHtml = unescapeHTML(jobHtml.substr(f3Pos + f3.size()));
                if (auto f4Pos = jobHtml.find(f4); f4Pos != string::npos)
                {
                    jobHtml = jobHtml.substr(f4Pos + f4.size());
                    result.company = unescapeHTML(jobHtml.substr(0, jobHtml.find("\"")));

                    if (auto f5Pos = jobHtml.find(f5); f5Pos != string::npos)
                    {
                        jobHtml = jobHtml.substr(f5Pos + f5.size());
                        result.company_logo_url = jobHtml.substr(0, jobHtml.find("\""));
                        if (result.company_logo_url.find("https://") == string::npos)
                            result.company_logo_url = "" + result.company_logo_url;

                        jobHtml = jobHtml.substr(jobHtml.find("\""));

                        if (auto f6Pos = jobHtml.find(f6); f6Pos != string::npos)
                        {
                            jobHtml = jobHtml.substr(f6Pos + f6.size());
                            if (auto posLi = jobHtml.find("</li>"); posLi != string::npos)
                            {
                                result.place = unescapeHTML(jobHtml.substr(0, posLi));
                                jobHtml = jobHtml.substr(posLi);
                            }

                            if (auto f7Pos = jobHtml.find(f7); f7Pos != string::npos)
                            {
                                jobHtml = jobHtml.substr(f7Pos + f7.size());
                                result.category = unescapeHTML(jobHtml.substr(0, jobHtml.find("</li>")));
                                jobHtml = jobHtml.substr(jobHtml.find("</li>"));
                            }
                        }
                    }
                }
            }
        }
    }

    return result;

}
