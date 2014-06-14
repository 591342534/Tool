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
	//�̶���ʽ
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
		//����ͨѶsocket
		SOCKET sockClient = socket(AF_INET,SOCK_STREAM,0);

		SOCKADDR_IN addrSrv;
		addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//�趨��Ҫ���ӵķ�������ip��ַ
		addrSrv.sin_family = AF_INET;
		addrSrv.sin_port = htons(8000);//�趨��Ҫ���ӵķ������Ķ˿ڵ�ַ
		connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//���������������

		vtSocket.push_back(sockClient);

		if (i % 100 == 0)
		{
			std::cout << "��������" << i << std::endl;
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
	
	////����ͨѶsocket
	//SOCKET sockClient = socket(AF_INET,SOCK_STREAM,0);

	//SOCKADDR_IN addrSrv;
	//addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//�趨��Ҫ���ӵķ�������ip��ַ
	//addrSrv.sin_family = AF_INET;
	//addrSrv.sin_port = htons(8000);//�趨��Ҫ���ӵķ������Ķ˿ڵ�ַ
	//connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//���������������

	//while (1)
	//{
	//	system("pause");

	//	//������Ϣ����ʦ
	//	send(sockClient,"������С�� ѧ�ţ�123456789��IP��ַ��192.168.1.14!",
	//		strlen("������С�� ѧ�ţ�123456789��IP��ַ��192.168.1.14!")+1,0);


	//	//����������ʦ����Ϣ
	//	char recvBuf[100];
	//	recv(sockClient, recvBuf, 100, 0);
	//	printf("������ʦ����Ϣ��\n%s\n", recvBuf);
	//}

	//

	
	//closesocket(sockClient);
	

	return 0;
}

