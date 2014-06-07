// StringFormat.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "StringJoin.h"
#include "LogFactory.h"


int _tmain(int argc, _TCHAR* argv[])
{
	LOG_DEBUG("test" + 33 + ":" + 12.123 + "[" + __FILE__ + "]" + 888 + "|" + true + "end");

	system("pause");

	return 0;
}

