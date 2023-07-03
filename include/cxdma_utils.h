#ifndef __cxdma_utils_h_
#define __cxdma_utils_h_

#include <stdint.h>
#include<sys/types.h>
/*
* man 2 write:
* On Linux, write() (and similar system calls) will transfer at most
* 	0x7ffff000 (2,147,479,552) bytes, returning the number of bytes
*	actually transferred.  (This is true on both 32-bit and 64-bit
*	systems.)
*/

#define RW_MAX_SIZE	0x7ffff000

#define INVALID_FD -1

class cxdma_utils
{
public:
	static ssize_t read_to_buffer(const char* fname, int fd, void* buffer, uint64_t size, uint64_t base);

	static ssize_t write_from_buffer(const char* fname, int fd, void* buffer, uint64_t size, uint64_t base);

private:
	cxdma_utils() = delete;
	cxdma_utils(const cxdma_utils& rhs) = delete;
	cxdma_utils(cxdma_utils&& rhs) = delete;
	cxdma_utils& operator=(const cxdma_utils& rhs) = delete;
	cxdma_utils& operator=(cxdma_utils&& rhs) = delete;
};

#endif // !__cxdma_utils_h_