#include "ijobservice.h"
 
CommonJobServiceWrapper::CommonJobServiceWrapper(ICacheDB *db, NLogger log, IProxyFinderService *proxyFinder, vector<string> searchUrls){
    this->db = db;
    this->log = log;
    this->searchUrls = searchUrls;
    this->proxyFinder = proxyFinder;

    //randomize timers interval to try prevent all services running at same time :D
    // auto interval = 3 Timer_minutes;
    // interval += rand() % (2 Timer_minutes);

    auto interval = 5 Timer_minutes;
    interval += rand() % (5 Timer_minutes);

    this->log.info("Polling interval: " + to_string((double)((double)interval/1000.f/1000.f/60.f)) +" minutes");
    workTimer.init(interval, [&](Timer& t){
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

    log.info2("Getting jobs done");

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
        auto [jobFound, jobHtml, remainHtml] = getNextJobHtml(html);

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

tuple<Error/*error*/, string /*extracted data*/, string/*remain text*/> 
CommonJobServiceWrapper::helper_extractCuttingString(string text, string begin, string end, bool remove_begin_from_result, bool remove_end_from_result)
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

string CommonJobServiceWrapper::helper_downloadUsingChrome(string url, function<bool(string html)> f_toValidadeDownloadedData, int maxTries)
{
    while (maxTries > 0)
    {

        string cmd =    string("google-chrome ") +
                        string("--user-data-dir=/tmp/itjobpullingchromedata/ ") +
                        string("--headless ") +
                        string("--dump-dom ") +
                        string("--virtual-time-budget=15000 ") +
                        string("--timeout=15000 ") +
                        string("--run-all-compositor-stages-before-draw ") +
                        string("--disable-gpu ") +
                        string("--user-agent=\"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.93 Safari/537.36\" ") +
                        //string("--proxy-server=\"socks5://"+Utils::pickRandomProxy(true)+"\" ") +
                        string("--proxy-server=\""+this->proxyFinder->pickRandomProxy().get().toString()+"\" ") +
                        string("'"+url+"' ");

        log.debug("Download page with google-chrome (running the command "+cmd+")");
        //((Logger*)(log.mainLogger))->flushCaches(); ;

        //auto result = Utils::ssystem(cmd);
        auto result = Utils::ssystem2(cmd, 30000);

        if (result.find("\nCommand timeout") != string::npos)
            log.error("google-chrome command timeout");
        
        if (f_toValidadeDownloadedData(result))
            return result;
        
        maxTries--;
    }

    return "";
}