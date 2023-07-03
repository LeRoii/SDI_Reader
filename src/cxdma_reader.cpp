#include "cxdma_reader.h"
#include "cxdma_utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

cxdma_reader::cxdma_reader() : _fpga_fd(INVALID_FD)
{
}

cxdma_reader::~cxdma_reader()
{
	xdma_close();
}

int32_t cxdma_reader::xdma_open(const std::string& dev_name)
{
	return xdma_open(dev_name.c_str());
}

int32_t cxdma_reader::xdma_open(const char* dev_name)
{
	_dev_name = dev_name;

	if (_fpga_fd != INVALID_FD)
	{
		return _fpga_fd;
	}
	//_fpga_fd = open(_dev_name.c_str(), O_RDWR | O_TRUNC);
	//_fpga_fd = open(_dev_name.c_str(), O_RDONLY | O_DSYNC);
	//_fpga_fd = open(_dev_name.c_str(), O_RDONLY | O_RSYNC);
	//_fpga_fd = open(_dev_name.c_str(), O_RDONLY | O_SYNC);
	//打开xdma_c2h设备，以读写模式、不阻塞、直接IO（零拷贝）
	//_fpga_fd = open(_dev_name.c_str(), O_RDWR | O_NONBLOCK | O_DIRECT);
	//打开xdma_c2h设备，以读写模式、不阻塞
	_fpga_fd = open(_dev_name.c_str(), O_RDWR | O_NONBLOCK);
	// _fpga_fd = open(_dev_name.c_str(), O_RDWR);
	printf("_fpga_fd:%d\n", _fpga_fd);
	if (_fpga_fd == INVALID_FD)
	{
		return -1;
	}
	return 0;
}

void cxdma_reader::xdma_close()
{
	if (_fpga_fd != INVALID_FD)
	{
		close(_fpga_fd);
		_fpga_fd = INVALID_FD;
	}
}


//从xdma_c2h设备读取数据到缓存，
//buffer用户态缓存
//size需要读取的字节数
//addr读取的ddr起始位置
ssize_t cxdma_reader::xdma_read(void* buffer, const uint64_t size, const uint64_t addr)
{
	if (_fpga_fd == INVALID_FD)
	{
		return -1;
	}
	ssize_t rc = cxdma_utils::read_to_buffer(_dev_name.c_str(), _fpga_fd, buffer, size, addr);
	return rc;
}


//获取FPGA设备描述符
const int32_t& cxdma_reader::xdma_getFD() const
{
	return _fpga_fd;
}