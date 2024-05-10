#pragma once

#include "Core_Defines.h"

#if !DEBUG_BUILD
#define Log(...) ((void)0)
#define ShortLog(...) ((void)0)
#else
void LogMsg(const char* aFile, int aLine, const char* aMsgFormat, ...);
void LogShortMsg(const char* aMsgFormat, ...);

#define Log(...) LogMsg(__FILE__, __LINE__, ##__VA_ARGS__);
#define ShortLog(...) LogShortMsg(##__VA_ARGS__);
#endif
