#include "Core_Module.h"

namespace Core
{
	class ModuleManager
	{
	public:
		// TODO add error handling (OnInitialize/OnFinalize could fail for some modules)
		bool RegisterModule(Module* aModule);
		bool UnregisterModule(Module* aModule);

		void Update(Module::UpdateType aType);

	private:
		void TryInitializeModule(Module* aModule);
		void FinalizeModule(Module* aModule);

		void RebuildUpdateQueue();
		void PushModuleToUpdateQueue(Module* aModule);

		Module* GetModule(const std::string& anId);

		std::vector<Module*> myModules;
		std::vector<Module*> myModulesToUpdate; // sorted by update order
	};
}
