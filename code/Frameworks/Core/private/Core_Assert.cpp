#include "Core_Assert.h"

#if DEBUG_BUILD
#include <cstdarg>
#include <iostream>

void Abort(const char* aFile, int aLine, const char* aString)
{
	Abort(aFile, aLine, aString, nullptr);
}

void Abort(const char* aFile, int aLine, const char* aString, const char* aMsgFormat, ...)
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

	std::cerr << "Assert failed:\t" << message << "\n"
		<< "Source:\t" << aFile << ", line " << aLine << "\n"
		<< "Expected:\t" << aString << "\n";
	abort();
}
#endif
