#include  "textfiledb.h" 
 
TextFileDB::TextFileDB(string fileName) 
{
    this->fileName = fileName;
    load(); 

    thread th([&](){
        autoDumpWait.lock();
        int timeout = 5000000;
        while (autoDumpRunning)
        {
            timeout -= 1000;
            if (timeout <= 0)
            {
                timeout = 5000000;
                dump();
            }
            usleep(1000);

        }
        autoDumpWait.unlock();
    });

    th.detach();
} 
 
TextFileDB::~TextFileDB() 
{ 
    autoDumpRunning = false;
    autoDumpWait.lock();
    autoDumpWait.unlock();
    //finalDump;

    dump(); 
} 
 
void TextFileDB::dump()
{
    stringstream dt;
    for (auto &c: vars)
    {
        dt << Utils::stringReplace(c.first, ":", "$##") << ":" << Utils::stringReplace(c.second.getString(), ":", "$##") << "\n";
    }

    Utils::writeTextFileContent(fileName, dt.str());
}

void TextFileDB::load()
{
    string dt = Utils::readTextFileContent(fileName) + "\n";
    auto pos = dt.find("\n");
    while (pos != string::npos)
    {
        auto currLine = dt.substr(0, pos);
        dt = dt.substr(pos+1);

        if (auto varSepPos = currLine.find(":"); varSepPos != string::npos)
            vars[Utils::stringReplace(currLine.substr(0, varSepPos), "$##", ":")] = Utils::stringReplace(currLine.substr(varSepPos+1), "$##", ":");

        pos = dt.find("\n");
    }
}

DynamicVar TextFileDB::get(string name, DynamicVar defaultValue)
{
    if (vars.count(name) > 0)
        return vars[name];
    else 
        return defaultValue;
}

void TextFileDB::set(string name, DynamicVar value)
{
    this->vars[name] = value;
}