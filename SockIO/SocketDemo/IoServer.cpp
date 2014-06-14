#include "IoServer.h"
#include "DebugLog.h"

CIoServer::CIoServer( PORT port )
{
	m_bInit = true;
	m_port = 0;
	
	m_sListen = INVALID_SOCKET;
	m_nElapsed = 0;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);//�ú����Ĺ����Ǽ���һ��Winsocket��汾
	if ( err != 0 ) {
		m_bInit = false;
		return;
	}
	
	if ( LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1 ) {
			WSACleanup( );
			m_bInit = false;
			return; 
	}

	m_port = port;

	m_sListen = socket(AF_INET, SOCK_STREAM, 0);//�������ӵĿɿ��Է���SOCK_STRAM

	int opt = 1;
	if (SOCKET_ERROR == setsockopt(m_sListen, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int)))
	{
		m_bInit = false;
		return ;
	}

	struct linger li;
	li.l_onoff = 1;
	li.l_linger = 1;
	setsockopt(m_sListen, SOL_SOCKET, SO_LINGER, (char *)&li, sizeof(li));

	SOCKADDR_IN addrSrv;	//��ű��ص�ַ��Ϣ��
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);	//htol�������ֽ���long��ת��Ϊ�����ֽ���
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(m_port);	//htos�������˿�ת�����ַ���1024���ϵ����ּ���

	if (0 != bind(m_sListen, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)))	//��socket�󶨵���Ӧ��ַ�Ͷ˿���
	{
		LOGERROR("�󶨶˿�ʧ�ܣ�");

		m_bInit = false;
		return ;
	}

	if (0 != listen(m_sListen, SOMAXCONN)) //�ȴ������е���󳤶�Ϊ5
	{
		LOGERROR("�����˿�[port:" + m_port + "]ʧ��!");

		m_bInit = false;
		return ;
	}

	u_long argp = 1;
	if (SOCKET_ERROR == ioctlsocket(m_sListen, FIONBIO, &argp))
	{
		m_bInit = false;
		return ;
	}

	LOGDEBUG("���ڼ����˿ڣ�" + m_port);
}

CIoServer::~CIoServer()
{
	WSACleanup();
}

void CIoServer::Run( unsigned long nElapsed )
{
	for (int i=0; i<1000; ++i)
	{
		bool bContinue = false;
		if (OnAccept())
		{
			bContinue = true;
		}
	}

	OnRecv();

	m_nElapsed += nElapsed;
	if (m_nElapsed > 1000)
	{
		m_nElapsed -= 1000;

		LOGINFO("��ǰ����������" + m_mapClient.size());
	}
}

bool CIoServer::Send( SOCKET s, const char *pData, int nLength )
{
	int nRet = send(s, pData, nLength, 0);

	return (SOCKET_ERROR != nRet);
}

bool CIoServer::OnAccept()
{
	SOCKADDR_IN addrRemote;
	int len = sizeof(SOCKADDR);

	SOCKET sNewAccept = accept(m_sListen, (SOCKADDR*)&addrRemote, &len);//����һ���µ��׽�������ͨ�ţ�����ǰ��ļ����׽���

	if (INVALID_SOCKET == sNewAccept)
	{
		return false;
	}

	std::map<SOCKET, SOCKET>::iterator it = m_mapClient.find(sNewAccept);
	if (it != m_mapClient.end())
	{
		LOGERROR("Socket�ظ�,socket:" + sNewAccept);
		return false;
	}

	m_mapClient.insert(std::make_pair(sNewAccept, sNewAccept));

	OnAccept(sNewAccept);

	return true;
}

bool CIoServer::OnRecv()
{
	bool bContinue = false;

	std::map<SOCKET, SOCKET>::iterator it = m_mapClient.begin();
	for (; it != m_mapClient.end(); )
	{
		SOCKET s = it->second;

		char szRecv[1024] = {0};
		int nRet = recv(s, szRecv, 1024, 0);
		if (SOCKET_ERROR == nRet)
		{
			// û���յ�����
			++it;
			continue;
		}
		else if (0 == nRet)
		{
			// socket�Ͽ�
			OnClose(s);

			it = m_mapClient.erase(it);

			bContinue = true;
			continue;
		}

		OnRecv(s, szRecv, nRet);
		++it;

		bContinue = true;
	}

	return bContinue;
}

bool CIoServer::OnAccept( SOCKET s )
{
	//LOGDEBUG("�����µ�����,socket:" + s);

	return true;
}

bool CIoServer::OnRecv( SOCKET s, const char *pData, int nLength )
{
	char szRecvData[1024] = {0};
	memcpy(szRecvData, pData, nLength);

	// ���յ�����
	LOGDEBUG("Socket[" + s + "]���յ����ݣ�" + szRecvData + "���ݳ��ȣ�" + nLength);

	// ��������
	char szSendData[1024] = "hello world";
	Send(s, szSendData, strlen(szSendData));

	return true;
}

bool CIoServer::OnClose( SOCKET s )
{
	//LOGDEBUG("Socket[" + s + "]�Ͽ����ӡ�");

	return true;
}
