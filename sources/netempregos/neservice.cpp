#include  "neservice.h" 

NetEmpregos::NEService::NEService(ICacheDB *db, ILogger *logger, vector<string> searchUrls){
    this->db = db;
    this->log = logger->getNamedLogger("NetEmpregosService");
    if (searchUrls.size() > 0)
        this->searchUrls = searchUrls;
}


Error NetEmpregos::NEService::start()
{
    if (!running)
    {
        thread th([&](){
            running = true;
            workThreadWait.lock();
            int timeout = 0;
            while (running)
            {
                timeout -= 1000;
                if (timeout <= 0)
                {
                    timeout = 10 seconds;
                    this->work();
                }
                usleep(1000);
                
            }
            workThreadWait.unlock();

        });
        th.detach();

        return Errors::NoError;
    }
    else
        return Errors::createError("Process is already running");
}

void NetEmpregos::NEService::stop()
{
    running = false;
    workThreadWait.lock();
    workThreadWait.unlock();
}

void NetEmpregos::NEService::work()
{
    // for (auto &currJob: extractJobs(downloadNetEmpregosPage()))
    //     processJob(currJob);
    auto [error, lastJobs] = getLastPublishedNetEmpregosJobs();
    if (error == Errors::NoError)
    {
        for (auto &currJob: lastJobs)
        {
            auto error2 = processJob(currJob);
            if (error2 != Errors::NoError)
                log.error(error2);
        }
    }
    else
        log.error(error);

}

tuple<Error, vector<NetEmpregos::NEJob>> NetEmpregos::NEService::getLastPublishedNetEmpregosJobs()
{
    string htmls = "";
    for (auto &c: searchUrls)
        htmls += this->downloadNetEmpregosPage(c);

    auto [error2, jobs] = extractJobs(htmls);
    if (error2 == Errors::NoError){
        return {Errors::NoError, jobs};
    }
    else
        return {error2, {}};
}
 
NetEmpregos::NEService::~NEService() 
{ 
    this->stop();
} 
 
string NetEmpregos::NEService::downloadNetEmpregosPage(string url)
{
    Utils::ssystem("rm /tmp/netEmpregos.html 2>/dev/null");
    //auto result = Utils::ssystem("curl -sS \""+this->url+"\" --connect-timeout 25 --output \"/tmp/netEmpregos.html\"");

    Utils::downloadWithRandomProxy(url, "/tmp/netEmpregos.html", 5, 5);
    string result =  Utils::readTextFileContent("/tmp/netEmpregos.html");
    return result;
}

tuple<Error, vector<NetEmpregos::NEJob> /*jobs*/> NetEmpregos::NEService::extractJobs(string html)
{
    vector<NEJob> foundJobs;
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

Error NetEmpregos::NEService::processJob(NetEmpregos::NEJob currJob)
{
    if (!db->get("processed."+currJob.url, false).getBool())
    {
        db->set("processed."+currJob.url, true);
        jobsStream.stream(currJob);
    }
}

//returns job_was_found, remain_html, found_job_htlm
tuple<bool, string, string> NetEmpregos::NEService::getNextJobHtml(string html)
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

NetEmpregos::NEJob NetEmpregos::NEService::extractJobFromHTML(string jobHtml)
{
    NEJob result;

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

std::string NetEmpregos::NEService::unescapeHTML(const std::string& html) {
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