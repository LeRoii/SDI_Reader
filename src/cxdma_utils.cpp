#include "cxdma_utils.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <chrono>
#include <errno.h>

ssize_t cxdma_utils::read_to_buffer(const char* fname, int fd, void* buffer, uint64_t size, uint64_t base)
{
	ssize_t rc;
	uint64_t count = 0;
	char* buf = (char*)buffer;
	off_t offset = base;
	int loop = 0;

	while (count < size) {
		uint64_t bytes = size - count;

		if (bytes > RW_MAX_SIZE)
			bytes = RW_MAX_SIZE;

		if (offset) {
			rc = lseek(fd, offset, SEEK_SET);
			if (rc != offset) {
				fprintf(stderr, "%s, seek off 0x%lx != 0x%lx.\n",
					fname, rc, offset);
				perror("seek file");
				return -EIO;
			}
		}

		/* read data from file into memory buffer */
		// std::cout<<"read_to_buffer before read:timestamp:"<<std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()<<std::endl;

		rc = read(fd, buf, bytes);
		
		if (rc < 0) {
			fprintf(stderr, "%s, read 0x%lx @ 0x%lx failed %ld.\n",
				fname, bytes, offset, rc);
			printf("errno is :%d\n", errno);
			perror("read file");
			return -EIO;
		}

		count += rc;
		if (rc != bytes) {
			fprintf(stderr, "%s, read underflow 0x%lx/0x%lx @ 0x%lx.\n",fname, rc, bytes, offset);
			break;
		}

		buf += bytes;
		offset += bytes;
		loop++;
	}

	if (count != size && loop)
		fprintf(stderr, "%s, read underflow 0x%lx/0x%lx.\n",fname, count, size);
	
	// std::cout<<"read_to_buffer endtimestamp:"<<std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()<<std::endl;

	return count;
}

ssize_t cxdma_utils::write_from_buffer(const char* fname, int fd, void* buffer, uint64_t size, uint64_t base)
{
	ssize_t rc;
	uint64_t count = 0;
	char* buf = (char*)buffer;
	off_t offset = base;
	int loop = 0;

	while (count < size) {
		uint64_t bytes = size - count;

		if (bytes > RW_MAX_SIZE)
			bytes = RW_MAX_SIZE;

		if (offset) {
			rc = lseek(fd, offset, SEEK_SET);
			if (rc != offset) {
				fprintf(stderr, "%s, seek off 0x%lx != 0x%lx.\n",
					fname, rc, offset);
				perror("seek file");
				return -EIO;
			}
		}

		/* write data to file from memory buffer */
		rc = write(fd, buf, bytes);
		if (rc < 0) {
			fprintf(stderr, "%s, write 0x%lx @ 0x%lx failed %ld.\n",
				fname, bytes, offset, rc);
			perror("write file");
			return -EIO;
		}

		count += rc;
		if (rc != bytes) {
			fprintf(stderr, "%s, write underflow 0x%lx/0x%lx @ 0x%lx.\n",
				fname, rc, bytes, offset);
			break;
		}
		buf += bytes;
		offset += bytes;

		loop++;
	}

	if (count != size && loop)
		fprintf(stderr, "%s, write underflow 0x%lx/0x%lx.\n",
			fname, count, size);

	return count;
}
