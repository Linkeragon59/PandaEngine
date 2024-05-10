#include "Core_Utils.h"

#if DEBUG_BUILD
#include <crtdbg.h>
#endif

void InitMemoryLeaksDetection()
{
#if DEBUG_BUILD
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif
}
