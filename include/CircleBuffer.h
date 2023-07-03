/*
*	环形缓冲区
*	可动态创建任意大小的环形缓冲区
*	支持缓冲区读写功能
*	缓冲区空定义：当第一个有效数据位置和下一个可存储位置重合时，
*	缓冲区满定义：当下一个可存储位置紧随第一个有效数据位置之后，
*	保证缓冲区中总有一个存储单元为空，这种状态定义为满，此种设计使得缓冲区容量最大为m_Size - 1
*	data		20230309 
*	author		fex
*	version		V1.0.0
*/

#ifndef __CIRCLEBUFFER_H
#define __CIRCLEBUFFER_H

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

class CircleBuffer
{
	private:
		uint8_t  *m_Buffer;
		//缓冲区大小，实际有效数据存储占比为（m_Size - 1）/m_Size * 100%
		uint64_t m_Size;
		//第一个有效存储位置
		uint64_t m_ValidPosition;
		//下一个可存储位置
		uint64_t m_StorablePosition;
		
	public:
		//显示构造函数，防止CircleBuffer对象隐式转换为const uint64_t类型
		explicit CircleBuffer(const uint64_t size);
		//禁用拷贝构造
		CircleBuffer(CircleBuffer& object)=delete;
		//禁用赋值构造函数
		CircleBuffer& operator=(CircleBuffer& object)=delete;
		
		//析构函数
		virtual ~CircleBuffer();
		
		//缓冲区满
		const bool isFull() const;
		//缓冲区空
		const bool isEmpty() const;
		
		//获取缓冲区可用长度
		const uint64_t getLenOfValidSpace() const;
		//获取缓冲区有效数据总量
		const uint64_t getLenOfValidData() const;
		
		//获取第一个有效数据位置
		const uint64_t getValidPosition() const;
		//获取下一个可存储位置
		const uint64_t getStortablePosition() const;
		
		
		//向环形缓冲区中增加数据
		const uint64_t pushData(const uint8_t *buffer, const uint64_t size);
		//删除环形缓冲区中的数据
		const uint64_t popData(const uint64_t size);
		
		//获取循环缓冲区
		const uint8_t* getBuffer();
		//读取一定容量的环形缓冲区数据到目标线性缓冲区
		const uint64_t readData(uint8_t *buffer, const uint64_t size);

		//clear validdata
		const bool isClear();
};

#endif