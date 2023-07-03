#include "cpthread.h"
#include <cstdlib>

cpthread::cpthread()
    : _running(false)
    , _finished(false)
    , _interruption_requested(false)
    , _stacksize(0)
    , _priority(pthread_priority::inherit_priority)
{
}

cpthread::~cpthread()
{
    std::lock_guard<std::mutex> lck(_mutex);
    if (_running && !_finished)
    {
        request_interruption();
        terminate();
    }
}

void cpthread::start(pthread_priority priority)
{
    std::lock_guard<std::mutex> lck(_mutex);
    if (_running)
    {
        return;
    }

    _running = true;
    _finished = false;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    _priority = priority;
    switch (priority)
    {
    case inherit_priority:
        pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
        break;
    default:
        int sched_policy = SCHED_OTHER;
        if (pthread_attr_getschedpolicy(&attr, &sched_policy) != 0)
        {
            // failed to get the scheduling policy, don't bother
            // setting the priority QThread::start: Cannot determine default scheduler policy
            break;
        }
        sched_param sched_parm = { 0 };
        if (!sched_get_priority(priority, sched_policy, sched_parm.__sched_priority))
        {
            // failed to get the scheduling parameters, don't
            // bother setting the priority "QThread::start: Cannot determine scheduler priority range"
            break;
        }
        if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0
            || pthread_attr_setschedpolicy(&attr, sched_policy) != 0
            || pthread_attr_setschedparam(&attr, &sched_parm) != 0)
        {
            // could not set scheduling hints, fallback to inheriting them
            // we'll try again from inside the thread
            pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
            _priority = priority | priority_reset_flag;
        }
        break;
    }
    if (_stacksize > 0)
    {
        int code = pthread_attr_setstacksize(&attr, _stacksize);
        if (code)
        {
            // QThread::start: Thread stack size error
            // we failed to set the stacksize, and as the documentation states,
            // the thread will fail to run...
            _running = false;
            _finished = false;
            return;
        }
    }
    if (!_cpu_set.empty())
    {
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &_cpu_set);
    }
    pthread_t thread_id;
    int code = pthread_create(&thread_id, &attr, (__start_routine)&cpthread::exec, this);
    if (code == EPERM)
    {
        pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
        code = pthread_create(&thread_id, &attr, (__start_routine)&cpthread::exec, this);
    }
    _tid.store(reinterpret_cast<void*>(thread_id), std::memory_order_relaxed);

    pthread_attr_destroy(&attr);

    if (code)
    {
        //QThread::start: Thread creation error
        _running = false;
        _finished = false;
        _tid.store(nullptr, std::memory_order_relaxed);
    }
}

void cpthread::set_priority(pthread_priority priority)
{
    if (priority == pthread_priority::inherit_priority)
    {
        // setPriority: Argument cannot be InheritPriority
        return;
    }
    std::lock_guard<std::mutex> lck(_mutex);
    if (!_running)
    {
        //setPriority: Cannot set priority, thread is not running
        return;
    }
    _priority = priority;
    // copied from start() with a few modifications:
    int sched_policy = SCHED_OTHER;
    sched_param sched_parm = { 0 };
    if (pthread_getschedparam(reinterpret_cast<pthread_t>(_tid.load(std::memory_order_relaxed)), &sched_policy, &sched_parm) != 0)
    {
        // failed to get the scheduling policy, don't bother setting
        // the priority setPriority: Cannot get scheduler parameters
        return;
    }
    if (!sched_get_priority(priority, sched_policy, sched_parm.__sched_priority))
    {
        // failed to get the scheduling parameters, don't
        // bother setting the priority setPriority: Cannot determine scheduler priority range
        return;
    }
    int status = pthread_setschedparam(reinterpret_cast<pthread_t>(_tid.load(std::memory_order_relaxed)), sched_policy, &sched_parm);
    // were we trying to set to idle priority and failed?
    if (status == -1 && sched_policy == SCHED_IDLE && errno == EINVAL)
    {
        // reset to lowest priority possible
        pthread_getschedparam(reinterpret_cast<pthread_t>(_tid.load(std::memory_order_relaxed)), &sched_policy, &sched_parm);
        sched_parm.sched_priority = sched_get_priority_min(sched_policy);
        pthread_setschedparam(reinterpret_cast<pthread_t>(_tid.load(std::memory_order_relaxed)), sched_policy, &sched_parm);
    }
}

pthread_priority cpthread::priority() const
{
    std::lock_guard<std::mutex> lck(_mutex);
    // mask off the high bits that are used for flags
    return pthread_priority(_priority & 0xffff);
}

void cpthread::set_affinity_cpuset(uint32_t cpu)
{
    cpu_set_wapper cpuset(cpu);
    set_affinity_cpuset(cpuset);
}

void cpthread::set_affinity_cpuset(cpu_set_wapper& cpuset)
{
    std::lock_guard<std::mutex> lck(_mutex);
    if (!_running)
    {
        _cpu_set = cpuset;
        return;
    }
    int code = pthread_setaffinity_np(reinterpret_cast<pthread_t>(_tid.load(std::memory_order_relaxed)), sizeof(cpu_set_t), &cpuset);
    if (code == 0)
    {
        _cpu_set = cpuset;
    }
    else
    {
        _cpu_set.reset();
        pthread_getaffinity_np(reinterpret_cast<pthread_t>(_tid.load(std::memory_order_relaxed)), sizeof(cpu_set_t), &_cpu_set);
    }
}

const cpu_set_wapper& cpthread::affinity_cpuset() const
{
    std::lock_guard<std::mutex> lck(_mutex);
    return _cpu_set;
}

void cpthread::set_stacksize(uint32_t stacksize)
{
    std::lock_guard<std::mutex> lck(_mutex);
    if (!_running)
    {
        _stacksize = stacksize;
    }
}

uint32_t cpthread::stacksize() const
{
    std::lock_guard<std::mutex> lck(_mutex);
    return _stacksize;
}

bool cpthread::running() const
{
    std::lock_guard<std::mutex> lck(_mutex);
    return _running;
}

bool cpthread::finished() const
{
    std::lock_guard<std::mutex> lck(_mutex);
    return _finished;
}

void cpthread::request_interruption()
{
    std::lock_guard<std::mutex> lck(_mutex);
    if (!_running || _finished)
    {
        return;
    }
    _interruption_requested.store(true, std::memory_order_relaxed);
}

bool cpthread::interruption_requested() const
{
    // fast path: check that the flag is not set:
    if (!_interruption_requested.load(std::memory_order_relaxed))
    {
        return false;
    }
    // slow path: if the flag is set, take into account run status:
    std::lock_guard<std::mutex> lck(_mutex);
    return _running && !_finished;
}

void cpthread::terminate()
{
    std::lock_guard<std::mutex> lck(_mutex);
    if (!_tid.load(std::memory_order_relaxed))
    {
        return;
    }
    int code = pthread_cancel(reinterpret_cast<pthread_t>(_tid.load(std::memory_order_relaxed)));
    if (code)
    {
        //"QThread::start: Thread termination error
    }
}

void* cpthread::exec()
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_cleanup_push(cpthread::finish, (void*)this);
    {
        std::lock_guard<std::mutex> lck(_mutex);
        // do we need to reset the thread priority?
        if (_priority & priority_reset_flag)
        {
            set_priority(pthread_priority(_priority & ~priority_reset_flag));
        }
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_testcancel();

    run();

    pthread_cleanup_pop(1);

    return nullptr;
}

void cpthread::finish(void* arg)
{
    cpthread* thr = reinterpret_cast<cpthread*>(arg);
    std::lock_guard<std::mutex> lck(thr->_mutex);
    thr->_running = false;
    thr->_finished = true;
    thr->_tid.store(nullptr, std::memory_order_relaxed);
}

bool cpthread::sched_get_priority(pthread_priority priority, int& sched_policy, int& sched_prio)
{
    if (priority == pthread_priority::inherit_priority)
    {
        return false;
    }
    sched_prio = 0;
    if (priority == pthread_priority::idle_priority)
    {
        sched_policy = SCHED_IDLE;
        return true;
    }
    if (priority == pthread_priority::normal_priority)
    {
        sched_policy = SCHED_OTHER;
        return true;
    }
    int prio_min = -1;
    int prio_max = -1;
    int priority_lowest = -1;
    int priority_highest = -1;
    if (priority >= pthread_priority::rr_lowest_priority && priority <= pthread_priority::rr_highest_priority)
    {
        sched_policy = SCHED_RR;
        priority_lowest = pthread_priority::rr_lowest_priority;
        priority_highest = pthread_priority::rr_highest_priority;
    }
    else if (priority >= pthread_priority::fifo_lowest_priority && priority <= pthread_priority::fifo_highest_priority)
    {
        sched_policy = SCHED_FIFO;
        priority_lowest = pthread_priority::fifo_lowest_priority;
        priority_highest = pthread_priority::fifo_highest_priority;
    }
    if (priority_lowest == -1 || priority_highest == -1)
    {
        return false;
    }
    prio_min = sched_get_priority_min(sched_policy);
    prio_max = sched_get_priority_max(sched_policy);
    if (prio_min == -1 || prio_max == -1)
    {
        return false;
    }
    int prio = prio_min + ((priority - priority_lowest) * (prio_max - prio_min) / priority_highest);
    sched_prio = std::max(prio_min, std::min(prio_max, prio));
    return true;
}