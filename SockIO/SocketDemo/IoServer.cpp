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

	err = WSAStartup(wVersionRequested, &wsaData);//该函数的功能是加载一个Winsocket库版本
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

	m_sListen = socket(AF_INET, SOCK_STREAM, 0);//面向连接的可靠性服务SOCK_STRAM

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

	SOCKADDR_IN addrSrv;	//存放本地地址信息的
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);	//htol将主机字节序long型转换为网络字节序
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(m_port);	//htos用来将端口转换成字符，1024以上的数字即可

	if (0 != bind(m_sListen, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)))	//将socket绑定到相应地址和端口上
	{
		LOGERROR("绑定端口失败！");

		m_bInit = false;
		return ;
	}

	if (0 != listen(m_sListen, SOMAXCONN)) //等待队列中的最大长度为5
	{
		LOGERROR("监听端口[port:" + m_port + "]失败!");

		m_bInit = false;
		return ;
	}

	u_long argp = 1;
	if (SOCKET_ERROR == ioctlsocket(m_sListen, FIONBIO, &argp))
	{
		m_bInit = false;
		return ;
	}

	LOGDEBUG("正在监听端口：" + m_port);
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

		LOGINFO("当前在线人数：" + m_mapClient.size());
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

	SOCKET sNewAccept = accept(m_sListen, (SOCKADDR*)&addrRemote, &len);//建立一个新的套接字用于通信，不是前面的监听套接字

	if (INVALID_SOCKET == sNewAccept)
	{
		return false;
	}

	std::map<SOCKET, SOCKET>::iterator it = m_mapClient.find(sNewAccept);
	if (it != m_mapClient.end())
	{
		LOGERROR("Socket重复,socket:" + sNewAccept);
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
			// 没有收到数据
			++it;
			continue;
		}
		else if (0 == nRet)
		{
			// socket断开
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
	//LOGDEBUG("建立新的连接,socket:" + s);

	return true;
}

bool CIoServer::OnRecv( SOCKET s, const char *pData, int nLength )
{
	char szRecvData[1024] = {0};
	memcpy(szRecvData, pData, nLength);

	// 接收到数据
	LOGDEBUG("Socket[" + s + "]接收到数据：" + szRecvData + "数据长度：" + nLength);

	// 发送数据
	char szSendData[1024] = "hello world";
	Send(s, szSendData, strlen(szSendData));

	return true;
}

bool CIoServer::OnClose( SOCKET s )
{
	//LOGDEBUG("Socket[" + s + "]断开连接。");

	return true;
}
