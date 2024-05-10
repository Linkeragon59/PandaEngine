#include "Core_ModuleManager.h"

namespace Core
{
	bool ModuleManager::RegisterModule(Module* aModule)
	{
		if (std::find(myModules.begin(), myModules.end(), aModule) != myModules.end())
			return false;

		myModules.push_back(aModule);
		aModule->OnRegister();
		TryInitializeModule(aModule);

		RebuildUpdateQueue();
		return true;
	}

	bool ModuleManager::UnregisterModule(Module* aModule)
	{
		auto module = std::find(myModules.begin(), myModules.end(), aModule);
		if (module == myModules.end())
			return false;

		FinalizeModule(*module);
		(*module)->OnUnregister();
		myModules.erase(module);

		RebuildUpdateQueue();
		return true;
	}

	void ModuleManager::Update(Module::UpdateType aType)
	{
		for (Module* module : myModulesToUpdate)
			module->OnUpdate(aType);
	}

	void ModuleManager::TryInitializeModule(Module* aModule)
	{
		for (const std::string& dependency : aModule->myDependencies)
		{
			Module* module = GetModule(dependency);
			if (!module || !module->myIsInitialized)
			{
				// The newly registered module can't be initialized yet
				return;
			}
		}

		aModule->OnInitialize();
		aModule->myIsInitialized = true;
		
		// Try initialize modules that depend on the newly initialized module
		for (Module* module : myModules)
		{
			if (module->myIsInitialized)
				continue;
			if (std::find(module->myDependencies.begin(), module->myDependencies.end(), aModule->GetIdInternal()) == module->myDependencies.end())
				continue;
			TryInitializeModule(module);
		}
	}

	void ModuleManager::FinalizeModule(Module* aModule)
	{
		if (!aModule->myIsInitialized)
			return;

		// Set the flag now to avoid considering this module anymore in the search below
		aModule->myIsInitialized = false;
		
		// First finalize modules that depend on the module to finalize
		for (Module* module : myModules)
		{
			if (!module->myIsInitialized)
				continue;
			if (std::find(module->myDependencies.begin(), module->myDependencies.end(), aModule->GetIdInternal()) == module->myDependencies.end())
				continue;
			FinalizeModule(module);
		}

		aModule->OnFinalize();
	}

	void ModuleManager::RebuildUpdateQueue()
	{
		myModulesToUpdate.clear();
		for (Module* module : myModules)
			PushModuleToUpdateQueue(module);
	}

	void ModuleManager::PushModuleToUpdateQueue(Module* aModule)
	{
		if (!aModule->myIsInitialized)
			return;

		if (std::find(myModulesToUpdate.begin(), myModulesToUpdate.end(), aModule) != myModulesToUpdate.end())
			return;

		// First push the dependencies to the update queue
		for (const std::string& dependency : aModule->myDependencies)
			if (Module* module = GetModule(dependency))
				PushModuleToUpdateQueue(module);

		myModulesToUpdate.push_back(aModule);
	}

	Module* ModuleManager::GetModule(const std::string& anId)
	{
		for (Module* module : myModules)
			if (module->GetIdInternal() == anId)
				return module;
		return nullptr;
	}
}
