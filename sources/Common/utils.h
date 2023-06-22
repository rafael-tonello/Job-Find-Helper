#ifndef __UTILS__H__
#define __UTILS__H__

#include <string>
#include <fstream>
#include <future>
#include <map>
#include <unistd.h>
#include <exception>
#include <ThreadPool.h>
#include <sys/wait.h>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cctype>

#include <timersForDebug.h>

using namespace std;
    using named_lock_f = function<void()>;
    
    class Utils{
    private:
    #ifdef D__TESTING__
    private: 
    #endif
        static mutex names_locks_mutexes_mutex;
        static map<string, timed_mutex*> names_locks_mutexes;
        static int idCount;
        static bool srandOk;

        static void runSRand();
        static void initializeProxyList();
        static void checkAndAddProxies(vector<string> proxyesAndPorts);


        static vector <string> validProxies;
        static char proxyListInitializedState;
    public:
        static void named_lock(string session_name, named_lock_f f, int timeout_ms = -1);
        static void named_lock_forceunlock(string session_name);
        static int64_t getCurrentTimeMicroseconds();
        static int64_t getCurrentTimeMilliseconds();
        static int64_t getCurrentTimeSeconds();

        static string StringToHex(string& input, size_t size);
        static string StringToHex(string& input);
        static string charVecToHex(char* data, size_t size);
        static string charVecToHex(const char* data, size_t size);
        static string getOnly(string source, string validChars);
        static string ssystem (string, bool removeTheLastLF = true);
        static future<string> asystem(string, bool removeTheLastLF = true);
        static future<string> httpGet(string url, map<string, string> headers = {});
        static future<string> httpPost(string url, string body, string contentType = "application/json", map<string, string> headers = {});
        static void process_mem_usage(double& vm_usage, double& resident_set);
        static double process_vm_usage();
        static double process_resident_usage();
        /**
         * @brief Create a Unique Id object
         * 
         * @param validChars valid chars in the new id
         * @param size the maximum of chars in thenew id
         * @param prefix a text to bet put in the start of the new id (in addition to 'size' generated chars)
         * @param sufix a text to bet put in the end of the new id (in addition to 'size' generated chars)
         * @param includeTimeStampAtBegining include a chrono millisseconds count since ephoc
         * @return string 
         */
        static string createUniqueId(string validChars = "abcedfghijklmnopqrsuvxywzABCDEFGHIJKLMNOPQRSTUVXYWZ", int size = 20, string prefix = "UID", string sufix = "", bool includeTimeStampAtBegining = true);
        /**
         * @brief Create a Unique Id object using the specified format (aaaahhh000)
         * 
         * @param format format of the new id. Use 'A' for a-z char, 'H' for hex (0-f) char, '0' to a number (0-9) or '?' to get any char (number or letter)
         * @param prefix a text to bet put in the start of the new id (in addition generated chars)
         * @param sufix a text to bet put in the end of the new id (in addition generated chars)
         * @return string 
         */
        static string createUniqueId_customFormat(string format, string prefix = "", string sufix = "");
        static string createUnidqueId_guidFormat();

        static string readTextFileContent(string fileName);
        static void writeTextFileContent(string fileName, string content);
        static void appendTextFileContent(string fileName, string content);

        

        // static string findAndReplaceAll(std::string data, std::string toSearch, std::string replaceStr);

        static string downloadWithRandomProxy(string url, string destFileName, int maxTries = 5, int connectionTimeout = 10);

        static string stringReplace(string source, string replace, string by);
        static string stringReplace(string source, vector<tuple<string, string>> replaceAndByTuples);
        static string sr(string source, string replace, string by){return stringReplace(source, replace, by);};
        static string sr(string source, vector<tuple<string, string>> replaceAndByTuples){return stringReplace(source, replaceAndByTuples);};
        
        static bool isNumber(string source);

        static map<void*, string> getANameDB;
        enum NameType{ALGORITHM_GENERATED, REAL_NAME_COMBINATION};
        static string getAName(int number, NameType typeOfName = ALGORITHM_GENERATED, int AlgoGenMaxSyllables = 3);
        static string getAName(void* p, NameType typeOfName = ALGORITHM_GENERATED, int AlgoGenMaxSyllables = 3);

        static string getNestedExceptionText(exception &e, string prefix ="", int level = 0);

        template <typename S, typename T>
        static vector<T> mapVector(vector<S> source, function<T(S)> f)
        {
            vector<T> ret;
            ret.resize(source.size());
            std::transform(source.begin(), source.end(), ret.begin(), f);
            return ret;
        }

        template <typename T>
        static vector<T> filterVector(vector<T> source, function<bool(T)> f)
        {
            vector<T> result;
            std::copy_if(source.begin(), source.end(), std::back_inserter(result), f);
            return result;
        }

        static string pickRandomProxy(bool checkProxy = false);



        template<typename T>
        static future<void> parallel_foreach(vector<T> items, function<void(T, void* additionalArgs)> f, ThreadPool *tasker, void* additionalArgs = NULL)
        {
            vector<future<void>> pendingTasks = {};
            for (auto &c: items)
            {
                pendingTasks.push_back(tasker->enqueue([&](T &item, void* argsp){
                    f(item, argsp);
                }, c, additionalArgs));
            };

            /*return tasker->enqueue([&](auto pendingTasks2){
                for (auto &c: pendingTasks2)
                    c.wait();
            }, pendingTasks);*/

            return tasker->enqueue([&](){
                for (auto &c: pendingTasks)
                    c.wait();
            });
        }

        static future<void> parallel_for(int from, int to, function<void(int)> f, ThreadPool * tasker)
        {
            vector<future<void>> pendingTasks;
            for (int c = from; c != to; from > to ? c-- : c++)
            {
                if (c != to)
                    pendingTasks.push_back(tasker->enqueue([&](int index){
                        f(index);
                    }, c));
            }

            return tasker->enqueue([&](){
                for (auto &c: pendingTasks)
                    c.wait();
            });
        }


    };

#endif
