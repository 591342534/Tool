// SocketDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "IoServer.h"

#include <Windows.h>

int _tmain(int argc, _TCHAR* argv[])
{
	CIoServer ioServer(8000);

	DWORD dwStart = GetTickCount();
	while(true)
	{
		DWORD dwNow = GetTickCount();
		DWORD dwElapsed = dwNow -dwStart;
		dwStart = dwNow;

		ioServer.Run(dwElapsed);

		Sleep(100);
	}

	return 0;
}

