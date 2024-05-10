#pragma once

#include "Core_Defines.h"

#if !DEBUG_BUILD
#define Assert(X, ...) ((void)0)
#define Verify(X, ...) ((void)X)
#else
void Abort(const char* aFile, int aLine, const char* aString);
void Abort(const char* aFile, int aLine, const char* aString, const char* aMsgFormat, ...);

#define AbortIfNot(X, ...) \
	if (!(X)) \
	{ \
		Abort(__FILE__, __LINE__, #X, ##__VA_ARGS__); \
	}

#define Assert(X, ...) AbortIfNot(X, ##__VA_ARGS__)
#define Verify(X, ...) AbortIfNot(X, ##__VA_ARGS__)
#endif

#define assert(...) Assert(__VA_ARGS__)
