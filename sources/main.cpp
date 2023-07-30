#include <iostream>
#include <unistd.h>
#include <textfiledb.h>
#include <icachedb.h>
#include <logger.h>
#include <netempregosservice.h>
#include <itjobsservice.h>
#include <linkedinservice.h>
#include <LoggerConsoleWriter.h>
#include <LoggerFileWriter.h>
#include <prefixtreedb.h>
#include <argparser.h>
#include <utils.h>
#include <iproxyfinderservice.h>
#include <proxyfinderservice.h>


//semantic versioning
string INFO_VERSION = "0.2.0";

using namespace std;
class App{
private:    
    ArgParser argParser;
    vector<string> urls;
    vector<string> commands;
    int logLevel;
    NLogger *log;

    vector<IJobService*> jobServices;

    ICacheDB *db = new PrefixTreeDB(getApplicationDirectory() + "/db");
    ILogger *logger = new Logger({new LoggerConsoleWriter(logLevel), new LoggerFileWriter(determinteLogFile())}, false);
    IProxyFinderService *proxyFinderService = nullptr;
    ProxyFinder::ProxyFinderService *proxyFinderService2;

public:
    App(){
    }

    ~App()
    {
        for (auto &c: jobServices)
            delete c;

        delete db;
        delete log;
        delete logger;

        if (proxyFinderService != nullptr)
            delete proxyFinderService;
    }

    int run(int argc, char** argv){
        cout << "Net-Empregos monitor and trigger, version " << INFO_VERSION << endl;
        argParser = ArgParser(argc, argv);

        if (argParser.containsExact( {string("-h"), string("--help")} ))
            return displayHelp();
 
        parseUrls();
        parseCommands();
        this->logLevel = determineLogLevel(commands.size());
        this->logger = new Logger({new LoggerConsoleWriter(logLevel), new LoggerFileWriter(determinteLogFile())}, true, true);
        this->log = logger->getNamedLoggerP("Main");
        
        
        this->db = new PrefixTreeDB(getApplicationDirectory() + "/db");

        initProxyFinderService();
        initNetempregosService();
        initItjobsService();
        initLinkedinService();

        while (true)
            usleep(1000);
        return 0;
    }

    void initProxyFinderService()
    {
        this->proxyFinderService = new ProxyFinder::ProxyFinderService();
        this->proxyFinderService2 = (ProxyFinder::ProxyFinderService*)proxyFinderService;
    }

    void initNetempregosService()
    {
        auto validUrls = Utils::filterVector<string>(this->urls, [](string item) { auto pos = item.find("net-empregos.com"); return pos > 7 && pos < 15; });
        IJobService *ne_service = new NetEmpregosService(db, logger, proxyFinderService, validUrls);
        ne_service->jobsStream.subscribe([&](Job foundJob){ this->processReceivedJOB(foundJob); });
        ne_service->start();

        this->jobServices.push_back(ne_service);
    }

    void initItjobsService()
    {
        auto validUrls =Utils::filterVector<string>(this->urls, [](string item) { auto pos = item.find("itjobs.pt"); return pos > 7 && pos < 15; });
        IJobService *ij_service = new ITJobsService(db, logger, proxyFinderService, validUrls);
        ij_service->jobsStream.subscribe([&](Job foundJob){ this->processReceivedJOB(foundJob); });
        ij_service->start();

        this->jobServices.push_back(ij_service);
    }

    void initLinkedinService()
    {
        auto validUrls =Utils::filterVector<string>(this->urls, [](string item) { auto pos = item.find("linkedin.com"); return pos > 7 && pos < 15; });
        IJobService *ij_service = new LinkedinService(db, logger, proxyFinderService, validUrls);
        ij_service->jobsStream.subscribe([&](Job foundJob){ this->processReceivedJOB(foundJob); });
        ij_service->start();

        this->jobServices.push_back(ij_service);
    }

    void processReceivedJOB(Job job)
    {
        string msg =    string("New Job found on NetEmpregos.com: \n")+
                        string("    ") + job.title + string(" (by ")+ job.company+ string(")\n") + 
                        string("    ") + string("place of job: ") + job.location + string("\n") + 
                        string("    ") + string("url: ") + job.url +string("\n");
                        if (job.additionalInfo.size() > 0)
                        {
                            msg += string("    ") + string("more info: ") + job.url +string("\n");
                            job.additionalInfo.forEach([&](auto key, auto value){
                               msg += string("        ")  + key + ": " + value.getString() + string("\n");
                            });
                        }
                        
    
    
        log->info(msg);

        thread th([&, job](){
            for (auto &command: commands)
                runJobCommand(command, job, log);
        });

        th.detach();
    }

    void parseUrls()
    {
        urls = argParser.getList({"-u", "--url"});
        if (urls.size() > 0)
        {
            cout << endl << "   Monitoring urls: " <<endl;
            for (auto &c: urls)
                cout << "       " << c << endl;
        }
        else
            cout << "   No urls provided. Monitoring default url:" << endl << "       https://www.net-empregos.com/emprego-informatica-programacao.asp" << endl;
    }

    void parseCommands()
    {
        commands = argParser.getList({"-c", "--command"});
        if (commands.size() > 0)
        {
            cout << endl << "   Commands: " << endl;
            for (auto &c: commands)
                cout << "       " << c << endl;
        }
        else
            cout << "   No comamnds specified to be triggered." << endl;
    }

    int determineLogLevel(size_t totalOfCommands)
    {
        int tmpLogLevel = LOGGER_LOGLEVEL_DEBUG2;
        //if user do not specify any command, sets the log level (of the console) to 'info',
        //to mkeep stdout clean to user be able to focus in the jobs informations
        if (totalOfCommands == 0)
        {
            cout << endl << "As you do not specify any command to be triggered, this app will suply some output messages to try to maintain the terminal clean and you can focus in the jobs oportunities." << endl;
            tmpLogLevel = LOGGER_LOGLEVEL_INFO;
        }

        if (auto ll = argParser.getList({"--log-level -ll"}); ll.size() > 0)
        {
            string text = Utils::strToUpper(ll[0]);
            if (text == "DEBUG") tmpLogLevel = LOGGER_LOGLEVEL_DEBUG;
            else if (text == "INFO2") tmpLogLevel = LOGGER_LOGLEVEL_INFO2;
            else if (text == "INFO") tmpLogLevel = LOGGER_LOGLEVEL_INFO;
            else if (text == "WARNING") tmpLogLevel = LOGGER_LOGLEVEL_WARNING;
            else if (text == "ERROR") tmpLogLevel = LOGGER_LOGLEVEL_ERROR;
            else if (text == "CRITICAL") tmpLogLevel = LOGGER_LOGLEVEL_CRITICAL;
        }

        return tmpLogLevel;
    }

    std::string getApplicationDirectory() 
    {
        char result[ PATH_MAX ];
        ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
        std::string appPath = std::string( result, (count > 0) ? count : 0 );

        std::size_t found = appPath.find_last_of("/\\");
        string directory = appPath.substr(0,found);

        return directory;
    }

    std::string determinteLogFile()
    {
        string logFile = getApplicationDirectory() + "/log.log";

        return logFile;
    }

    void runJobCommand(string command, Job job, NLogger *log)
    {
        command = Utils::sr(command, {
            {"#title#", "$title"},
            {"#url#", "$url"},
            {"#company#", "$company"},
            {"#logo#", "$logo"},
            {"#location#", "$location"},
            {"#category#", "$category"},
            {"#json#", "$json"},
            {"#jsonfile#", "$jsonfile"}
        });

        auto json = job.serialize();
        auto jsonNoFormat = job.serialize(false);
        auto tmpJsonFile = "/tmp/" + Utils::createUniqueId_customFormat("?????") + ".json";
        Utils::writeTextFileContent(tmpJsonFile, json);

        command = string("export LANG=C.UTF-8; ")+
                string("title=\"")+job.title+string("\"; ")+
                string("url=\"")+job.url+string("\"; ")+
                string("company=\"")+job.company+string("\"; ")+
                string("logo=\"")+job.company_logo_url+string("\"; ")+
                string("location=\"")+job.location+string("\"; ")+
                string("category=\"")+job.additionalInfo["category"].getString()+string("\"; ")+
                string("json=\"")+Utils::sr(jsonNoFormat, "\"", "\\\"")+string("\"; ")+
                string("jsonfile=\"")+tmpJsonFile+string("\"; ")+

                command;

        log->debug("running command: " + command);
        Utils::ssystem(command);
        Utils::ssystem("rm "+tmpJsonFile);
    }

    int displayHelp()
    {
        string text = string("") +
            string("Net-Empregos monitor. Job oportunities monitoring and command trigger.\n")+
            string("\n")+
            string("Usage: netempmon [options]\n")+
            string("\n")+
            string("Options: \n")+
            string("    -h, --help       Displays this help text\n")+
            string("\n")+
            string("    -u, --url        A net-empregos valid url with a job list.\n")+
            string("                     you can use a search url, or a category url.\n")+
            string("                     you can use this argument multiple times.\n")+
            string("                     If you do not specify any url, the system will work.\n")+
            string("                     with predefined ones. The predefined urls will look.\n")+
            string("                     look for only for T.I. jobs.\n")+
            string("\n")+
            string("    -c, --command    A cmd to be execute when a new is found. Some\n")+
            string("                     placeholders can be used on commmand execution:\n")+
            string("                         #title# -> is replaced by the job title\n")+
            string("                         #url# -> is replaced by the job url\n")+
            string("                         #company# -> is replaced by the company name\n")+
            string("                         #logo# -> is replaced by the url of the company logo\n")+
            string("                         #place# -> is replaced by the url of the place of job\n")+
            string("                         #category# -> is replaced by the url of the job category\n")+
            string("                         #json# -> is replace by a JSON with all job data\n")+
            string("                         #jsonfile# -> is replace for the address of a json file\n")+
            string("                                       that contains all job data\n")+
            string("\n")+
            string("    --log-level      Specify the output log level of the app. You can use:")+
            string("                         debug: all messages will be displayed\n")+
            string("                         info: Info, warnings and erros will be showed\n")+
            string("                         warning: Only warnings and erros will be displayed.\n")+
            string("                                  Note that the normal app output (info) will\n")+
            string("                                  be also supressed\n")+
            string("                         error: Only error messages will be displayed.\n")+
            string("                     This option only changes the terminal output (stdout). The \n")+
            string("                     log file ever will receive all outputs texts from the app");
            string("\n");
        cout << text << endl;
        return 0;
    }

};

int main(int argc, char** argv){
    App app;
    return app.run(argc, argv);
}


