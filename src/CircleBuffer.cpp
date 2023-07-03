/*
*	环形缓冲区
*	可动态创建任意大小的环形缓冲区
*	支持缓冲区读写功能
*	20230309 fex
*/

#include "CircleBuffer.h"

//构造函数
CircleBuffer::CircleBuffer(const uint64_t size):
	m_Size(size),
	m_ValidPosition(0),
	m_StorablePosition(0)
{
	m_Buffer = new uint8_t[size];
}

//析构函数
CircleBuffer::~CircleBuffer()
{
	delete [] m_Buffer;
}

//缓冲区满，下一个可用位置+1对m_Size取余后的值等于当前有效位置
const bool CircleBuffer::isFull() const
{
	return ((m_StorablePosition + 1) % m_Size) == m_ValidPosition;
}

//缓冲区空，缓冲区中所有空间可用，下一个可用位置和第一个有效位置重合
const bool CircleBuffer::isEmpty() const
{
	return m_ValidPosition == m_StorablePosition;
} 

		
//获取缓冲区可用长度
const uint64_t CircleBuffer::getLenOfValidSpace() const
{
	//如果下一个可存储位置大于首个有效数据位置
	if(m_StorablePosition > m_ValidPosition)
	{
		return (m_Size - 1) - (m_StorablePosition - m_ValidPosition);
	}
	//如果下一个可存储位置小于首个有效数据位置
	else if(m_StorablePosition < m_ValidPosition)
	{
		return m_ValidPosition - m_StorablePosition - 1;
	}
	//如果下一个可存储位置等于首个有效数据位置
	else
	{
		return m_Size - 1;
	}
}

//获取缓冲区有效数据总量
const uint64_t CircleBuffer::getLenOfValidData() const
{
	return m_Size - 1 - getLenOfValidSpace();
}


//获取第一个有效数据位置
const uint64_t CircleBuffer::getValidPosition() const
{
	return m_ValidPosition;
}

//获取下一个可存储位置
const uint64_t CircleBuffer::getStortablePosition() const
{
	return m_StorablePosition;
}


//向环形缓冲区中增加数据
const uint64_t CircleBuffer::pushData(const uint8_t *buffer, const uint64_t size)
{
	//获取缓冲区可用长度
	const uint64_t lenOfValidSpace = getLenOfValidSpace();
	//如果要存入的数据总量小于等于当前可用数据空间
	if(size < lenOfValidSpace)
	{
		//
		if(size <= m_Size - 1 - m_StorablePosition)
		{
			//连续内存数据拷贝
			memcpy(m_Buffer+m_StorablePosition, buffer, size);
		}
		else
		{
			//第一段连续内存数据拷贝
			memcpy(m_Buffer+m_StorablePosition, buffer, m_Size-m_StorablePosition);
			
			//第二段连续内存数据拷贝
			memcpy(m_Buffer, buffer+m_Size-m_StorablePosition, size-(m_Size-m_StorablePosition)<0?0:size-(m_Size-m_StorablePosition));
			
		}
		//下一个有效存储位置计算
		m_StorablePosition += size;
		m_StorablePosition %= m_Size;
		return size;
	}
	//如果要存入的数据总量大于环形缓冲区可用空间，则只存入当前缓冲区能容纳的最大量数据
	else
	{ 
		//
		if(lenOfValidSpace <= m_Size - 1 - m_StorablePosition)
		{
			//连续内存数据拷贝
			memcpy(m_Buffer+m_StorablePosition, buffer, lenOfValidSpace);
		}
		else
		{
			//第一段连续内存数据拷贝
			memcpy(m_Buffer+m_StorablePosition, buffer, m_Size-m_StorablePosition);
			
			//第二段连续内存数据拷贝
			memcpy(m_Buffer, buffer+m_Size-m_StorablePosition, lenOfValidSpace-(m_Size-m_StorablePosition)<0?0:lenOfValidSpace-(m_Size-m_StorablePosition));
			
		}
		
		//下一个有效存储位置计算
		m_StorablePosition += lenOfValidSpace;
		m_StorablePosition %= m_Size;
		
		return lenOfValidSpace;
	}
	
	/*uint64_t i = 0;
	
	const uint64_t lenOfValidSpace = getLenOfValidSpace();
	
	for(;i < size && i <= lenOfValidSpace;i++)
	{
		m_Buffer[(m_StorablePosition+i)%m_Size] = buffer[i];
	}
	//下一个有效存储位置计算
	m_StorablePosition += i;
	m_StorablePosition %= m_Size;
	
	return i;*/
}

//删除环形缓冲区中的数据
const uint64_t CircleBuffer::popData(const uint64_t size)
{
	const uint64_t lenOfValid = getLenOfValidData();
	//如果缓冲区中的数据总量大于等于要删除的数据量
	if(lenOfValid >= size)
	{
		//重新计算第一个有效数据位置
		m_ValidPosition += size;
		m_ValidPosition %= m_Size;
		return size;
	}
	//如果缓冲区中的数据总量小于要删除的数据量，则删除缓冲区所有数据
	else
	{
		//更新
		m_ValidPosition += lenOfValid;
		m_ValidPosition %= m_Size;
		
		return lenOfValid;
	}

}


//获取循环缓冲区
const uint8_t* CircleBuffer::getBuffer()
{
	return this->m_Buffer;
}


//读取一定容量的环形缓冲区数据到目标线性缓冲区
const uint64_t CircleBuffer::readData(uint8_t *buffer, const uint64_t size)
{ 
	//判断缓冲区是否为空，需要读取的数据量是否大于0
	if(buffer != nullptr && buffer != NULL && size > 0)
	{
		//当前要读取的数据量小于等于环形缓冲区的有效数据量
		if(size <= getLenOfValidData())
		{
			//第一段数据拷贝
			memcpy(buffer, m_Buffer+m_ValidPosition, m_Size - m_ValidPosition >= size? size : m_Size - m_ValidPosition);
			//如果第一段数据不足以满足size的数据量，则复制第二段数据到目标缓冲区
			if(m_Size - m_ValidPosition < size)
			{
				//剩余数据拷贝
				memcpy(buffer+m_Size-m_ValidPosition, m_Buffer, size - (m_Size - m_ValidPosition));
			}
			
			return size;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}	
}


//clear validdata
const bool CircleBuffer::isClear()
{
	this->m_ValidPosition = this->m_StorablePosition;
}