#ifndef __cxdma_rw_synchronizer_h_
#define __cxdma_rw_synchronizer_h_

#include <cstdint>
#include <string>
#include "event.h"

class cxdma_rw_synchronizer
{
public:
	cxdma_rw_synchronizer() = default;

public:
	void wait_to_read()
	{
		_writer_event.wait();
	}

	bool wait_to_read(long ms)
	{
		return _writer_event.wait_for(ms);
	}

	void read_complete()
	{
		_reader_event.notify_one();
	}

	void wait_to_write()
	{
		_reader_event.wait();
	}

	bool wait_to_write(long ms)
	{
		return _reader_event.wait_for(ms);
	}

	void write_complete()
	{
		_writer_event.notify_one();
	}

private:
	cxdma_rw_synchronizer(const cxdma_rw_synchronizer &) = delete;
	cxdma_rw_synchronizer& operator = (const cxdma_rw_synchronizer&) = delete;

private:
	event _reader_event;
	event _writer_event;
};

#endif // !__cxdma_rw_synchronizer_h_