#ifndef __TIME_COUNTER_H_
#define __TIME_COUNTER_H_

#include <chrono>
using namespace std::chrono;
class time_counter
{
public:
    time_counter() : _begin(steady_clock::now())
    {
    }

    void reset()
    {
        _begin = steady_clock::now();
    }

    double elapsed_sec() const
    {
        return duration_cast<duration<double>>(steady_clock::now() - _begin).count();
    }

    int64_t elapsed_msec() const
    {
        return duration_cast<milliseconds>(steady_clock::now() - _begin).count();
    }

    int64_t elapsed_usec() const
    {
        return duration_cast<microseconds>(steady_clock::now() - _begin).count();
    }

private:
    time_point<steady_clock> _begin;
};

#endif // __TIME_COUNTER_H_

