#ifndef _LOG_
#define _LOG_

#include "StringJoin.h"

class CDebugLog
{
public:
	CDebugLog(const char *pLog)
	{
		std::cout << pLog << std::endl;
	}
};

typedef StringJoin<1024> StringJoinSTL;

#define LOGDEBUG(E)	{ StringJoinSTL sj; sj + "[DEBUG]" + E; CDebugLog(sj.GetData()); }
#define LOGERROR(E)	{ StringJoinSTL sj; sj + "[ERROR]" + E; CDebugLog(sj.GetData()); }
#define LOGINFO(E)	{ StringJoinSTL sj; sj + "[INFO]" + E; CDebugLog(sj.GetData()); }

#endif