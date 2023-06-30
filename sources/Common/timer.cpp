#include  "timer.h" 

int Timer::idCount  = 1;
bool Timer::threadRuning = false;
map<int, Timer*> Timer::timers = {};
mutex Timer::timersLocker;
 
Timer::Timer(){};

Timer::Timer(int64_t interval, function<void(Timer& t)>f, bool firstShotImediately) 
{
    this->init(interval, f, firstShotImediately);
}

void Timer::init(int64_t interval, function<void(Timer& t)>f, bool firstShotImediately, bool initStoped)
{
    if (this->id <= 0)
        this->id = Timer::idCount++;

    this->interval = interval;
    this->f = f;
    this->readyForNewCall = true;

    if (initStoped)
        this->stop(); //ensures the timer is not in the active timers list (Timer::timers map)
    else
        this->start(firstShotImediately);

}

void Timer::start(bool firstShotImediately)
{
    //ensures timer id
    if (this->id <= 0)
        this->id = Timer::idCount++;

    timersLocker.lock();
    Timer::timers[this->id] = this;
    timersLocker.unlock();
    this->currentTimeCountStart = Utils::getCurrentTimeMicroseconds();
    if (firstShotImediately)
        this->currentTimeCountStart -= this->interval;
    Timer::verifyTimersAndThread();
}
 
Timer::~Timer() 
{
    this->stop();
}
 
void Timer::stop(){
    timersLocker.lock();
    if (Timer::timers.count(this->id))
        Timer::timers.erase(this->id);

    timersLocker.unlock();

    //verifyTimersAndThread();
};

void Timer::verifyTimersAndThread()
{
    if ((!Timer::threadRuning) && (Timer::timers.size() > 0))
    {
        Timer::threadRuning = true;
        thread th([&](){
            while (Timer::timers.size() > 0)
            {
                int64_t currentTime = Utils::getCurrentTimeMicroseconds();
                timersLocker.lock();
                for (auto &c: Timer::timers)
                {
                    if (currentTime - c.second->currentTimeCountStart >= c.second->interval)
                    {
                        c.second->currentTimeCountStart = currentTime;
                        if (c.second->readyForNewCall)
                        {
                            c.second->readyForNewCall = false;
                            thread th2([](Timer* tmrp){
                                tmrp->f(*(tmrp));
                                tmrp->readyForNewCall = true;
                            }, c.second);
                            th2.detach();
                        }
                    }
                }
                timersLocker.unlock();
                usleep(TimerSleepInterval);
            }
            threadRuning = false;
        });
        th.detach();
    }
}

void Timer::changeInterval(int64_t interval, bool resetTimecount)
{
    this->interval = interval;
    if (resetTimecount)
        this->currentTimeCountStart = Utils::getCurrentTimeMicroseconds();
}