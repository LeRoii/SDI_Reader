#ifndef __cxdma_raw_receiver_h_
#define __cxdma_raw_receiver_h_

#include <list>
#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include <thread>
#include "cpthread.h"
#include "time_counter.h"
#include "cxdma_reader.h"
#include "cxdma_rw_synchronizer.h"
#include "CircleBuffer.h"
#include <opencv2/opencv.hpp>

class cxdma_receiver : public cpthread
{
public:
	cxdma_receiver(const std::string&, cxdma_rw_synchronizer&);

	cxdma_receiver(const std::string&&, cxdma_rw_synchronizer&);

	~cxdma_receiver();

	//启动
	void Start();
	
	//等待退出
	void Wait();
	
	//使用C++11标准类thread创建线程，注意需要gcc版本支持c++11	
	//从fpga板读取数据到环形缓冲区
	// void OnReadData();
	//分析环形缓冲区数据，并将视频数据截取出来进行显示, channel0
	void OnAnalysisData();
	void procImg();

	//分析环形缓冲区数据，并将视频数据截取出来进行显示, channel1
	void OnAnalysisData_1();

	void OnReadDataCh1();
	void OnReadDataCh0();

	void getFrame(cv::Mat &frame);
	void getFrame(cv::Mat &frame0, cv::Mat &frame1);

private:
	//使用标准c++11thread
	void run() override;

private:
	void readConfig();
	uint64_t rd_buf_size = 1024 * 1024 * 20;//读取缓冲区的大小

	cxdma_rw_synchronizer& _rw_syn;

	time_counter _t_counter;
	
	
	//循环缓冲区大小
	uint64_t m_Size;
	//环形缓冲区
	// CircleBuffer *m_CircleBuffer_ch0;
	// CircleBuffer *m_CircleBuffer_ch1;
	
	
	//声明线程对象
	std::thread r_thread0,r_thread1;
	// std::thread s_thread;
	// std::thread a_thread;
	
	// std::thread a_thread_1;

	//通道0 成员数据
    std::int8_t m_enabled_ch0;
	std::string _dev_name_ch0;
	cxdma_reader _xdma_reader_ch0;
	//通道0环形缓冲区
	CircleBuffer *m_CircleBuffer_ch0;
    //通道0图片数据队列
    // std::vector<cv::Mat> m_PicMat_Vec_ch0;
    //通道0，校验数据线程
	std::thread a_thread_ch0;
    //通道0，分辨率
    std::string m_resolution_ch0;
    //通道0，图片高度，根据分辨率计算
    std::uint64_t m_Pic_Height_ch0;
    //通道0，图片宽度，根据分辨率计算
    std::uint64_t m_Pic_Width_ch0;
    //通道0，图片数据量，根据分辨率计算
    std::uint64_t m_Pic_Capicity_ch0;
    //4M 4K字节对齐缓冲区
    uint8_t* rd_buf_ch0;
    //通道0图片数据临时保存缓冲区
    uint8_t* m_pic_buf_ch0;

    //通道1数据配置
    std::int8_t m_enabled_ch1;
	std::string _dev_name_ch1;
	cxdma_reader _xdma_reader_ch1;
	//通道1环形缓冲区
	CircleBuffer *m_CircleBuffer_ch1;
    //通道1图片数据队列
    // std::vector<cv::Mat> m_PicMat_Vec_ch1;
    //通道1，校验数据线程
	std::thread a_thread_ch1;
    //通道1，分辨率
    std::string m_resolution_ch1;
    //通道1，图片高度，根据分辨率计算
    std::uint64_t m_Pic_Height_ch1;
    //通道1，图片宽度，根据分辨率计算
    std::uint64_t m_Pic_Width_ch1;
    //通道1，图片数据量，根据分辨率计算
    std::uint64_t m_Pic_Capicity_ch1;
    //4M 4K字节对齐缓冲区
    uint8_t* rd_buf_ch1;
    //通道1图片数据临时保存缓冲区
    uint8_t* m_pic_buf_ch1;

	std::string m_cfgPath;
};

#endif /// !__cxdma_raw_receiver_h_
