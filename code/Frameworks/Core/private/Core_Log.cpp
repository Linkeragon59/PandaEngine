#include "Core_Log.h"

#if DEBUG_BUILD
#include <cstdarg>
#include <iostream>

void LogMsg(const char* aFile, int aLine, const char* aMsgFormat, ...)
{
	std::string message;
	if (aMsgFormat)
	{
		va_list vaArgs;
		va_start(vaArgs, aMsgFormat);

		va_list vaCopy;
		va_copy(vaCopy, vaArgs);
		int len = std::vsnprintf(NULL, 0, aMsgFormat, vaCopy);
		va_end(vaCopy);

		message.resize((size_t)len + 1);
		message[(size_t)len] = 0;
		std::vsnprintf(message.data(), message.size(), aMsgFormat, vaArgs);
		va_end(vaArgs);
	}

	std::cout << "Log:\t" << message << std::endl
		<< "Source:\t" << aFile << ", line " << aLine << std::endl;
}

void LogShortMsg(const char* aMsgFormat, ...)
{
	std::string message;
	if (aMsgFormat)
	{
		va_list vaArgs;
		va_start(vaArgs, aMsgFormat);

		va_list vaCopy;
		va_copy(vaCopy, vaArgs);
		int len = std::vsnprintf(NULL, 0, aMsgFormat, vaCopy);
		va_end(vaCopy);

		message.resize((size_t)len + 1);
		message[(size_t)len] = 0;
		std::vsnprintf(message.data(), message.size(), aMsgFormat, vaArgs);
		va_end(vaArgs);
	}

	std::cout <<  message << std::endl;
}

#endif
