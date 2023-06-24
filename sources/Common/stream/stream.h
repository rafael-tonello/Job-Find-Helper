#ifndef __Common__STREAM__
#define __Common__STREAM__

#include <functional>
#include <vector>
#include <mutex>
#include <map>
#include <future>

using namespace std;
namespace Common{
    #define StreamObservingID uint

    template <typename T>
    /*A sincronous stream*/
    class Stream{
    private:
        StreamObservingID currId = 0;
        map<StreamObservingID, function<void(T data)>> listeners;
        mutex listenerMutex;
        T last;

        vector<T> startupCache;
    public:
        /* this function add an obsever tot he Stream. This observer will receive all data that is added via 'add' function
         * @param observer an function to be called whena  new data arrives in the stream. This function is am lambda with the signature [](T data){ }
         * @return this function returns an observer id, that can be used to stop observatin the stream
         */
        StreamObservingID listen(function<void(T data)> observer);
        /* Another way to listen data comming from the stream (redirects to 'listen' function) */
        StreamObservingID subscribe(function<void(T data)> f){ return listen(f); }

        /* This function adds data to the stream, i.e., all observer will receive this data
         * @param data is the data to be sent to all observers
         */
        void add(T data);

        /* Another way to add data to the stream (redirects to 'add' function) */
        void stream(T data);

        future<T> waitNext();

        /* Return the last received data */
        T get(); 

        /* Stops observating the stream
         * @param the id of the observer, previusly returned by 'listen' function
         */
        void stopListen(StreamObservingID id);

        /* Another way to stop lisening data comming from the stream (redirects to 'stopListen' function) */
        void unsubscribe(StreamObservingID subscribeId){ stopListen(subscribeId); }
    };
}

template <typename T>
StreamObservingID Common::Stream<T>::listen(function<void(T data)> observer)
{
    StreamObservingID ret = currId++;
    listenerMutex.lock();
    listeners[ret] = observer;
    listenerMutex.unlock();

    for (size_t c = 0; c < startupCache.size(); c++)
        observer(startupCache[c]);
    startupCache.clear();

    return ret;
}

template <typename T>
void Common::Stream<T>::stream(T data)
{
    last = data;
    listenerMutex.lock();
    if (this->listeners.size() > 0)
    {
        for (auto &curr : this->listeners)
            curr.second(data);
    }
    else
        startupCache.push_back(data);

    listenerMutex.unlock();
}

template <typename T>
void Common::Stream<T>::add(T data)
{
    this->stream(data);
}

template <typename T>
void Common::Stream<T>::stopListen(StreamObservingID id)
{
    listenerMutex.lock();
    listeners.erase(id);
    listenerMutex.unlock();;
}

template <typename T>
T Common::Stream<T>::get()
{
    return last;
}

template <typename T>
future<T> Common::Stream<T>::waitNext()
{
    shared_ptr<promise<T>> prom;
        
    StreamObservingID  id;
    id = this->listen([&](T data){
        this->stopListen(id);
        prom->set_value(data);
    });

    return prom->get_future();
}

#endif
