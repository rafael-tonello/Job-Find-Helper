#include <string>
#include <vector>


using namespace std;
class ArgParser
{
private:
    vector<string> args;

    string _lastError = "";

public:
    ArgParser(){};
    ArgParser(int c, char** p, bool usesCommandAsFirst = false)
    {
        int i = usesCommandAsFirst ? 0 : 1;
        for ( ; i < c; i++)
        {
            this->args.push_back(string(p[i]));
        }
    }

    ArgParser(vector<string> args, bool usesCommandAsFirst = false)
    {
        size_t i = usesCommandAsFirst ? 0 : 1;
        size_t c = args.size();
        for ( ; i < c; i++)
        {
            this->args.push_back(args[i]);
        }
    }

    size_t size(){
        return this->args.size();
    }

    string get(int index, string defaultValue = "")
    {
        _lastError = "";
        if (index >= 0 && index < this->args.size())
            return this->args[index];

        _lastError = "Argument not found by index";
        return defaultValue;
    }

    string get(string argPrefix, string defaultValue = "")
    {
        _lastError = "";
        
        auto results = getList({argPrefix});
        if (results.size() > 0)
            return results[0];

        this->_lastError =   "argument not found";
        return defaultValue;
    }

    vector<string> getList(vector<string> validArgPrefixes)
    {
        auto matchAnyPrefix = [&](string text){
            for (auto c: validArgPrefixes)
                if (c.find(text) == 0)
                    return c;

            return string("");
        };

        vector<string> result;
        _lastError = "";
        for (int i = 0; i < this->args.size(); i++)
        {
            auto c = this->args[i];
            if (auto match = matchAnyPrefix(c); match != "") // != string::npos)
            {
                if (match == c)
                {
                    //retuirn next argument 
                    if (i+1 < this->args.size())
                    {
                        result.push_back(this->args[i+1]);
                        i++;
                    }

                }
                else if (match.size() < c.size())
                {
                    auto tmp = c.substr(match.size());
                    if (tmp.size() > 0 && string(" :=").find(tmp[0]) != string::npos)
                        tmp = tmp.substr(1);
                    result.push_back(tmp);
                }
            }
        }

        return result;
    }

    

    bool contains(string argPrefix)
    {
        for (int i = 0; i < this->args.size(); i++)
        {
            auto c = this->args[i];
            if (c.find(argPrefix) == 0 /*!= string::npos*/)
            {
                return true;
            }
        }
        return false;
    }

    bool contains(vector<string> anyOfTheseArgs)
    {
        for (auto &c: anyOfTheseArgs)
            if (contains(c))
                return true;
        return false;
    }

    bool containsExact(string arg)
    {
        for (int i = 0; i < this->args.size(); i++)
        {
            auto c = this->args[i];
            if (c == arg)
            {
                return true;
            }
        }
        return false;
    }

    bool containsExact(vector<string> anyOfTheseArgs)
    {
        for (auto &c: anyOfTheseArgs)
            if (containsExact(c))
                return true;
        return false;
    }

    string getLastError(){
        return this->_lastError;
    }

};
