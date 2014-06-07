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
				// CTwoQueues����malloc�ڴ�ʧ�ܡ�
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

		unsigned int GetFreeLen()	//�����ڴ泤��
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

			// ��¼���ݳ���
			(*pDataLen) = nLen;
			pDataWrite += sizeof(unsigned int);

			// ��������
			memcpy(pDataWrite, pData, nLen);
			pDataWrite += nLen; 

			// ������Ŀ+1
			nDataCounts += 1;

			// ���ݴ洢ռ���ڴ��С
			nDataLen += (sizeof(unsigned int) + nLen);

			return true;
		}

		bool IsEmpty()	//�ж��Ƿ�������
		{
			return (nDataCounts==0);
		}

		bool PrepareData(const void*& pData, unsigned int& nLen)	//׼��һ������
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

		void ConfimData()	//ɾ��һ������
		{
			if (nDataCounts > 0)
			{
				// �ƶ���ȡָ��
				pDataRead += (sizeof(unsigned int) + (*(unsigned int*)pDataRead));
				// ������Ŀ-1
				nDataCounts -= 1;
			}
		}

		void ReSet()	//�����ڴ�洢����
		{
			pDataRead = pDataMem;
			pDataWrite = pDataMem;

			nDataCounts = 0;
			nDataLen = 0;
		}

		unsigned int nMaxSize;	//���洢�����С
		char* pDataMem;			//�����ڴ�����

		unsigned int nDataLen;	//���ݴ洢ռ���ڴ��С
		char* pDataWrite;	//�ڴ�д��λ��
		char* pDataRead;	//�ڴ��ȡλ��
		unsigned int nDataCounts;	//������Ŀ

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
			// �ͷ��ڴ�
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
		// ��ʼ������
		// nMemSize �� �����ڴ���С(Ĭ��0x00100000==1M)
		// wMaxMemCount �� ����ڴ������(Ĭ��16�飬���ռ��16M�ڴ�)
		*/
		void Init(unsigned int nMemSize = 0x00100000, unsigned short wMaxMemCount = 16)
		{
			if (0 == nMemSize)
			{
				// ��ʼ��CTwoQueues����ʧ�ܡ�
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

			// ������������
			m_pWriteDataMem->_next = m_pReadDataMem;
			m_pReadDataMem->_next = m_pWriteDataMem;

			// �ڴ������ʱ��ͳ�פʹ���ڴ������
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
					// �����������bug�ˡ�д��ڵ�Ϊ��ʱ�������ƶ��ڵ�
					//LeaveCriticalSection(&m_crit);
					m_lock.Unlock();
					return false;
				}

				// д��ʧ�ܣ��ƶ�д��ڵ�
				if (m_pWriteDataMem->_next == m_pReadDataMem)
				{
					// д��׷���˶�ȡ�������ڴ��
					DATAMEM* pDataMem = Fectch();
					if (NULL == pDataMem)
					{
						//LeaveCriticalSection(&m_crit);
						m_lock.Unlock();
						// �ڴ������ʧ�ܣ������Ǵﵽ�ڴ�����
						return false;
					}

					// д��Ͷ�ȡ�м����һ���ڵ�
					m_pWriteDataMem->_next = pDataMem;
					pDataMem->_next = m_pReadDataMem;
				}

				// �ı�д��λ��
				m_pWriteDataMem = m_pWriteDataMem->_next;

				bResult = m_pWriteDataMem->PushData(pData, nLen);
				if (!bResult)
				{
					//LeaveCriticalSection(&m_crit);
					m_lock.Unlock();
					// д�����ݣ�����һ���ڴ���С��
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

				// ���ݶ�ȡ��ϣ������л��ڵ�
				//EnterCriticalSection(&m_crit);
				m_lock.Lock();
				
				if (m_pReadDataMem->_next->IsEmpty())
				{
					// û��������
					bCanRead = false;
				}
				else
				{
					if (m_pReadDataMem->_next == m_pWriteDataMem)
					{
						// ��ȡ׷��д��
						// д��ڵ�ǰ��
						m_pWriteDataMem = m_pWriteDataMem->_next;
					}

					// ��ȡ�ڵ�ǰ��
					m_pReadDataMem = m_pReadDataMem->_next;

					// ��������
					bCanRead = true;

					// ��ȡ�ڵ��л�
					m_nSwitchQueueTimes++;
				}

				// �����ȡ��д��������ݿ���Ŀ
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

				// �ڴ�����
				DWORD dwNow = GetTickCount();
				if (dwNow - m_dwMemClearTime > 10000)
				{
					if (m_wR2W < 2)
					{
						m_wR2W = 2;
					}
					
					// �ͷ��ڴ�
					if (m_wCurDataMemCount-m_wR2W > 0)
					{
						DATAMEM* pDelDataMem = m_pWriteDataMem->_next;

						if (pDelDataMem == m_pReadDataMem)
						{
							// ��ǰ�ڵ�Ϊ��ȡ�ڵ㣬����ɾ��
						}
						else
						{
							// ж��ɾ���ڵ�
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
				// ��������
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

		// ����˫��������
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
						// д��ڵ����һ���ڵ��Ƕ�ȡ�ڵ㣬�Ͳ�Ҫ�ټ���ɾ���ˡ�
						break;
					}
					else
					{
						// ж��ɾ���ڵ�
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
						// д��ڵ����һ���ڵ��Ƕ�ȡ�ڵ㣬�Ͳ�Ҫ�ټ���ɾ���ˡ�
						break;
					}
					else
					{
						// ж��ɾ���ڵ�
						m_pReadDataMem->_next = pDelDataMem->_next;

						Recycle(pDelDataMem);
						pDelDataMem = NULL;
					}
				}
			}

			// ���ò�������
			m_wR2W = 0;
			m_dwMemClearTime = GetTickCount();
			m_nSwitchQueueTimes = 0;

			m_pReadDataMem->ReSet();
			m_pWriteDataMem->ReSet();

			//LeaveCriticalSection(&m_crit);
			m_lock.Unlock();
		}

		// ��ȡ�����л�����
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

		// ��ն����л�����
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
				// ����
				return false;
			}
		}

	private:
		//CRITICAL_SECTION m_crit;
		MyLock m_lock;
		// ���ж�ȡ��д��λ��
		DATAMEM* m_pWriteDataMem;
		DATAMEM* m_pReadDataMem;

		unsigned int m_nMemSize;	//�ڴ���С
		unsigned short m_wMaxDataMemCount;	//�������������ڴ������
		unsigned short m_wCurDataMemCount;	//��������ڴ������

		DWORD m_dwMemClearTime;	//�ڴ�����ʱ���¼
		unsigned short m_wR2W;	//��פʹ���ڴ������(��ȡ��д����ڴ������)

		unsigned int m_nSwitchQueueTimes;	//�����л�����

	};
}