#include "cxdma_receiver.h"
#include <chrono>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <signal.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <mutex>

#include "./common.h"

using namespace std;
using namespace std::chrono;

std::mutex mtx0, mtx1;
	

cv::Mat ret,ret_1, finalret, detret;

bool quit = false;

//Channel0
//保存图片数据
// uint8_t *t_pic;//= new uint8_t[PIXEL_SIZE_IN_BYTE*2];
int frameCnt = 0;


//Channel1
//保存图片数据
// uint8_t *t_pic_1= new uint8_t[PIXEL_SIZE_IN_BYTE*2];
int frameCnt_1 = 0;

static void signal_handle(int signum)
{
	quit = true;
	// _xdma_reader_ch0.xdma_close();
}

cxdma_receiver::cxdma_receiver(const string &cfgpath, cxdma_rw_synchronizer& rw_syn) : 
	_rw_syn(rw_syn), m_cfgPath(cfgpath)
	
{
	readConfig();

	struct sigaction sig_action;
	sig_action.sa_handler = signal_handle;
	sigemptyset(&sig_action.sa_mask);
	sig_action.sa_flags = 0;
	sigaction(SIGINT, &sig_action, NULL);

	//申请环形缓冲区
    //通道0使能
    if(this->m_enabled_ch0)
    {
	    m_CircleBuffer_ch0 = new CircleBuffer(m_Size);
        
        //通道0图片数据临时保存缓冲区
        m_pic_buf_ch0 = new uint8_t[this->m_Pic_Capicity_ch0];
        memset(m_pic_buf_ch0, 0 , this->m_Pic_Capicity_ch0);
       
        //申请堆内存缓冲区
        this->rd_buf_ch0 = (uint8_t*)malloc(sizeof(uint8_t)*(rd_buf_size + 4096));
        posix_memalign((void**)&this->rd_buf_ch0, 4096 /*alignment */, rd_buf_size + 4096);
        
        //初始化缓冲区数据
        memset(this->rd_buf_ch0, 0, rd_buf_size + 4096);
    }

    //通道1使能
    if(this->m_enabled_ch1)
    {
	    m_CircleBuffer_ch1 = new CircleBuffer(m_Size);
        
        //通道1图片数据临时保存缓冲区    
        m_pic_buf_ch1 = new uint8_t[this->m_Pic_Capicity_ch1];
        memset(m_pic_buf_ch1, 0 , this->m_Pic_Capicity_ch1); 

        this->rd_buf_ch1 = (uint8_t*)malloc(sizeof(uint8_t)*(rd_buf_size + 4096));
        posix_memalign((void**)&this->rd_buf_ch1, 4096 /*alignment */, rd_buf_size + 4096);
        
        //初始化缓冲区数据
        memset(this->rd_buf_ch1, 0, rd_buf_size + 4096);

    }

}

cxdma_receiver::cxdma_receiver(const string&& cfgpath, cxdma_rw_synchronizer& rw_syn): 
	cxdma_receiver(cfgpath, rw_syn)
{
}

cxdma_receiver::~cxdma_receiver()
{
	printf("cxdma_receiver destruct\n");
	// fclose(fout);
	_xdma_reader_ch0.xdma_close();

	_xdma_reader_ch1.xdma_close();
	
	delete m_CircleBuffer_ch0;
	delete m_CircleBuffer_ch1;

	printf("cxdma_receiver destruct end\n");
}

//读取配置文件信息
void cxdma_receiver::readConfig()
{
	NXProject::Common::GetInstance()->SetCfgPath(m_cfgPath);
	printf("m_cfgPathm_cfgPathm_cfgPath:%s\n", m_cfgPath.c_str());
    //从配置文件中读取信息
    char read_str[32]={0};
    //通道0使能信息
    if(NXProject::Common::GetInstance()->GetValueByKeyFromConfig("CHANNEL0_ENABLED", read_str, 32) != -1)
    {
        this->m_enabled_ch0 = std::stoi(read_str);
        std::cout<<"配置文件中，通道0使能信息："<<read_str<<std::endl;
    }
    else
    {
        this->m_enabled_ch0 = 1;//默认可以打开通道0
        printf("从配置文件 ./config/NXConfig.ini 中读取通道0使能信息出错\n请检查配置文件\n默认通道0使能值为1\n");
    }
    memset(read_str, 0,32);
    //通道1使能信息
    if(NXProject::Common::GetInstance()->GetValueByKeyFromConfig("CHANNEL1_ENABLED", read_str, 32) != -1)
    {
        this->m_enabled_ch1 = std::stoi(read_str);
        std::cout<<"配置文件中，通道1使能信息："<<read_str<<std::endl;
    }
    else
    {
        this->m_enabled_ch1 = 0;//默认不能打开通道1
        printf("从配置文件 ./config/NXConfig.ini 中读取通道1使能信息出错\n请检查配置文件\n默认通道1使能值为0\n");
    }
    memset(read_str, 0,32);
    //通道0设备文件名
    if(NXProject::Common::GetInstance()->GetValueByKeyFromConfig("CHANNEL0_NAME", read_str, 32) != -1)
    {
        this->_dev_name_ch0 = read_str;
        std::cout<<"配置文件中，通道0设备文件名信息："<<read_str<<std::endl;
    }
    else
    {
        this->_dev_name_ch0 = "/dev/xdma0_c2h_0";//通道0默认设备文件名
        printf("从配置文件 ./config/NXConfig.ini 中读取通道0设备文件名信息出错\n请检查配置文件\n默认通道0设备文件名为/dev/xdma0_c2h_0\n");
    }
    memset(read_str, 0,32);
    //通道1设备文件名
    if(NXProject::Common::GetInstance()->GetValueByKeyFromConfig("CHANNEL1_NAME", read_str, 32) != -1)
    {
        this->_dev_name_ch1 = read_str;
        std::cout<<"配置文件中，通道1设备文件名信息："<<read_str<<std::endl;
    }
    else
    {
        this->_dev_name_ch1 = "/dev/xdma0_c2h_1";//通道1默认设备文件名
        printf("从配置文件 ./config/NXConfig.ini 中读取通道1设备文件名信息出错\n请检查配置文件\n默认通道1设备文件名为/dev/xdma0_c2h_1\n");
    }
    memset(read_str, 0,32);
    //通道0，视频分辨率
    if(NXProject::Common::GetInstance()->GetValueByKeyFromConfig("CHANNEL0_RESOLUTION", read_str, 32) != -1)
    {
        this->m_resolution_ch0 = read_str;
        //根据分辨率计算图片数据信息，只计算分辨率，不计算频率
        //如果是1080P
        if(strncmp(read_str, "1080", 4) == 0)
        {
            this->m_Pic_Height_ch0 = 1080;
            this->m_Pic_Width_ch0 = 1920;
        }
        else if(strncmp(read_str, "720", 3) == 0)
        {
            this->m_Pic_Height_ch0 = 720;
            this->m_Pic_Width_ch0 = 1280;
        }
        else//默认1080P30
        {
            this->m_Pic_Height_ch0 = 1080;
            this->m_Pic_Width_ch0 = 1920;
        }

        this->m_Pic_Capicity_ch0 = this->m_Pic_Height_ch0 * this->m_Pic_Width_ch0 * 2;

        
        std::cout<<"配置文件中，通道0分辨率信息："<<read_str<<std::endl;

    }
    else
    {
        this->m_resolution_ch0 = "1080P30";//分辨率默认1080P30
        
        this->m_Pic_Height_ch0 = 1080;
        this->m_Pic_Width_ch0 = 1920;
        this->m_Pic_Capicity_ch0 = this->m_Pic_Height_ch0 * this->m_Pic_Width_ch0 * 2;
        printf("从配置文件./config/NXConfig.ini 中读取通道0视频分辨率信息出错\n请检查配置文件\n通道0使用默认分辨率1080P30\n");
    }
    memset(read_str, 0,32);
    //通道1，视频分辨率
    if(NXProject::Common::GetInstance()->GetValueByKeyFromConfig("CHANNEL1_RESOLUTION", read_str, 32) != -1)
    {
        this->m_resolution_ch1 = read_str;
        //根据分辨率计算图片数据信息，只计算分辨率，不计算频率
        //如果是1080P
        if(strncmp(read_str, "1080", 4) == 0)
        {
            this->m_Pic_Height_ch1 = 1080;
            this->m_Pic_Width_ch1 = 1920;
        }
        else if(strncmp(read_str, "720", 3) == 0)
        {
            this->m_Pic_Height_ch1 = 720;
            this->m_Pic_Width_ch1 = 1280;
        }
        else//默认1080P30
        {
            this->m_Pic_Height_ch1 = 1080;
            this->m_Pic_Width_ch1 = 1920;
        }

        this->m_Pic_Capicity_ch1 = this->m_Pic_Height_ch1 * this->m_Pic_Width_ch1 * 2;

        std::cout<<"配置文件中，通道1分辨率信息："<<read_str<<std::endl;
		

    }
    else
    {
        this->m_resolution_ch1 = "1080P30";//分辨率默认1080P30
        this->m_Pic_Height_ch1 = 1080;
        this->m_Pic_Width_ch1 = 1920;
        this->m_Pic_Capicity_ch1 = this->m_Pic_Height_ch1 * this->m_Pic_Width_ch1 * 2;
        printf("从配置文件./config/NXConfig.ini 中读取通道1视频分辨率信息出错\n请检查配置文件\n通道1使用默认分辨率1080P30\n");
    }
    memset(read_str, 0,32);
    //循环缓冲区容量，单位M
    if(NXProject::Common::GetInstance()->GetValueByKeyFromConfig("CIRCLEBUFFER_CAPACITY", read_str, 32) != -1)
    {
        this->m_Size = std::stoi(read_str) * 1024 * 1024;
        std::cout<<"配置文件中，循环缓冲区容量："<<read_str<<std::endl;
    }
    else
    {
        this->m_Size = 64 * 1024 * 1024;//默认使用64M容量大小的缓冲区
        printf("从配置文件./config/NXConfig.ini 中读取循环缓冲区容量信息出错\n请检查配置文件\n默认64M大小循环缓冲区大小\n");
    }
    memset(read_str, 0,32);
    //每次从pcie读取的数据量块大小
    if(NXProject::Common::GetInstance()->GetValueByKeyFromConfig("READ_BLOCKSIZE", read_str, 32) != -1)
    {
        this->rd_buf_size = std::stoi(read_str) * 1024 * 1024;
        std::cout<<"配置文件中，每次从pcie读取的数据量块大小："<<read_str<<std::endl;
    }
    else
    {
        this->rd_buf_size = 4 * 1024 * 1024;//默认每次从pcie读取4M数据
        printf("从配置文件./config/NXConfig.ini 中读取PCIe数据读取块大小信息出错\n请检查配置文件\n读取PCIe数据块的大小默认为4M\n");
    }
}



//启动
void cxdma_receiver::Start()
{
	//使用C++11标准类thread创建线程，注意需要gcc版本支持c++11

	//通道0使能
    if(this->m_enabled_ch0)
    {

		r_thread0 = std::thread(&cxdma_receiver::OnReadDataCh0, this);
		r_thread0.detach();//主线程与子线程分离，保证主线程结束不影响子线程
		printf("create Receive thread succeed\n");

        //创建分析数据的线程,channel0
        a_thread_ch0 = std::thread(&cxdma_receiver::OnAnalysisData, this);
        a_thread_ch0.detach();
        printf("create channel0 Analysis thread succeed\n");

    }

    //通道1使能
	if(this->m_enabled_ch1)
    {
		r_thread1 = std::thread(&cxdma_receiver::OnReadDataCh1, this);
		r_thread1.detach();//主线程与子线程分离，保证主线程结束不影响子线程
		printf("create Receive thread succeed\n");
        //创建分析数据的线程, channel1
        a_thread_ch1 = std::thread(&cxdma_receiver::OnAnalysisData_1, this);
        a_thread_ch1.detach();
        printf("create channel1 Analysis thread succeed\n");
    }

	ret_1 = cv::Mat(1080, 1920, CV_8UC3);
	ret = cv::Mat(1080, 1920, CV_8UC3);

	// cv::putText(ret_1, "NO INPUT", cv::Point(750, 520), cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(192,192,192), 3);
	// cv::putText(ret, "NO INPUT", cv::Point(750, 520), cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(192,192,192), 3);
}

//等待退出
void cxdma_receiver::Wait()
{
	//阻塞读取数据线程，等待线程结束
	// if(r_thread.joinable())
	// {
	// 	r_thread.join();
	// }

	sleep(2);

	cv::Mat finalret, detr;

	ret_1 = cv::Mat(1080, 1920, CV_8UC3);
	ret = cv::Mat(1080, 1920, CV_8UC3);

	cv::putText(ret_1, "NO INPUT", cv::Point(750, 520), cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(192,192,192), 3);
	cv::putText(ret, "NO INPUT", cv::Point(750, 520), cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(192,192,192), 3);

	while(1)
	{
		if(!ret.empty() && !ret_1.empty())
		{
			mtx0.lock();
			cv::resize(ret, ret, cv::Size(960,540));
			cv::resize(ret_1, ret_1, cv::Size(960,540));
			cv::vconcat(ret, ret_1, finalret);
			// printf("ret col:%d, row:%d\n", ret.cols, ret.rows);
			// ret.copyTo(ret_1(cv::Rect(0,0,960,540)));
			mtx0.unlock();

		}
		usleep(1000*30);
	}
	//阻塞分析线程，等待线程结束 fex 20230418 close Analysis thread
	//a_thread_ch0.join();
}

void cxdma_receiver::getFrame(cv::Mat &frame)
{
	cv::Mat finalret;
	mtx0.lock();
	// cv::resize(ret, ret, cv::Size(960,540));
	// cv::resize(ret_1, ret_1, cv::Size(960,540));
	cv::vconcat(ret, ret_1, finalret);

	
	// printf("ret col:%d, row:%d\n", ret.cols, ret.rows);
	// ret.copyTo(ret_1(cv::Rect(0,0,960,540)));
	mtx0.unlock();

	frame = finalret.clone();

}
void cxdma_receiver::getFrame(cv::Mat &frame0, cv::Mat &frame1)
{
	mtx0.lock();
	frame0 = ret.clone();
	frame1 = ret_1.clone();
	mtx0.unlock();
}

void cxdma_receiver::OnReadDataCh0()
{
	if(m_CircleBuffer_ch0 == NULL || m_CircleBuffer_ch0 == nullptr ||
		m_pic_buf_ch0 == NULL || m_pic_buf_ch0 == nullptr ||
		rd_buf_ch0 == NULL || rd_buf_ch0 == nullptr)
	{
		printf("cxdma_receiver readData end, where m_CircleBuffer is nullptr or NULL\n");
		return;
	}

	//打开xdma_c2h设备
	if(_xdma_reader_ch0.xdma_open(_dev_name_ch0) == -1)
	{
		std::cout<<"open device "<<_dev_name_ch0<<"fail"<<std::endl;
	}
	else
	{
		std::cout<<"open device "<<_dev_name_ch0<<"succeed"<<std::endl;
	}

	uint64_t lenOfValid;
	while ( !quit)
	{
		_t_counter.reset();
//		printf("before read\n");
		ssize_t rd_n = 0;
        //通道0使能
            rd_n = _xdma_reader_ch0.xdma_read(rd_buf_ch0, rd_buf_size, 0);
            //std::cout<<"cxdma_receiver _xdma_reader readData:"<<rd_n<<" bytes"<<std::endl;//fex 20230418 close printf
            usleep(1000*5);
            // continue;
            // std::cout<<_dev_name_ch0<<":";
            // printf("rcv_count:%lld   rd_n:%lld\n",rcv_count_ch0, rd_n);
            // rcv_count_ch0++;
            //printf("channel 0 read end, spend %d ms\n", _t_counter.elapsed_msec());
            _t_counter.reset();
            //printf("test 0\n");
            //std::cout<<"test 00000"<<endl;
            //fex 20230418 close read deal
            //读取的数据量>0
            lenOfValid = m_CircleBuffer_ch0->getLenOfValidData();
            printf("lenOfValid channel 0 :[%f]\n", (double)lenOfValid/m_Size);
            if (rd_n > 0)
            {
                //已经存入环形缓冲区的数据量
                uint64_t t_count = 0;
                //已存入环形缓冲区的数据量 < 本次从fpga读取的数据量
                while(t_count < rd_n && !quit)
                {
                    //本轮复制到环形缓冲区的数据量
                    uint64_t t_perCount = m_CircleBuffer_ch0->pushData(rd_buf_ch0+t_count, rd_n-t_count);
                    //计算本次已存入环形缓冲区的数据总量
                    t_count += t_perCount;
                    // printf("push :%d\n", t_perCount);
                    //
                    if(t_perCount < 1)
                    {
                        usleep(1);
                        std::cout<<"fullllll"<<endl;

                    }
                    //printf("%lld    %lld\n", t_count, rd_n);
                    // std::cout<<"test 111111"<<endl;
                }
                //printf("end pushData channel0\n");
            }
            else
            {
                printf("rd_n <0\n");
                _xdma_reader_ch0.xdma_close();
                sleep(5);
                printf("xdma_close\n");
                int ret = _xdma_reader_ch0.xdma_open(_dev_name_ch0);
                printf("reset, open:%d\n", ret);
            }
            //printf("channel 0 end, spend %d ms\n", _t_counter.elapsed_msec());
            
            usleep(1);
            //continue;
	}

}

void cxdma_receiver::OnReadDataCh1()
{
	if(m_CircleBuffer_ch1 == NULL || m_CircleBuffer_ch1 == nullptr ||
		m_pic_buf_ch1 == NULL || m_pic_buf_ch1 == nullptr ||
		rd_buf_ch1 == NULL || rd_buf_ch1 == nullptr)
	{
		printf("cxdma_receiver readData end, where m_CircleBuffer_1 is nullptr or NULL\n");
		return;
	}

	//打开xdma_c2h设备
	if(_xdma_reader_ch1.xdma_open(_dev_name_ch1) == -1)
	{
		std::cout<<"open device "<<_dev_name_ch1<<" fail"<<std::endl;
	}
	else
	{
		std::cout<<"open device "<<_dev_name_ch1<<" succeed"<<std::endl;
	}

		uint64_t lenOfValid;
	while ( !quit)
	{
		_t_counter.reset();
//		printf("before read\n");
		ssize_t rd_n = 0;
        //通道0使能
            rd_n = _xdma_reader_ch1.xdma_read(rd_buf_ch1, rd_buf_size, 0);
            //std::cout<<"cxdma_receiver _xdma_reader readData:"<<rd_n<<" bytes"<<std::endl;//fex 20230418 close printf
            usleep(1000*5);
            // continue;
            // std::cout<<_dev_name_ch0<<":";
            // printf("rcv_count:%lld   rd_n:%lld\n",rcv_count_ch0, rd_n);
            // rcv_count_ch0++;
            //printf("channel 0 read end, spend %d ms\n", _t_counter.elapsed_msec());
            _t_counter.reset();
            //printf("test 0\n");
            //std::cout<<"test 00000"<<endl;
            //fex 20230418 close read deal
            //读取的数据量>0
            lenOfValid = m_CircleBuffer_ch1->getLenOfValidData();
            printf("lenOfValid channel 0 :[%f]\n", (double)lenOfValid/m_Size);
            if (rd_n > 0)
            {
                //已经存入环形缓冲区的数据量
                uint64_t t_count = 0;
                //已存入环形缓冲区的数据量 < 本次从fpga读取的数据量
                while(t_count < rd_n && !quit)
                {
                    //本轮复制到环形缓冲区的数据量
                    uint64_t t_perCount = m_CircleBuffer_ch1->pushData(rd_buf_ch1+t_count, rd_n-t_count);
                    //计算本次已存入环形缓冲区的数据总量
                    t_count += t_perCount;
                    // printf("push :%d\n", t_perCount);
                    //
                    if(t_perCount < 1)
                    {
                        usleep(1);
                        std::cout<<"fullllll"<<endl;

                    }
                    //printf("%lld    %lld\n", t_count, rd_n);
                    // std::cout<<"test 111111"<<endl;
                }
                //printf("end pushData channel0\n");
            }
            else
            {
                printf("rd_n <0\n");
                _xdma_reader_ch1.xdma_close();
                sleep(5);
                printf("xdma_close\n");
                int ret = _xdma_reader_ch1.xdma_open(_dev_name_ch1);
                printf("reset, open:%d\n", ret);
            }
            //printf("channel 0 end, spend %d ms\n", _t_counter.elapsed_msec());
            
            usleep(1);
            //continue;
	}
}

void cxdma_receiver::procImg()
{
}


//分析环形缓冲区数据，并将视频数据截取出来进行显示
void cxdma_receiver::OnAnalysisData()
{
	int PIXEL_SIZE_IN_BYTE_1080 = 1920*1080;
	// int PIXEL_SIZE_IN_BYTE_1080 = 1280*720;
	printf("cxdma_receiver analysisData start\n");
	
	if(m_CircleBuffer_ch0 == NULL || m_CircleBuffer_ch0 == nullptr)
	{
		printf("cxdma_receiver analysisData end, where m_CircleBuffer_ch0 is nullptr or NULL\n");
		return;
	}
	//获取环形缓冲区buffer地址
	const uint8_t * t_buffer = m_CircleBuffer_ch0->getBuffer();
	//判断环形缓冲区地址位置为空
	if(!t_buffer)
	{
		std::cout<<"cxdma_receiver analysisData circle buffer is NULL \n"<<endl;
		return;
	}
	
	//保存图片数据
	// uint8_t *t_pic= new uint8_t[PIXEL_SIZE_IN_BYTE_1080*2];
	
	uint64_t errorCount = 0;
	uint64_t position=0;
	cv::Mat t_yuvimg = cv::Mat(this->m_Pic_Height_ch0, this->m_Pic_Width_ch0, CV_8UC2);

	
	//环形缓冲区不是空指针时，开始处理环形缓冲区数据
	while(!quit)
	{
		

		//找到图片头位置
		uint64_t lenOfValid = m_CircleBuffer_ch0->getLenOfValidData();
		// printf("cxdma_receiver analysisData lenOfValid:%ld\n", lenOfValid);
		//环形缓冲区有效数据量大于0
		while(lenOfValid >= 4  && !quit)
		{
			// lenOfValid = m_CircleBuffer_ch0->getLenOfValidData();
			// if(lenOfValid > 0)
			// {
			// 	m_CircleBuffer_ch0->popData(m_CircleBuffer_ch0->getLenOfValidData());
			// 	continue;
			// }
			// printf("cxdma_receiver analysisData lenOfValid:%ld\n", lenOfValid);
			//获取第一个有效数据位置
			const uint64_t validPos = m_CircleBuffer_ch0->getValidPosition();
			//如果当前连续四个字节是0x12345678，则有可能是图片头
			if(t_buffer[validPos%m_Size] == 0x12 && t_buffer[(validPos+1)%m_Size] == 0x34 
				&& t_buffer[(validPos+2)%m_Size] == 0x56 && t_buffer[(validPos+3)%m_Size] ==0x78)
			{
				// stepOfPosition = 0;
				// std::cout<<"cxdma_receiver analysisData,"<<"current validPos:"<<validPos<<",got 0x12345678"<<endl;
				
				//有效数据总量 大于 4+图片量+4
				while((lenOfValid = m_CircleBuffer_ch0->getLenOfValidData()) < 8+m_Pic_Capicity_ch0 && !quit)
				{
					// std::cout<<"cxdma_receiver analysisData,m_CircleBuffer_ch0->getLenOfValidData():"<<m_CircleBuffer_ch0->getLenOfValidData()
					// 		 <<", 小于"<<8+PIXEL_SIZE_IN_BYTE*2<<" bytes"<<endl;
					//等待读取足够的有效数据
					usleep(1000*1);
				}
				//寻找下一个图片头，刚好在计算的固定位置找到，说明当前图片头为真正的图片头数据
				const uint64_t t_pos = validPos + 4 + m_Pic_Capicity_ch0; 
				if(t_buffer[(t_pos)%m_Size] == 0x12 && t_buffer[(t_pos+1)%m_Size] == 0x34 
					&& t_buffer[(t_pos+2)%m_Size] == 0x56 && t_buffer[(t_pos+3)%m_Size] == 0x78)
				{
					
					// std::cout<<"cxdma_receiver analysisData, got 0x12345678  start show picture!"<<endl;
					
					//环形缓冲区先弹出图片四个头字节
					m_CircleBuffer_ch0->popData(4);
					//position+=4;
					lenOfValid-=4;
					// stepOfPosition += 4;
					
					//复制图片数据到线性数据区
					m_CircleBuffer_ch0->readData(m_pic_buf_ch0, m_Pic_Capicity_ch0);
					
					
					//环形缓冲区弹出图片数据
					m_CircleBuffer_ch0->popData(m_Pic_Capicity_ch0);
					//position+=PIXEL_SIZE_IN_BYTE*2;
					lenOfValid-=m_Pic_Capicity_ch0;
					// stepOfPosition += PIXEL_SIZE_IN_BYTE_1080*2;
					
					
					//使用openCV库显示图片数据到界面
					memcpy(t_yuvimg.data, m_pic_buf_ch0, m_Pic_Capicity_ch0);
					mtx0.lock();
					cv::cvtColor(t_yuvimg, ret, cv::COLOR_YUV2RGB_YUYV);
					mtx0.unlock();
					
					std::cout<<"channel0  ok:"<<",frameCnt:"<<frameCnt++<<", errCnt:"<<errorCount<<endl;
					
					continue;
				}
				//寻找下一个图片头，未在计算的固定位置找到，说明当前头出错
				else
				{
					
					//std::cout<<"cxdma_receiver analysisData, got 0x12345678 but can`t show picture!"<<endl;
					//std::cout<<position<<","<<stepOfPosition<<endl;
					std::cout<<"channel0 err header"<<endl;
					//环形缓冲区先弹出图片四个头字节
					m_CircleBuffer_ch0->popData(4);
					//position+=4;
					// stepOfPosition=4;
					lenOfValid-=4;
					errorCount++;
					continue;
				}
			}
			//如果当前position开始连续的四个字节不是0x12345678，则弹出一个元素
			else
			{
				// std::cout<<"cxdma_receiver analysisData,"<<"current validPos:"<<validPos<<",don`t get 0x12345678"<<endl;
				//删除一个元素
				m_CircleBuffer_ch0->popData(1);
				//position+=1;
				// stepOfPosition+=1;
				lenOfValid-=1;
				if(m_CircleBuffer_ch0->getLenOfValidData() > (float)0.8*m_Size)
				{
					m_CircleBuffer_ch0->isClear();
					break;
				}
			}
		}
	}
	
	printf("cxdma_receiver analysisData end\n");
}

//分析环形缓冲区数据，并将视频数据截取出来进行显示,channel1
void cxdma_receiver::OnAnalysisData_1()
{
	printf("cxdma_receiver analysisData_1 start\n");
	
	if(m_CircleBuffer_ch1 == NULL || m_CircleBuffer_ch1 == nullptr)
	{
		printf("cxdma_receiver analysisData_1 end, where m_CircleBuffer_ch1 is nullptr or NULL\n");
		return;
	}

	// while(!quit)
	// {
	// 	if(this->m_CircleBuffer_ch1->getLenOfValidData()>0)
	// 	{
	// 		this->m_CircleBuffer_ch1->isClear();
	// 	}

	// 	usleep(50);

	// }

	// return;


	//获取环形缓冲区buffer地址
	const uint8_t * t_buffer = m_CircleBuffer_ch1->getBuffer();
	//判断环形缓冲区地址位置为空
	if(!t_buffer)
	{
		std::cout<<"cxdma_receiver analysisData_1 circle buffer is NULL \n"<<endl;
		return;
	}
	
	//保存图片数据
	// uint8_t *t_pic= new uint8_t[PIXEL_SIZE_IN_BYTE*2];
	
	uint64_t errorCount = 0;
	uint64_t position=0;
	// uint64_t stepOfPosition=0;

	cv::Mat t_yuvimg = cv::Mat(this->m_Pic_Height_ch1, this->m_Pic_Width_ch1, CV_8UC2);
	
	//环形缓冲区不是空指针时，开始处理环形缓冲区数据
	while(!quit)
	{
		

		//找到图片头位置
		bool bFlag = false;
		uint64_t lenOfValid = m_CircleBuffer_ch1->getLenOfValidData();
		// printf("cxdma_receiver analysisData lenOfValid:%ld\n", lenOfValid);
		//环形缓冲区有效数据量大于0
		while(lenOfValid >= 4  && !quit)
		{
			//获取第一个有效数据位置
			const uint64_t validPos = m_CircleBuffer_ch1->getValidPosition();
			//如果当前连续四个字节是0x12345678，则有可能是图片头
			if(t_buffer[validPos%m_Size] == 0x12 && t_buffer[(validPos+1)%m_Size] == 0x34 
				&& t_buffer[(validPos+2)%m_Size] == 0x56 && t_buffer[(validPos+3)%m_Size] ==0x78)
			{
				// stepOfPosition = 0;
				// std::cout<<"cxdma_receiver analysisData,"<<"current validPos:"<<validPos<<",got 0x12345678"<<endl;
				
				//有效数据总量 大于 4+图片量+4
				while((lenOfValid = m_CircleBuffer_ch1->getLenOfValidData()) < 8+m_Pic_Capicity_ch1 && !quit)
				{
					// std::cout<<"cxdma_receiver analysisData,m_CircleBuffer_ch1->getLenOfValidData():"<<m_CircleBuffer_ch1->getLenOfValidData()
					// 		 <<", 小于"<<8+PIXEL_SIZE_IN_BYTE*2<<" bytes"<<endl;
					//等待读取足够的有效数据
					usleep(1000*1);
				}
				//寻找下一个图片头，刚好在计算的固定位置找到，说明当前图片头为真正的图片头数据
				const uint64_t t_pos = validPos + 4 + m_Pic_Capicity_ch1; 
				if(t_buffer[(t_pos)%m_Size] == 0x12 && t_buffer[(t_pos+1)%m_Size] == 0x34 
					&& t_buffer[(t_pos+2)%m_Size] == 0x56 && t_buffer[(t_pos+3)%m_Size] == 0x78)
				{
					
					// std::cout<<"cxdma_receiver analysisData, got 0x12345678  start show picture!"<<endl;
					
					//环形缓冲区先弹出图片四个头字节
					m_CircleBuffer_ch1->popData(4);
					//position+=4;
					lenOfValid-=4;
					// stepOfPosition += 4;
					
					//复制图片数据到线性数据区
					m_CircleBuffer_ch1->readData(m_pic_buf_ch1, m_Pic_Capicity_ch1);
					
					
					//环形缓冲区弹出图片数据
					m_CircleBuffer_ch1->popData(m_Pic_Capicity_ch1);
					//position+=PIXEL_SIZE_IN_BYTE*2;
					lenOfValid-=m_Pic_Capicity_ch1;
					// stepOfPosition += PIXEL_SIZE_IN_BYTE*2;
					
					
					//使用openCV库显示图片数据到界面
					memcpy(t_yuvimg.data, m_pic_buf_ch1, m_Pic_Capicity_ch1);
					
					mtx0.lock();
					cv::cvtColor(t_yuvimg, ret_1, cv::COLOR_YUV2RGB_YUYV);
					mtx0.unlock();
			
					std::cout<<"channel1  ok:"<<",frameCnt_1:"<<frameCnt_1++<<", errCnt_1:"<<errorCount<<endl;
					//环形缓冲区先弹出图片四个头字节,
					
					continue;
				}
				//寻找下一个图片头，未在计算的固定位置找到，说明当前头出错
				else
				{
					
					//std::cout<<"cxdma_receiver analysisData, got 0x12345678 but can`t show picture!"<<endl;
					//std::cout<<position<<","<<stepOfPosition<<endl;
					std::cout<<"channel1 err header"<<endl;
					//环形缓冲区先弹出图片四个头字节
					m_CircleBuffer_ch1->popData(4);
					//position+=4;
					// stepOfPosition=4;
					lenOfValid-=4;
					errorCount++;
					continue;
				}
				
				//逐个字节去查找图片头0x1234568
				/*if(stepOfPosition != PIXEL_SIZE_IN_BYTE*2+4)
				{
					std::cout<<position<<","<<stepOfPosition<<endl;
				}
				//环形缓冲区先弹出图片四个头字节
				m_CircleBuffer_ch1->popData(4);
				position+=4;
				stepOfPosition=4;
				lenOfValid-=4;*/
				
			}
			//如果当前position开始连续的四个字节不是0x12345678，则弹出一个元素
			else
			{
				// std::cout<<"cxdma_receiver analysisData,"<<"current validPos:"<<validPos<<",don`t get 0x12345678"<<endl;
				//删除一个元素
				m_CircleBuffer_ch1->popData(1);
				//position+=1;
				// stepOfPosition+=1;
				lenOfValid-=1;
				if(m_CircleBuffer_ch1->getLenOfValidData() > (float)0.8*m_Size)
				{
					m_CircleBuffer_ch1->isClear();
					break;
				}
			}
		}
	}
					
	printf("cxdma_receiver analysisData_1 end\n");
}


void cxdma_receiver::run()
{
	
}