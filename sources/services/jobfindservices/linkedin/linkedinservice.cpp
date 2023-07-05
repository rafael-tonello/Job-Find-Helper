#include  "linkedinservice.h" 
 
LinkedinService::LinkedinService(ICacheDB *db, ILogger *logger, IProxyFinderService *proxyFinder, vector<string> searchUrls): CommonJobServiceWrapper(
    db,    
    logger->getNamedLogger("LinkedinService"),
    proxyFinder,
    searchUrls
){
    log.info("Initing Linkedin.com pulling service.");
    if (searchUrls.size() == 0)
    {
        log.info("No url provided. Using default URL ("+defaultUrl+").");
        this->searchUrls = { defaultUrl };
    }
}
 
LinkedinService::~LinkedinService() 
{ 
     
} 

tuple<bool, string, string> LinkedinService::getNextJobHtml(string html)
{
    auto [error, found, remain] = helper_extractCuttingString(
        html, 
        "<div class=\"base-card relative w-full ",
        "          </time>"
    );

    if (error == Errors::NoError)
        return {true, found, remain};
    else
        return {false, "", ""};

}

Job LinkedinService::extractJobFromHTML(string jobHtml)
{
    Job result;

    auto [error, url, remain] = helper_extractCuttingString(jobHtml, "href=\"", "\"");
    result.url = url;
    jobHtml = remain;

    if (auto pos = result.url.find("?"); pos != string::npos)
        result.url = result.url.substr(0, pos);

    auto [error2, description, remain2] = helper_extractCuttingString(jobHtml, "<span class=\"sr-only\">", "</span>");
    result.title = StringUtils::trim(description);
    jobHtml = remain2;


    if (auto pos = jobHtml.find("<a class=\"hidden-nested-link\""); pos != string::npos)
        jobHtml = jobHtml.substr(pos);
    
    auto [error3, company, remain3] = helper_extractCuttingString(jobHtml, ">", "</a>");
    result.company = StringUtils::trim(company);
    jobHtml = remain3;


    auto [error4, location, remain4] = helper_extractCuttingString(jobHtml, "<span class=\"job-search-card__location\">", "</span>");
    result.location = StringUtils::trim(location);
    jobHtml = remain4;


    auto [error5, benefits, remain5] = helper_extractCuttingString(jobHtml, "<span class=\"result-benefits__text\">", "<!---->      </span>");
    if (error5 == Errors::NoError)
    {
        result.additionalInfo.add("benefits", benefits);
        jobHtml = remain5;
    }

    
    if (auto pos2 = jobHtml.find("<time class=\"job-search-card__listdate\""); pos2 != string::npos)
        jobHtml = jobHtml.substr(pos2);
    
    auto [error6, datetime, remain6] = helper_extractCuttingString(jobHtml, ">", "</time>");
    result.pubicationDateTime = StringUtils::trim(datetime);
    jobHtml = remain6;

    
    result.jobId = "linkedin.com."+Utils::getOnly(result.url, "0123456789");

    return result;
}

future<Job> LinkedinService::loadJobDetails(Job jobWithoutDetails)
{
    return std::async(std::launch::async, [&, jobWithoutDetails](){
        log.debug("Loading aditional job details.");

        //html = this->helper_downloadUsingChrome(jobWithoutDetails.url);


        return jobWithoutDetails;
    });
}

string LinkedinService::downloadPage(string url){ 
    return this->helper_downloadUsingChrome(url, [](string dataToBeValidated)
    {
        return dataToBeValidated.find("<ul class=\"jobs-search__results-list\">") != string::npos;
    });
}