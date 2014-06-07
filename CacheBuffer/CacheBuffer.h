//***********************************
// author : alterhz
// date : 2014-6-7
// description : 
//***********************************


#pragma once

namespace DSC
{

#define __WINDOWS__

#if defined(__WINDOWS__)

#include <Windows.h>
#include <assert.h>

class MyLock
{
	CRITICAL_SECTION m_Lock ;
public :
	MyLock( ){ InitializeCriticalSectionAndSpinCount(&m_Lock, 4000); }
	~MyLock( ){ DeleteCriticalSection(&m_Lock); }
	void Lock( ){ EnterCriticalSection(&m_Lock); }
	void Unlock( ){ LeaveCriticalSection(&m_Lock); }
};

#elif defined(__LINUX__)

class MyLock
{
	pthread_mutex_t 	m_Mutex; 
public :
	MyLock( ){ pthread_mutex_init( &m_Mutex , NULL );}
	~MyLock( ){ pthread_mutex_destroy( &m_Mutex) ; }
	void Lock( ){ pthread_mutex_lock(&m_Mutex); }
	void Unlock( ){ pthread_mutex_unlock(&m_Mutex); }
};

#endif

	typedef struct tagDataMem
	{
		tagDataMem(unsigned int nSize)
		{
			if (0 == nSize)
			{
				assert(false);
				return ;
			}

			nMaxSize = nSize;
			pDataMem = (char*)malloc(nSize*sizeof(char));
			
			if (NULL == pDataMem)
			{
				// CTwoQueues申请malloc内存失败。
				assert(false);
			}

			pDataWrite = pDataMem;
			pDataRead = pDataMem;

			nDataLen = 0;
			nDataCounts = 0;

			_next = NULL;
		}
		~tagDataMem()
		{
			free(pDataMem);
			pDataMem = NULL;
		}

		unsigned int GetFreeLen()	//空闲内存长度
		{
			return nMaxSize-nDataLen;
		}

		bool PushData(void* pData, unsigned int nLen)
		{
			if (nDataLen+sizeof(unsigned int)+nLen >= nMaxSize)
			{
				return false;
			}

			unsigned int *pDataLen = (unsigned int*)pDataWrite;

			if (NULL == pDataLen)
			{
				return false;
			}

			// 记录数据长度
			(*pDataLen) = nLen;
			pDataWrite += sizeof(unsigned int);

			// 拷贝数据
			memcpy(pDataWrite, pData, nLen);
			pDataWrite += nLen; 

			// 数据条目+1
			nDataCounts += 1;

			// 数据存储占用内存大小
			nDataLen += (sizeof(unsigned int) + nLen);

			return true;
		}

		bool IsEmpty()	//判断是否有数据
		{
			return (nDataCounts==0);
		}

		bool PrepareData(const void*& pData, unsigned int& nLen)	//准备一条数据
		{
			if (nDataCounts > 0)
			{
				pData = pDataRead+sizeof(unsigned int);
				nLen = (*(unsigned int*)pDataRead);
				return true;
			}
			else
			{
				return false;
			}
		}

		void ConfimData()	//删除一条数据
		{
			if (nDataCounts > 0)
			{
				// 移动读取指针
				pDataRead += (sizeof(unsigned int) + (*(unsigned int*)pDataRead));
				// 数据条目-1
				nDataCounts -= 1;
			}
		}

		void ReSet()	//重置内存存储对象
		{
			pDataRead = pDataMem;
			pDataWrite = pDataMem;

			nDataCounts = 0;
			nDataLen = 0;
		}

		unsigned int nMaxSize;	//最大存储区域大小
		char* pDataMem;			//数据内存区域

		unsigned int nDataLen;	//数据存储占用内存大小
		char* pDataWrite;	//内存写入位置
		char* pDataRead;	//内存读取位置
		unsigned int nDataCounts;	//数据条目

		tagDataMem* _next;
	}DATAMEM;

	class CTwoQueues
	{
	public:
		CTwoQueues(void)
		{
			//InitializeCriticalSection(&m_crit);
			m_pWriteDataMem = NULL;
			m_pReadDataMem = NULL;
			m_nMemSize = 0x2000;
			m_wMaxDataMemCount = 0;
			m_wCurDataMemCount = 0;
			m_dwMemClearTime = 0;
			m_wR2W = 0;
			m_nSwitchQueueTimes = 0;
		}
		~CTwoQueues(void)
		{
			// 释放内存
			while (NULL != m_pWriteDataMem)
			{
				DATAMEM* pDelDataMem = m_pWriteDataMem->_next;
				m_pWriteDataMem->_next = pDelDataMem->_next;

				if (m_pWriteDataMem->_next == m_pWriteDataMem)
				{
					delete pDelDataMem;
					pDelDataMem = NULL;

					delete m_pWriteDataMem;
					m_pWriteDataMem = NULL;
					m_pReadDataMem = NULL;
				}
				else
				{
					delete pDelDataMem;
					pDelDataMem = NULL;
				}
			}

			//DeleteCriticalSection(&m_crit);
		}

	public:
		/*
		// 初始化队列
		// nMemSize ： 单个内存块大小(默认0x00100000==1M)
		// wMaxMemCount ： 最大内存块数量(默认16块，最大占用16M内存)
		*/
		void Init(unsigned int nMemSize = 0x00100000, unsigned short wMaxMemCount = 16)
		{
			if (0 == nMemSize)
			{
				// 初始化CTwoQueues对象失败。
				assert(false);
				return ;
			}

			m_nMemSize = nMemSize;
			m_wMaxDataMemCount = wMaxMemCount;
			m_wCurDataMemCount = 0;

			m_pWriteDataMem = Fectch();
			assert(!(m_pWriteDataMem==NULL));

			m_pReadDataMem = Fectch();
			assert(!(m_pReadDataMem==NULL));

			// 建立环形链表
			m_pWriteDataMem->_next = m_pReadDataMem;
			m_pReadDataMem->_next = m_pWriteDataMem;

			// 内存块清理时间和常驻使用内存块数量
			m_dwMemClearTime = GetTickCount();
			m_wR2W = 0;
		}

		bool PushData(void* pData, unsigned int nLen)
		{
			bool bResult = false;

			if (sizeof(unsigned int) + nLen >= m_nMemSize)
			{
				return false;
			}

			//EnterCriticalSection(&m_crit);
			m_lock.Lock();

			bResult = m_pWriteDataMem->PushData(pData, nLen);
			if (!bResult)
			{
				if (m_pWriteDataMem->IsEmpty())
				{
					// 进入这里，就是bug了。写入节点为空时，不能移动节点
					//LeaveCriticalSection(&m_crit);
					m_lock.Unlock();
					return false;
				}

				// 写入失败，移动写入节点
				if (m_pWriteDataMem->_next == m_pReadDataMem)
				{
					// 写入追上了读取，申请内存块
					DATAMEM* pDataMem = Fectch();
					if (NULL == pDataMem)
					{
						//LeaveCriticalSection(&m_crit);
						m_lock.Unlock();
						// 内存块申请失败，可能是达到内存上线
						return false;
					}

					// 写入和读取中间插入一个节点
					m_pWriteDataMem->_next = pDataMem;
					pDataMem->_next = m_pReadDataMem;
				}

				// 改变写入位置
				m_pWriteDataMem = m_pWriteDataMem->_next;

				bResult = m_pWriteDataMem->PushData(pData, nLen);
				if (!bResult)
				{
					//LeaveCriticalSection(&m_crit);
					m_lock.Unlock();
					// 写入数据，大于一个内存块大小。
					return false;
				}
			}

			//LeaveCriticalSection(&m_crit);
			m_lock.Unlock();

			return bResult;
		}

		bool PrepareData(const void*& pData, unsigned int& nLen)
		{
			bool bCanRead = true;

			if (m_pReadDataMem->IsEmpty())
			{
				m_pReadDataMem->ReSet();

				DATAMEM* pDelDataMemList = NULL;

				// 数据读取完毕，请求切换节点
				//EnterCriticalSection(&m_crit);
				m_lock.Lock();
				
				if (m_pReadDataMem->_next->IsEmpty())
				{
					// 没有数据了
					bCanRead = false;
				}
				else
				{
					if (m_pReadDataMem->_next == m_pWriteDataMem)
					{
						// 读取追上写入
						// 写入节点前进
						m_pWriteDataMem = m_pWriteDataMem->_next;
					}

					// 读取节点前进
					m_pReadDataMem = m_pReadDataMem->_next;

					// 存在数据
					bCanRead = true;

					// 读取节点切换
					m_nSwitchQueueTimes++;
				}

				// 计算读取到写入的总数据块数目
				unsigned short wCount = 1;
				DATAMEM* pTmpDataMem = m_pReadDataMem;
				for (; pTmpDataMem!=m_pWriteDataMem && wCount<=m_wCurDataMemCount; wCount++)
				{
					pTmpDataMem = pTmpDataMem->_next;
				}

				if (wCount > m_wR2W)
				{
					m_wR2W = wCount;
				}

				// 内存清理
				DWORD dwNow = GetTickCount();
				if (dwNow - m_dwMemClearTime > 10000)
				{
					if (m_wR2W < 2)
					{
						m_wR2W = 2;
					}
					
					// 释放内存
					if (m_wCurDataMemCount-m_wR2W > 0)
					{
						DATAMEM* pDelDataMem = m_pWriteDataMem->_next;

						if (pDelDataMem == m_pReadDataMem)
						{
							// 当前节点为读取节点，不能删除
						}
						else
						{
							// 卸下删除节点
							m_pWriteDataMem->_next = pDelDataMem->_next;

							Recycle(pDelDataMem);
							pDelDataMem = NULL;
						}
					}

					m_dwMemClearTime = dwNow;
					m_wR2W = 0;
				}

				//LeaveCriticalSection(&m_crit);
				m_lock.Unlock();
			}
			else
			{
				// 存在数据
				bCanRead = true;
			}

			if (bCanRead)
			{
				return m_pReadDataMem->PrepareData(pData, nLen);
			}
			else
			{
				return false;
			}
		}

		void ConfimData()
		{
			m_pReadDataMem->ConfimData();
		}

		// 重置双队列数据
		void ClearData()
		{
			//EnterCriticalSection(&m_crit);
			m_lock.Lock();
			
			if (NULL != m_pWriteDataMem)
			{
				
				while (m_pWriteDataMem->_next != NULL)
				{
					DATAMEM* pDelDataMem = m_pWriteDataMem->_next;

					if (pDelDataMem == m_pReadDataMem)
					{
						// 写入节点的下一个节点是读取节点，就不要再继续删除了。
						break;
					}
					else
					{
						// 卸下删除节点
						m_pWriteDataMem->_next = pDelDataMem->_next;

						Recycle(pDelDataMem);
						pDelDataMem = NULL;
					}
				}
			}

			if (NULL != m_pReadDataMem)
			{

				while (m_pReadDataMem->_next != NULL)
				{
					DATAMEM* pDelDataMem = m_pReadDataMem->_next;

					if (pDelDataMem == m_pWriteDataMem)
					{
						// 写入节点的下一个节点是读取节点，就不要再继续删除了。
						break;
					}
					else
					{
						// 卸下删除节点
						m_pReadDataMem->_next = pDelDataMem->_next;

						Recycle(pDelDataMem);
						pDelDataMem = NULL;
					}
				}
			}

			// 重置部分数据
			m_wR2W = 0;
			m_dwMemClearTime = GetTickCount();
			m_nSwitchQueueTimes = 0;

			m_pReadDataMem->ReSet();
			m_pWriteDataMem->ReSet();

			//LeaveCriticalSection(&m_crit);
			m_lock.Unlock();
		}

		// 获取队列切换次数
		unsigned int GetSwitchQueueTimes() 
		{ 
			unsigned int nSwitchQueueTimes = 0;

			//EnterCriticalSection(&m_crit);
			m_lock.Lock();
			nSwitchQueueTimes = m_nSwitchQueueTimes;
			//LeaveCriticalSection(&m_crit);
			m_lock.Unlock();

			return nSwitchQueueTimes;
		}

		// 清空队列切换次数
		void ClearSwitchQueueTimes() 
		{ 
			//EnterCriticalSection(&m_crit);
			m_lock.Lock();
			m_nSwitchQueueTimes = 0;
			//LeaveCriticalSection(&m_crit);
			m_lock.Unlock();
		}

	private:
		DATAMEM* Fectch()
		{
			if (m_wCurDataMemCount < m_wMaxDataMemCount)
			{
				DATAMEM* pNewDataMem = new DATAMEM(m_nMemSize);

				m_wCurDataMemCount++;

				return pNewDataMem;
			}
			else
			{
				return NULL;
			}
		}

		bool Recycle(DATAMEM* pDataMem)
		{
			if (NULL == pDataMem)
			{
				return false;
			}

			delete pDataMem;
			pDataMem = NULL;

			if (m_wCurDataMemCount>1)
			{
				m_wCurDataMemCount--;
				return true;
			}
			else
			{
				// 错误
				return false;
			}
		}

	private:
		//CRITICAL_SECTION m_crit;
		MyLock m_lock;
		// 队列读取和写入位置
		DATAMEM* m_pWriteDataMem;
		DATAMEM* m_pReadDataMem;

		unsigned int m_nMemSize;	//内存块大小
		unsigned short m_wMaxDataMemCount;	//可以申请的最大内存块数量
		unsigned short m_wCurDataMemCount;	//已申请的内存块数量

		DWORD m_dwMemClearTime;	//内存清理时间记录
		unsigned short m_wR2W;	//常驻使用内存块数量(读取到写入的内存块数量)

		unsigned int m_nSwitchQueueTimes;	//队列切换次数

	};
}