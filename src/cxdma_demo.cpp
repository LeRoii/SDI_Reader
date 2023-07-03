// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include <unistd.h>
// #include <fcntl.h>

// #include <math.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
#include <unistd.h>
#include <signal.h>
// #include <signal.h>
// #include <sys/wait.h>
// #include <sstream>
// #include <cstdio>

// #include "cxdma_rw_synchronizer.h"
// #include "cxdma_receiver.h"
// #include <opencv2/opencv.hpp>

#include "sdireader.h"

void load_driver()
{
    std::ostringstream ostr;
    ostr << "sudo /home/nx/0420/xdma_cmake/load_driver.sh 4 " << " >/dev/null 2>&1";
    system(ostr.str().c_str());
}

// int main()
// {
// 	idetector *detector = nullptr;
// 	detector = new idetector("/home/nx/code/yolov5/build/visd.engine");
// 	cv::VideoCapture cap;
// 	cv::VideoWriter writer;
// 	writer.open("det.avi", CV_FOURCC('M', 'J', 'P', 'G'), 20, cv::Size(1280,720), true);
 
// 	cap.open("/home/nx/data/IMG_3066.MOV");
// 	cv::Mat frame, detret;
// 	while(1)
// 	{
// 		cap >> frame;
// 		if(frame.empty())
// 		{
// 			printf("video end\n");
// 			return 0;
// 		}
// 		cv::resize(frame,frame,cv::Size(1280,720));
// 		detector->process(frame, detret);
// 		writer << detret;
// 		cv::imshow("1", detret);
// 		cv::waitKey(10);
// 	}

// }

bool quit = false;

static void signal_handle(int signum)
{
	quit = true;
	// _xdma_reader_ch0.xdma_close();
}

int main(int argc, char* argv[])
{

	struct sigaction sig_action;
	sig_action.sa_handler = signal_handle;
	sigemptyset(&sig_action.sa_mask);
	sig_action.sa_flags = 0;
	sigaction(SIGINT, &sig_action, NULL);


    load_driver();

	sleep(1);

	cv::Mat frame0, frame1, frame;

	Sdireader_Init("../config/NXConfig.ini");
	

	// cxdma_rw_synchronizer xdma_rw_syn;
	// cxdma_receiver xdma_receiver("../config/NXConfig.ini", xdma_rw_syn);
	
	// //启动线程
	// xdma_receiver.Start();

	sleep(2);

	

	while(!quit)
	{
		Sdireader_GetFrame(frame0, frame1);
		cv::resize(frame1, frame, cv::Size(960,540));
		cv::imshow("1", frame);
		cv::waitKey(10);
	}

	return 0;
}

