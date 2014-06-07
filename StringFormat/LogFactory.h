
#ifndef _LOGFACTORY_
#define _LOGFACTORY_

#include <iostream>

class CLogFactory
{
public:
	void LogDebug(const char *pData)
	{
		std::cout << pData << std::endl;
	}

};

typedef StringJoin<1024> StringJoinSTL;

#define LOG_DEBUG(E) { StringJoinSTL sj; sj + E; CLogFactory lf; lf.LogDebug(sj.GetData()); }

#endif