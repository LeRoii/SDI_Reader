#ifndef __event_h_
#define __event_h_

#include <mutex>
#include <condition_variable>

class event
{
public:
    event(bool auto_reset = true);

public:
    void wait();

    bool wait_for(long milliseconds);

    template <typename _Rep, typename _Period>
    bool wait_for(const std::chrono::duration<_Rep, _Period>& duration);

    template <typename _Clock, typename _Duration>
    bool wait_until(const std::chrono::time_point<_Clock, _Duration>& point);

    void notify_one();

    void notify_all();

    void reset();

private:
    event(const event&) = delete;
    event& operator = (const event&) = delete;

private:
    bool _auto;
    bool _state;

    std::mutex _mutex;
    std::condition_variable _cond;
};


#endif // !__event_h_
