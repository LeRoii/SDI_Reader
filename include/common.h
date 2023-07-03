/*****************
 * project:NXProject
 * source:common.h
 * author:FEX
 * time:2023-04-21
 * description:
 *  公共层基类定义
 *      单例类，主要定义常用的函数
 * copyright:
 *
 *
 * ***************/


#include <iostream>
#include <string>
#include <vector>
#include <map>


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>

#include <pthread.h>

#include <sys/stat.h>
#include <sys/types.h>


#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <thread>
#include <mutex>




namespace NXProject
{
    class Common
    {
    public:
        //获取实例对象
        static Common* GetInstance();

        //读取配置文件中的配置信息，key-要查询的关键字，value-查询到的值，length-value能容纳的最大字符串长度
        static int GetValueByKeyFromConfig(const char* key, char* value, const unsigned int length);

        static int SetCfgPath(const std::string path);

    private:
        static Common* m_object;

        //私有构造、析构函数
        Common();
        Common(const Common &);
        ~Common();
        Common& operator=(const Common&);

        //互斥对象，保证线程安全
        static std::mutex m_mutex;

    };//end class Common

}//end namespace MCCLIENT
