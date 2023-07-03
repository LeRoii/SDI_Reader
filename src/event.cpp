#include "event.h"

event::event(bool auto_reset) : _auto(auto_reset), _state(false)
{
}

void event::wait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cond.wait(lock, [this]() { return this->_state == true; });
    if (_auto)
    {
        _state = false;
    }
}
bool event::wait_for(long milliseconds)
{
    std::unique_lock<std::mutex> lock(_mutex);
    bool ret = _cond.wait_for(lock, std::chrono::milliseconds(milliseconds), [this]() { return this->_state == true; });
    if (ret && _auto)
    {
        _state = false;
    }
    return ret;
}

template <typename _Rep, typename _Period>
bool event::wait_for(const std::chrono::duration<_Rep, _Period>& duration)
{
    std::unique_lock<std::mutex> lock(_mutex);
    bool ret = _cond.wait_for(lock, duration, [this]() { return this->_state == true; });
    if (ret && _auto)
    {
        _state = false;
    }
    return ret;
}

template <typename _Clock, typename _Duration>
bool event::wait_until(const std::chrono::time_point<_Clock, _Duration>& point)
{
    std::unique_lock<std::mutex> lock(_mutex);
    bool ret = _cond.wait_until(lock, point, [this]() { return this->_state == true; });
    if (ret && _auto)
    {
        _state = false;
    }
    return ret;
}

void event::notify_one()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _state = true;
    _cond.notify_one();
}

void event::notify_all()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _state = true;
    _cond.notify_all();
}

void event::reset()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _state = false;
}