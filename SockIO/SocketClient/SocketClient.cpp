// SocketClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <vector>
#include <iostream>

int _tmain(int argc, _TCHAR* argv[])
{
	//固定格式
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 1, 1 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		return -1;
	}

	if ( LOBYTE( wsaData.wVersion ) != 1 ||
		HIBYTE( wsaData.wVersion ) != 1 ) {
			WSACleanup();
			return -1;
	}

	int nMaxLinkCount = 10000;

	std::vector<SOCKET> vtSocket;

	for (int i=0; i<nMaxLinkCount; ++i)
	{
		//建立通讯socket
		SOCKET sockClient = socket(AF_INET,SOCK_STREAM,0);

		SOCKADDR_IN addrSrv;
		addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//设定需要连接的服务器的ip地址
		addrSrv.sin_family = AF_INET;
		addrSrv.sin_port = htons(8000);//设定需要连接的服务器的端口地址
		connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//与服务器进行连接

		vtSocket.push_back(sockClient);

		if (i % 100 == 0)
		{
			std::cout << "连接量：" << i << std::endl;
		}
	}

	system("pause");

	for (std::vector<SOCKET>::iterator it =vtSocket.begin(); 
		it != vtSocket.end(); ++it)
	{
		closesocket(*it);
	}

	vtSocket.clear();

	WSACleanup();
	
	////建立通讯socket
	//SOCKET sockClient = socket(AF_INET,SOCK_STREAM,0);

	//SOCKADDR_IN addrSrv;
	//addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//设定需要连接的服务器的ip地址
	//addrSrv.sin_family = AF_INET;
	//addrSrv.sin_port = htons(8000);//设定需要连接的服务器的端口地址
	//connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//与服务器进行连接

	//while (1)
	//{
	//	system("pause");

	//	//发送信息给老师
	//	send(sockClient,"姓名：小明 学号：123456789，IP地址：192.168.1.14!",
	//		strlen("姓名：小明 学号：123456789，IP地址：192.168.1.14!")+1,0);


	//	//接受来自老师的信息
	//	char recvBuf[100];
	//	recv(sockClient, recvBuf, 100, 0);
	//	printf("来自老师的信息：\n%s\n", recvBuf);
	//}

	//

	
	//closesocket(sockClient);
	

	return 0;
}

