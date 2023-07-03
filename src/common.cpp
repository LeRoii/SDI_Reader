/*****************
 * project:NXProject
 * source:common.cpp
 * author:FEX
 * time:2023-04-21
 * description:
 *  公共层基类实现
 * copyright:
 *
 *
 * ***************/

#include "common.h"

namespace NXProject
{
    //静态对象初始化
    Common* Common::m_object = nullptr;
    std::mutex Common::m_mutex;
    static std::string cfgPath("/etc/config/NXConfig.ini");
    //获取实例对象
    Common* Common::GetInstance()
    {
        //如果当前实例对象为空，则创建对象
        if(nullptr == m_object || NULL == m_object)
        {
            //确保多线程使用的安全性
            std::lock_guard<std::mutex> mylock(Common::m_mutex);
            if(nullptr == m_object || NULL == m_object)
            {
                m_object = new Common;
            }
        }

        return m_object;
    }

    int Common::SetCfgPath(const std::string path)
    {
        cfgPath = path;

        return 0;
    }

    //读取配置文件中的配置信息，
    //key-要查询的关键字，value-查询到的值，length-保存值的字符串缓冲区最大长度
    //配置文件中每行的信息格式为：key=value
    int Common::GetValueByKeyFromConfig(const char* key, char* value, const unsigned int length)
    {
        if(NULL == key || NULL == value || length <= 0)
        {
            return -1;
        }

        int ret=0;
        int FD = 0;
        char* ConfigContent = new char[4096];
        char *findStr = NULL;


        // char path[]="../config/NXConfig.ini"; 
        

        //打开配置文件，读写模式
        FD  = open(cfgPath.c_str(),O_RDWR|O_CREAT,0644);

        if(FD < 0 )
        {
            printf("%s",strerror(errno));
            return -1;
        }

        //读取文件中的内容到ConfigContent缓冲区，缓冲区最多容纳2048字节
        ret = read(FD,ConfigContent,4096);
        //关闭打开的文件
        close(FD);
        //从文件中读取到的字符数大于等于1时，进行处理
        if(1 <= ret)
        {
            //字符串查找函数，在ConfigContent中查找key字符串，并返回找到的位置，如果没找到，则返回NULL
            findStr = strstr(ConfigContent,key);
            //如果没找到子字符串
            if(findStr == NULL)
            {
                return -1;
            }

            const int keyLen = strlen(key);
            int i=0 ,j=0;
            for( ; i< strlen(findStr)-1-keyLen && j<length-1; i++)
            {
                if(findStr[keyLen+1+i] == ' ' || findStr[keyLen+1+i] == '\n' || findStr[keyLen+1+i] == '\r'
                    || findStr[keyLen+1+i] == '\t'  || findStr[keyLen+1+i] == '#'   || findStr[keyLen+1+i] == EOF)
                {
                    break;
                }
                else
                {
                    value[j++] = findStr[keyLen+1+i];
                }
            }

            //printf("key-%s\r\nvalue-%s\r\n", key, value);
            //返回找到的字符串value的实际长度
            return j;
        }
        else//从配置文件中读取到的字符数小于1，则直接返回
        {
            return -1;
        }
    }



    //私有构造、析构函数
    Common::Common()
    {

    }
    Common::Common(const Common &)
    {

    }
    Common::~Common()
    {

    }
    Common& Common::operator=(const Common&)
    {

    }

}//end namespace MCCLIENT
