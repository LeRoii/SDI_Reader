#ifndef __PTHREAD__H_
#define __PTHREAD__H_

#include <pthread.h>
#include <cstdint>
#include <stdlib.h>
#include <errno.h>
#include <cmath>
#include <utility>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>

typedef void* __thread_arg;
typedef void* (*__start_routine)(void*);//����FUNC������һ��ָ������ָ�룬�ú�������Ϊvoid*������ֵΪvoid*  

enum { priority_reset_flag = 0x80000000 };

enum pthread_priority
{
	inherit_priority,
	idle_priority,
	normal_priority,

	rr_lowest_priority,
	rr_low_priority,
	rr_normal_priority,
	rr_high_priority,
	rr_highest_priority,

	fifo_lowest_priority,
	fifo_low_priority,
	fifo_normal_priority,
	fifo_high_priority,
	fifo_highest_priority,
};

class cpu_set_wapper : public cpu_set_t
{
public:
	cpu_set_wapper()
	{
		CPU_ZERO(this);
	}

	explicit cpu_set_wapper(const uint32_t cpu)
	{
		CPU_ZERO(this);
		CPU_SET(cpu, this);
	}

public:
	void reset()
	{
		CPU_ZERO(this);
	}

	void set_cpu(const int cpu)
	{
		CPU_SET(cpu, this);
	}

	void clr_cpu(const int cpu)
	{
		CPU_CLR(cpu, this);
	}

	bool cpu_isset(const int cpu) const
	{
		return CPU_ISSET(cpu, this) != 0;
	}

	int cpu_count() const
	{
		return CPU_COUNT(this);
	}

	bool empty() const
	{
		return  CPU_COUNT(this) == 0;
	}

	std::vector<uint32_t> cpu_array()
	{ 
		std::vector<uint32_t> cpuarray(CPU_COUNT(this));
		for (int i= 0; i < CPU_SETSIZE; i++)
		{
			if (CPU_ISSET(i, this))
			{
				cpuarray.push_back(i);
			}
		}
		return cpuarray;
	}
};

class cpthread
{
public:
	cpthread();
	~cpthread();

public:
	void start(pthread_priority = inherit_priority);
	void terminate();

	void set_priority(pthread_priority priority);
	pthread_priority priority() const;

	void set_affinity_cpuset(uint32_t cpu);
	void set_affinity_cpuset(cpu_set_wapper& cpuset);
	const cpu_set_wapper& affinity_cpuset() const;

	void set_stacksize(uint32_t stacksize);
	uint stacksize() const;

	bool running() const;

	bool finished() const;

	void request_interruption();

	bool interruption_requested() const;
	
public:
	void* exec();
	virtual void run() = 0;
	
private:
	mutable std::mutex _mutex;

	std::atomic<void*> _tid;

	uint32_t _stacksize;

	//std::underlying_type_t<pthread_priority> _priority;
	int _priority;

	cpu_set_wapper _cpu_set;

	bool _running;
	bool _finished;
	std::atomic<bool> _interruption_requested;

private:
	static void finish(void*);

public:
	static bool sched_get_priority(pthread_priority priority, int& sched_policy, int& sched_prio);
};
#endif // __PTHREAD__H_