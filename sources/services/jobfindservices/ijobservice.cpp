#include "ijobservice.h"
 
CommonJobServiceWrapper::CommonJobServiceWrapper(ICacheDB *db, NLogger log, IProxyFinderService *proxyFinder, vector<string> searchUrls){
    this->db = db;
    this->log = log;
    this->searchUrls = searchUrls;
    this->proxyFinder = proxyFinder;

    workTimer.init(5 Timer_seconds, [&](Timer& t){
        this->work();
    }, false, true);
}

Error CommonJobServiceWrapper::start()
{
    this->workTimer.start(true);
    return Errors::NoError;
}

void CommonJobServiceWrapper::stop()
{
    this->workTimer.stop();
}

void CommonJobServiceWrapper::work()
{
    // for (auto &currJob: extractJobs(downloadNetEmpregosPage()))
    //     processJob(currJob);
    log.info2("Getting jobs");
    auto [error, lastJobs] = findLastPublishedJobs();
    if (error == Errors::NoError)
    {   
        log.info2("Found "+to_string(lastJobs.size())+" jobs");
        vector<future<void>> pendingTasks;
        for (auto currJobP: lastJobs)
        {
            pendingTasks.push_back(std::async(std::launch::async, [&](Job currJob){
                auto error2 = processJob(currJob);
                if (error2 != Errors::NoError)
                    log.error(error2);
            }, currJobP));
        }

        for(auto &c: pendingTasks)
            c.wait();
    }
    else
        log.error(error);

}


tuple<Error, vector<Job>> CommonJobServiceWrapper::findLastPublishedJobs()
{
    string htmls = "";
    vector<future<string>> downloads;
    for (auto &c: searchUrls)
    {
        downloads.push_back(std::async(std::launch::async, [&, c](){
            log.debug("Downloading "+c);
            auto downloadedHtml = this->downloadPage(c);
            log.debug(to_string(downloadedHtml.size()) + " bytes downloaded");
            return downloadedHtml;
        }));
    }

    for (auto &currDownload: downloads)
        htmls += currDownload.get();

    auto [error2, jobs] = extractJobs(htmls);

    if (error2 == Errors::NoError){
        return {Errors::NoError, jobs};
    }
    else
        return {error2, {}};
}

tuple<Error, vector<Job> /*jobs*/> CommonJobServiceWrapper::extractJobs(string html)
{
    vector<Job> foundJobs;
    Error error = Errors::NoError;
    while (true)
    {
        auto [jobFound, remainHtml, jobHtml] = getNextJobHtml(html);

        if (jobFound)
        {
            html = remainHtml;
            foundJobs.push_back(extractJobFromHTML(jobHtml));
        }
        else
            break;
    }
    
    if (foundJobs.size() == 0)
    {
        consecutiveNoJobsFoundCount++;
        if (consecutiveNoJobsFoundCount >= 5)
        {
            consecutiveNoJobsFoundCount = 0;
            error = Errors::createError("No jobs found after 5 attempts");
        }
    }
    else
        consecutiveNoJobsFoundCount = 0;

    return {error, foundJobs};

}

Error CommonJobServiceWrapper::processJob(Job currJob)
{
    if (!db->get("jobs."+currJob.jobId+".processed", false).getBool())
    {
        //currJob = loadJobDetails(currJob).get();

        db->set("jobs."+currJob.jobId+".processed", true);
        jobsStream.stream(currJob);

    }

    return Errors::NoError;
}

std::string CommonJobServiceWrapper::unescapeHTML(const std::string& html) {
    return Utils::stringReplace(html,  {
        {"&amp;", "&"},
        {"&lt;", "<"},
        {"&gt;", ">"},
        {"&quot;", "\""},
        {"&apos;", "'"},
        {"&nbsp;", " "}
        // Adicione outras entidades HTML aqui, se necessário
    });

}

string CommonJobServiceWrapper::downloadPage(string url)
{
    string tmpName = "/tmp/netEmpregos"+Utils::createUniqueId_customFormat("?????-?????")+".html";
    auto proxy = proxyFinder->pickRandomProxy().get();

    string command = Utils::stringReplace(
        "curl -x \"{proxy}\" --connect-timeout 15 -sS \"{url}\" --output \"{output}\"",
        {
            {"{proxy}", proxy.toString()},
            {"{url}", url},
            {"{output}", tmpName}
        }
    );
    string cmdResult = Utils::ssystem(command);

    string result =  Utils::readTextFileContent(tmpName);
    Utils::ssystem("rm "+tmpName+"");
    return result;
}
