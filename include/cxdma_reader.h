#ifndef __cxdma_reader_h_
#define __cxdma_reader_h_

#include <cstdint>
#include <string>

class cxdma_reader
{
public:
	cxdma_reader();

	~cxdma_reader();

public:
	int32_t xdma_open(const std::string& dev_name);

	int32_t xdma_open(const char* dev_name);

	void xdma_close();

	//从xdma_c2h设备读取数据到缓存，
	//buffer用户态缓存
	//size需要读取的字节数
	//addr读取的ddr起始位置
	ssize_t xdma_read(void* buffer, const uint64_t size, const uint64_t addr);
	
	//获取FPGA设备描述符
	const int32_t& xdma_getFD() const;

private:
	int32_t _fpga_fd;

	std::string _dev_name;
};

#endif // !__cxdma_reader_h_