#include <iostream>
#include <unistd.h>
#include <textfiledb.h>
#include <icachedb.h>
#include <logger.h>
#include <neservice.h>
#include <LoggerConsoleWriter.h>
#include <LoggerFileWriter.h>
#include <patriciadb.h>
#include <argparser.h>


using namespace std;

//semantic versioning
string INFO_VERSION = "0.1.0";

std::string getApplicationDirectory();
std::string determinteLogFile();

int displayHelp();
void runJobCommand(string command, NetEmpregos::NEJob job);
int main(int argc, char** argv){

    cout << "Net-Empregos monitor and trigger, version " << INFO_VERSION << endl;
    ArgParser ap(argc, argv);

    if (ap.containsExact( {string("-h"), string("--help")} ))
        return displayHelp();

    auto urls = ap.getList({"-u", "--url"});

    auto commands = ap.getList({"-c", "--command"});
    
    if (urls.size() > 0)
    {
        cout << "   Monitoring urls: " <<endl;
        for (auto &c: urls)
            cout << "       " << c << endl;
    }
    else
        cout << "   Monitoring default url:" << endl << "       https://www.net-empregos.com/emprego-informatica-programacao.asp" << endl;
    cout << "   Commands: " << endl;
    for (auto &c: commands)
        cout << "       " << c << endl;


    //ICacheDB *db= new TextFileDB(getApplicationDirectory() + "/db.txt");
    ICacheDB *db = new PatriciaDB(getApplicationDirectory() + "/db");
    ILogger *logger = new Logger({new LoggerConsoleWriter(), new LoggerFileWriter(determinteLogFile())}, false);

    NetEmpregos::NEService service(db, logger, urls);

    service.jobsStream.subscribe([commands](NetEmpregos::NEJob foundJob){
        cout << "New Job found on NetEmpregos.com: " << endl;
        cout << "    " << foundJob.title << " (by "<< foundJob.company<< ")" << endl;
        cout << "    " << "place of job: " << foundJob.place << endl;
        cout << "    " << "more info in: " << foundJob.url << endl << endl;

        thread th([foundJob, commands](){
            for (auto &command: commands)
                runJobCommand(command, foundJob);
        });

        th.detach();
    });                                                                     

    service.start();

    while (true)
        usleep(1000);

    
    return 0;
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

void runJobCommand(string command, NetEmpregos::NEJob job)
{
    command = Utils::sr(command, {
        {"#title#", "$title"},
        {"#url#", "$url"},
        {"#company#", "$company"},
        {"#logo#", "$logo"},
        {"#place#", "$place"},
        {"#category#", "$category"},
    });

    command = string("export LANG=C.UTF-8; ")+
            string("title=\"")+job.title+string("\"; ")+
            string("url=\"")+job.url+string("\"; ")+
            string("company=\"")+job.company+string("\"; ")+
            string("logo=\"")+job.company_logo_url+string("\"; ")+
            string("place=\"")+job.place+string("\"; ")+
            string("category=\"")+job.category+string("\"; ")+
            command;

    //cout << "running command: ----->" << command << endl;
    Utils::ssystem(command);
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
        string("\n")+
        string("    -c, --command    A cmd to be execute when a new is found. Some\n")+
        string("                     placeholders can be used on commmand execution:\n")+
        string("                         #title# -> is replaced by the job title\n")+
        string("                         #url# -> is replaced by the job url\n")+
        string("                         #company# -> is replaced by the company name\n")+
        string("                         #logo# -> is replaced by the url of the company logo\n")+
        string("                         #place# -> is replaced by the url of the place of job\n")+
        string("                         #category# -> is replaced by the url of the job category\n")+
        string("\n");
    cout << text << endl;
    return 0;
}
