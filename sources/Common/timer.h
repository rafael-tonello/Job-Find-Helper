#ifndef __TIMER__H__ 
#define __TIMER__H__ 
 
#include <functional>
#include <map>
#include <thread>
#include <unistd.h>
#include <atomic>
#include <mutex>

using namespace std;

#define Timer_microseconds *1
#define Timer_milliseconds Timer_microseconds*1000
#define Timer_seconds Timer_milliseconds*1000
#define Timer_minutes Timer_seconds*60
#define Timer_hours Timer_minutes*60

#define TimerSleepInterval 100 Timer_microseconds

class Timer{
private:
    static int idCount;
    static map<int, Timer*> timers;
    static bool threadRuning;
    static mutex timersLocker;

    static void verifyTimersAndThread();
    int id = 0;
    int timeCount;
    int interval;
    atomic<bool> readyForNewCall;
public:
    function<void(Timer& t)> f = [&](Timer& t){};

    void init(int interval, function<void(Timer& t)>f, bool firstShotImediately = true, bool initStoped = false);
    Timer(int interval, function<void(Timer& t)>f, bool firstShotImediately = true);
    Timer();
    ~Timer();

    void stop();
    void start(bool firstShotImediately = true);
    void changeInterval(int interval, bool resetTimecount = false);

};
 
#endif 
