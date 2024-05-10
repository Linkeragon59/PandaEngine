#include "Core_Module.h"

#include "Core_ModuleManager.h"

namespace Core
{
	bool Module::RegisterToManager(Module* aModule)
	{
		if (!Facade::GetInstance())
			return false;
		return Facade::GetInstance()->GetModuleManager()->RegisterModule(aModule);
	}

	bool Module::UnregisterFromManager(Module* aModule)
	{
		if (!Facade::GetInstance())
			return false;
		return Facade::GetInstance()->GetModuleManager()->UnregisterModule(aModule);
	}
}
