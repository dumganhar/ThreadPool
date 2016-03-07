//
//  CCThreadPool.hpp
//  cocos2d_libs
//
//  Created by James Chen on 3/3/16.
//
//

#ifndef CCThreadPool_hpp
#define CCThreadPool_hpp

#include "platform/CCPlatformMacros.h"

#include <functional>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <memory>

NS_CC_BEGIN

/**
 * @addtogroup base
 * @{
 */

class CC_DLL ThreadPool
{
public:
    static ThreadPool* getDefaultThreadPool();
    static void destroyDefaultThreadPool();
    
    ThreadPool(int minNum, int maxNum);
    
    // the destructor waits for all the functions in the queue to be finished
    ~ThreadPool();
    
    inline int getMinThreadNum() { return _minThreadNum; };
    inline int getMaxThreadNum() { return _maxThreadNum; };
    
    // number of idle threads
    int getIdleThreadNum();
    
    inline int getInitedThreadNum() { return _initedThreadNum; };

    void pushTask(const std::function<void(int)>& runnable);
    size_t getTaskNum();
    
    void setShrinkInterval(int seconds);
    void setShrinkStep(int step);
    void setStretchStep(int step);
    
    bool shrinkPool();
private:
    // deleted
    ThreadPool(const ThreadPool &);// = delete;
    ThreadPool(ThreadPool &&);// = delete;
    ThreadPool& operator=(const ThreadPool &);// = delete;
    ThreadPool& operator=(ThreadPool &&);// = delete;
    
    void init();
    void stop();
    void setThread(int i);

    void stretchPool(int count);

    // empty the queue
    void clearQueue();
    
    std::vector<std::unique_ptr<std::thread>> _threads;
    std::vector<std::shared_ptr<std::atomic<bool>>> _abortFlags;
    std::vector<std::shared_ptr<std::atomic<bool>>> _idleFlags;
    std::vector<std::shared_ptr<std::atomic<bool>>> _initedFlags;
    
    template <typename T>
    class ThreadSafeQueue {
    public:
        bool push(T const & value)
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            this->q.push(value);
            return true;
        }
        
        // deletes the retrieved element, do not use for non integral types
        bool pop(T & v)
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            if (this->q.empty())
                return false;
            v = this->q.front();
            this->q.pop();
            return true;
        }
        
        bool empty()
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            return this->q.empty();
        }
        
        size_t size()
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            return this->q.size();
        }
    private:
        std::queue<T> q;
        std::mutex mutex;
    };
    
    ThreadSafeQueue<std::function<void(int tid)> *> _taskQueue;
    std::atomic<bool> _isDone;
    std::atomic<bool> _isStop;
    std::atomic<int> _idleThreadNum;  // how many threads are waiting
    
    std::mutex _mutex;
    std::condition_variable _cv;
    
    int _minThreadNum;
    int _maxThreadNum;
    int _initedThreadNum;
    
    struct timeval _lastShrinkTime;
    float _shrinkInterval;
    int _shrinkStep;
    int _stretchStep;
};
    
// end of base group
/// @}

NS_CC_END


#endif /* CCThreadPool_hpp */
