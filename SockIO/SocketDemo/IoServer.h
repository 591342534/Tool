//////////////////////////////////////////////////////////////////////////

#ifndef _IOSERVER_
#define _IOSERVER_

#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <map>

typedef unsigned short PORT;

class CIoServer
{
public:
	CIoServer(PORT port);
	~CIoServer();


public:
	void Run(unsigned long nElapsed);

	bool Send(SOCKET s, const char *pData, int nLength);

public:
	virtual bool OnAccept(SOCKET s);
	virtual bool OnRecv(SOCKET s, const char *pData, int nLength);
	virtual bool OnClose(SOCKET s);

private:
	virtual bool OnAccept();
	virtual bool OnRecv();

private:
	bool m_bInit;
	PORT m_port;

	SOCKET m_sListen;

	unsigned long m_nElapsed;

	std::map<SOCKET, SOCKET> m_mapClient;

};

#endif
