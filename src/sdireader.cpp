#include "sdireader.h"
#include "cxdma_rw_synchronizer.h"
#include "cxdma_receiver.h"

static std::string cfgpath;
static cxdma_rw_synchronizer xdma_rw_syn;
static cxdma_receiver *xdma_receiver = nullptr; 

void Sdireader_Init(const std::string cfgpath)
{
    xdma_receiver = new cxdma_receiver(cfgpath, xdma_rw_syn);
    xdma_receiver->Start();

}

void Sdireader_GetFrame(cv::Mat &frame0, cv::Mat &frame1)
{
    xdma_receiver->getFrame(frame0, frame1);
}
